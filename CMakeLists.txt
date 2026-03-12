cmake_minimum_required(VERSION 3.23)

set(CMAKE_PROJECT_NAME libsig)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${CMAKE_PROJECT_NAME})
enable_language(C ASM)

# ==============================================================================
# 1. Core C Library
# ==============================================================================
# We build the core logic as a static library. 
# This makes it easy to link to both the standalone app and the MEX file.
add_library(libsig STATIC src/libsig.c)

# Tell CMake where the public headers for this library are located.
# "PUBLIC" means any target that links to `my_algo` will automatically 
# know to look in the `include/` directory for headers.

target_include_directories(libsig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ==============================================================================
# 2. Standalone C Executable
# ==============================================================================
add_executable(libsig_bench app/main.c)

# Link the standalone app to your core library
target_link_libraries(libsig_bench PRIVATE libsig)
