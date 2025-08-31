cmake_minimum_required(VERSION 3.29)

project(ASM32 VERSION 1.0 LANGUAGES CXX C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings and source encoding
if(MSVC)
    add_compile_options(
        /W4
        /permissive-
        /source-charset:windows-1252    # handle German umlauts in source files
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wno-gnu-anonymous-struct
        -Wnested-anon-types
    )
endif()

# Add executable
add_executable(${PROJECT_NAME}
    ASM32-Source/ASM32.cpp
    ASM32-Source/ELFWriter.cpp
    ASM32-Source/utils.cpp
    ASM32-Source/lexer.cpp
    ASM32-Source/codegen.cpp
    ASM32-Source/parser.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/ASM32-Source   # parent of elfio
)