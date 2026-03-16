cmake_minimum_required(VERSION 3.23)

set(CMAKE_PROJECT_NAME libsig)

# Set a default build type to Release if the user doesn't specify one
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, defaulting to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${CMAKE_PROJECT_NAME} LANGUAGES C)

# Idiomatic way to find the system's threading library
find_package(Threads REQUIRED)

# --- Library Target ---
add_library(libsig STATIC src/libsig.c)
target_include_directories(libsig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Use Generator Expressions for configuration-specific flags
target_compile_options(libsig PRIVATE
    # Flags for Release mode (Optimized, Vectorized, Assembly output)
    $<$<CONFIG:Release>:-O3 -march=native -ffast-math -fverbose-asm -save-temps=obj>
    
    # Flags for Debug mode (No optimization, Debug symbols, Extra warnings)
    $<$<CONFIG:Debug>:-O0 -g -Wall -Wextra -pedantic>
)

# Attach compiler flags directly to the library
target_compile_options(libsig PRIVATE
    -O3
    -march=native
    -ffast-math
    -fverbose-asm    # Adds C variable names as comments in the assembly
    -save-temps=obj  # Saves the .s files during the build process
)

target_link_libraries(libsig PRIVATE m)

# --- Executable Target ---
add_executable(libsig_bench
    app/main.c
    app/bench.c)

# 2. Tell the executable where to find its local header files
target_include_directories(libsig_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)

target_compile_options(libsig_bench PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -ffast-math>
    $<$<CONFIG:Debug>:-O0 -g -Wall -Wextra>
)

target_link_libraries(libsig_bench PRIVATE 
    libsig 
    Threads::Threads
)
