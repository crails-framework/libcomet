# Designed for C++ to JavaScript transpiling
# -DCMAKE_TOOLCHAIN_FILE="$CHEERP_PATH/share/cmake/Modules/CheerpToolchain.cmake"
cmake_minimum_required(VERSION 3.14)

project(libcomet
  LANGUAGES CXX
  VERSION 1.0.0)

include(FetchContent)

#FetchContent_Declare(
#  libdatatree
#  GIT_REPOSITORY https://github.com/crails-framework/libdatatree.git
#  GIT_TAG        master
#)

FetchContent_Declare(
  libcrails-router
  GIT_REPOSITORY https://github.com/crails-framework/libcrails-router.git
  GIT_TAG        master
)

FetchContent_Declare(
  libcrails-semantics
  GIT_REPOSITORY https://github.com/crails-framework/libcrails-semantics.git
  GIT_TAG        master
)

#FetchContent_MakeAvailable(libdatatree)
FetchContent_MakeAvailable(libcrails-router)
FetchContent_MakeAvailable(libcrails-semantics)

include_directories(
  ${libcrails-router_SOURCE_DIR}/libcrails-router
  ${libcrails-semantics_SOURCE_DIR}/libcrails-semantics
  ./libcomet
)

file(GLOB_RECURSE comet_src libcomet/*.cpp)
file(GLOB_RECURSE comet_headers libcomet/*.hpp)

add_library(comet STATIC ${comet_src})

target_link_libraries(comet PRIVATE crails-semantics)
target_compile_definitions(comet PUBLIC USE_OLD_CLIENTLIB) # Cheerp >3.0 provides a new clientlib

install(TARGETS comet DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY "${CMAKE_SOURCE_DIR}/libcomet/comet"
        DESTINATION "include"
        FILES_MATCHING
        PATTERN "*.hpp")
