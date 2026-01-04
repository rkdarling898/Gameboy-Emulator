cmake_minimum_required(VERSION 3.16)
project(emu C)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(SDL3-3.4.0 EXCLUDE_FROM_ALL)
add_subdirectory(SDL3_ttf-3.2.2 EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(emu main.c)

# Link to the actual SDL3 library.
target_link_libraries(emu PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
