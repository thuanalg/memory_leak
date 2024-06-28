# Add main.cpp file of the project root directory as a source file
project(sigev_project)
set(SOURCE_FILES main.c)
set(SOURCE_FILES_CPP main.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(simple_example ${SOURCE_FILES})
add_executable(siimple_cppexample ${SOURCE_FILES_CPP})