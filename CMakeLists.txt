# CMakeLists for Cyber Muyu (赛博木鱼)
cmake_minimum_required(VERSION 3.10)
project(cyber_muyu CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(cyber_muyu src/main.cpp)

# Link libraries: gpiod and ncurses (prefer wide-char ncursesw)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Try to find libgpiod; if not found, link directly by name
find_library(GPIOD_LIB gpiod)
if(NOT GPIOD_LIB)
    message(WARNING "libgpiod not found via find_library; linking by name 'gpiod'. Ensure libgpiod is installed.")
    set(GPIOD_LIB gpiod)
endif()

# Prefer linking with ncursesw if available
find_library(NCURSESW_LIB ncursesw)
if(NCURSESW_LIB)
    set(CURSES_LIBRARIES ${NCURSESW_LIB})
endif()

target_link_libraries(cyber_muyu ${CURSES_LIBRARIES} ${GPIOD_LIB})