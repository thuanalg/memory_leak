# Add main.cpp file of the project root directory as a source file
project(sigev_project)
set(SOURCE_FILES main.c)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(simple_example ${SOURCE_FILES})