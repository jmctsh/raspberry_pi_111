#include <gpiod.h>
#include <ncurses.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <locale.h>

// 简单的赛博木鱼 ASCII 图案
static const std::vector<std::string> MUYU_ART = {
    "      ____________      ",
    "   __/            \\__   ",
    "  /   赛博木鱼         \\  ",
    " |    ()        ()     | ",
    " |        _______      | ",
    "  \\_____/       \\_____/  ",
};

static void drawCenteredArt(int start_y, int cols) {
    for (size_t i = 0; i < MUYU_ART.size(); ++i) {
        int x = (cols - (int)MUYU_ART[i].size()) / 2;
        mvprintw(start_y + (int)i, x < 0 ? 0 : x, "%s", MUYU_ART[i].c_str());
    }
}

static void showMeritPlusOne(int art_y, int cols) {
    const char* msg = "功德+1";
    int x = cols / 2 - (int)std::string(msg).size() / 2;
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(art_y - 2, x < 0 ? 0 : x, "%s", msg);
    attroff(COLOR_PAIR(3) | A_BOLD);
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    // 擦除动画文本
    mvprintw(art_y - 2, x < 0 ? 0 : x, "%*s", (int)std::string(msg).size(), "");
}

static void drawCount(int y, int cols, int count) {
    std::string s = "累计功德: " + std::to_string(count);
    int x = cols / 2 - (int)s.size() / 2;
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y, x < 0 ? 0 : x, "%s", s.c_str());
    attroff(COLOR_PAIR(2) | A_BOLD);
}

// 尝试在多个 gpiochip 上打开指定的 GPIO 线路，返回成功的 line 指针及其所属 chip（通过输出参数）
static gpiod_line* open_gpio_line_any_chip(int gpio_offset, gpiod_chip** out_chip) {
    *out_chip = nullptr;
    for (int i = 0; i < 8; ++i) {
        char name[32];
        std::snprintf(name, sizeof(name), "gpiochip%d", i);
        gpiod_chip* chip = gpiod_chip_open_by_name(name);
        if (!chip) continue;
        gpiod_line* line = gpiod_chip_get_line(chip, gpio_offset);
        if (line) {
            // 尝试申请事件（下降沿），同时打开上拉
            if (gpiod_line_request_falling_edge_events_flags(line, "cyber_muyu", GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP) == 0) {
                *out_chip = chip;
                return line;
            }
        }
        gpiod_chip_close(chip);
    }
    return nullptr;
}

int main(int argc, char** argv) {
    int gpio = 17; // 默认使用 BCM GPIO17 (物理引脚 11)
    if (argc >= 2) {
        gpio = std::atoi(argv[1]);
        if (gpio < 0) gpio = 17;
    }

    // 初始化本地化（启用 UTF-8 多字节处理）
    setlocale(LC_ALL, "");
    // 初始化 ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // 非阻塞按键读取
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK); // 木鱼颜色
        init_pair(2, COLOR_GREEN, COLOR_BLACK);  // 计数颜色
        init_pair(3, COLOR_CYAN, COLOR_BLACK);   // "功德+1" 颜色
    }

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // 打开 GPIO 线路
    gpiod_chip* chip = nullptr;
    gpiod_line* line = open_gpio_line_any_chip(gpio, &chip);

    // UI 初始绘制
    erase();
    attron(COLOR_PAIR(1));
    int art_y = rows / 2 - (int)MUYU_ART.size() / 2;
    if (art_y < 3) art_y = 3; // 预留上方空间用于动画
    drawCenteredArt(art_y, cols);
    attroff(COLOR_PAIR(1));

    int count = 0;
    drawCount(art_y + (int)MUYU_ART.size() + 2, cols, count);
    mvprintw(1, 2, "GPIO: %d  按下按钮或按空格/回车增加功德；按 q 退出", gpio);
    if (!line) {
        attron(A_BOLD);
        mvprintw(2, 2, "警告: 未能打开 GPIO 线路或事件申请失败，键盘可用于模拟触发。");
        attroff(A_BOLD);
    }
    refresh();

    auto last_event = std::chrono::steady_clock::now() - std::chrono::milliseconds(1000);

    bool running = true;
    while (running) {
        // 键盘输入
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            running = false;
            break;
        } else if (ch == ' ' || ch == '\n') {
            ++count;
            showMeritPlusOne(art_y, cols);
            drawCount(art_y + (int)MUYU_ART.size() + 2, cols, count);
            refresh();
        }

        // GPIO 事件（下降沿）
        if (line) {
            timespec ts; ts.tv_sec = 0; ts.tv_nsec = 100 * 1000 * 1000; // 100ms
            int ret = gpiod_line_event_wait(line, &ts);
            if (ret == 1) {
                gpiod_line_event ev;
                if (gpiod_line_event_read(line, &ev) == 0) {
                    // 简单软件去抖：间隔 >= 150ms
                    auto now = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event).count() >= 150) {
                        last_event = now;
                        ++count;
                        showMeritPlusOne(art_y, cols);
                        drawCount(art_y + (int)MUYU_ART.size() + 2, cols, count);
                        refresh();
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 清理资源
    endwin();
    if (chip) gpiod_chip_close(chip);

    return 0;
}