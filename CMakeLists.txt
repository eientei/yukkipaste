cmake_minimum_required (VERSION 2.8)

project (yukkipaste)

add_definitions(-pedantic -Wall -g)

include_directories(
  src
  )

add_executable(${PROJECT_NAME}
  src/yukkipaste/main.c
  src/yukkipaste/yustring.c
  src/yukkipaste/yuutf.c
  )

