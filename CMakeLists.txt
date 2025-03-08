cmake_minimum_required(VERSION 3.10)
project(C64)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set build type
set(CMAKE_BUILD_TYPE Debug)

# set compilers
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# set c/c++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# include directories
include_directories(include)

# source files
file(GLOB_RECURSE CPP_SRC src/*.cpp)
file(GLOB_RECURSE C_SRC src/*.c)
file(GLOB_RECURSE ASM_SRC src/*.asm)

# set output directories
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)

# enable position-independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# default target
add_executable(${PROJECT_NAME} ${C_SRC} ${CPP_SRC})

# compiler & linker flags
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
target_link_libraries(${PROJECT_NAME} m)

# build types
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -g)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O2)
endif()

enable_language(ASM_NASM)

# compile asm files separately
foreach(ASM_FILE ${ASM_SRC})
    get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)
    set(ASM_OBJ ${BIN_DIR}/${ASM_NAME}.o)
    add_custom_command(
        OUTPUT ${ASM_OBJ}
        COMMAND nasm -f elf64 -o ${ASM_OBJ} ${ASM_FILE}
        DEPENDS ${ASM_FILE}
    )
    list(APPEND ASM_OBJECTS ${ASM_OBJ})
endforeach()

# link asm objects
target_sources(${PROJECT_NAME} PRIVATE ${ASM_OBJECTS})

set(EMCXX em++)
set(WASM_CFLAGS "-sWASM=1 -s EXPORTED_FUNCTIONS=\"['_startEmulator','_getFramebuffer','_keyDown','_keyUp','_getClockSpeed','_writeToMemory','_readFromMemory','_reset','_paused','_resume','_getMemory','_getDiffSize','_getDiff']\" -sMODULARIZE -sEXPORT_ES6 --no-entry -s EXPORTED_RUNTIME_METHODS=\"['ccall','cwrap']\" -O3 -flto -sASYNCIFY -sWASM_BIGINT=1 -sALLOW_MEMORY_GROWTH=1")
set(WASM_LDFLAGS "-sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web --no-entry -flto -O3")

add_custom_target(wasm
    COMMAND ${EMCXX} ${WASM_CFLAGS} ${WASM_LDFLAGS} -o svelte/static/${PROJECT_NAME}.js ${CPP_SRC} ${C_SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(format
    COMMAND clang-format -i ${C_SRC} ${CPP_SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
