cmake_minimum_required(VERSION 3.0)
project(tests)

file(GLOB SOURCE *.c *.h ../src/*.c ../src/*.h)

include_directories(../src/include)

if (MSVC)
  add_compile_options(/W4 /WX /wd4996)
else()
  add_compile_options(-Wall -Wextra -Werror -pedantic -std=c11)
endif()

add_definitions(-DHYMN_TESTING)

add_executable(${PROJECT_NAME} ${SOURCE})
