# CMakeList.txt: проект CMake для CMakeProject1; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("CMakeProject1")

set (CMAKE_CXX_STANDARD 23)

file(GLOB helloworld_SRC
     "*.h"
     "*.cpp"
)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(CMakeProject1 ${helloworld_SRC})

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
