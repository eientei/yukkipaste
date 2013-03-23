cmake_minimum_required (VERSION 2.8)

project (yukkipaste)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake/Modules
  )

add_definitions(-pedantic -Wall -g)

find_package(LibMagic)

configure_file ("${PROJECT_SOURCE_DIR}/src/yukkipaste/config.h.in"
                "${PROJECT_BINARY_DIR}/config.h" 
                )


include_directories(
  ${PROJECT_BINARY_DIR}
  src
  )

set(YUKKIPASTE_MODULES_DIR /share/yukkipaste/modules)

add_library(yutils SHARED
  src/yutils/yustring.c
  src/yutils/yuutf.c
  src/yutils/yulog.c
  src/yutils/yupointerarray.c
  src/yutils/yuarray.c
  src/yutils/yuoptions.c
  src/yutils/yujson.c
  src/yutils/yusoundex.c
  src/yutils/yumultipart.c
  src/yutils/yurandom.c
  src/yutils/yusectconf.c
  )

install_targets(/lib yutils)

add_executable(${PROJECT_NAME}
  src/yukkipaste/main.c
  )

target_link_libraries(${PROJECT_NAME}
  yutils
  dl
  ${LIBMAGIC_LIBRARIES}
  )

install_targets(/bin ${PROJECT_NAME})


add_library(pocoo MODULE
  src/yukkipaste-modules/pocoo/module.c
  )

target_link_libraries(pocoo
  yutils
  )

install_targets(${YUKKIPASTE_MODULES_DIR} pocoo)

add_library(codepad MODULE
  src/yukkipaste-modules/codepad/module.c
  )

target_link_libraries(codepad
  yutils
  )

install_targets(${YUKKIPASTE_MODULES_DIR} codepad)

file(GLOB headers-api
  "${CMAKE_SOURCE_DIR}/src/yukkipaste-api/*.h"
  )
file(GLOB headers-utils
  "${CMAKE_SOURCE_DIR}/src/yutils/*.h"
  )

configure_file(
  ${CMAKE_SOURCE_DIR}/src/yukkipaste/yukkipaste.pc
  ${PROJECT_BINARY_DIR}/yukkipaste.pc
  @ONLY
  )

install_files(/lib/pkgconfig .pc ${PROJECT_BINARY_DIR}/yukkipaste.pc)
install_files(/include/yukkipaste/yukkipaste-api .h ${headers-api})
install_files(/include/yukkipaste/yutils         .h ${headers-utils})
install(FILES ${CMAKE_SOURCE_DIR}/src/yukkipaste/config.example
        DESTINATION ~/.config/yukkipaste)
