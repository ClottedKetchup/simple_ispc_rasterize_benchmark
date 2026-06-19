cmake_minimum_required(VERSION 3.20)
project(ISPC_TEST LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

set(ISPC_EXECUTABLE "" CACHE PATH "Executable of ispc compiler")
if(NOT ISPC_EXECUTABLE)
    message(FATAL_ERROR "ISPC compiler not found. Please install ISPC and ensure it's in PATH.")
endif()

set(ISPC_TARGET_ARCH "x86-64" CACHE STRING "Architecture")
set_property(CACHE ISPC_TARGET_ARCH PROPERTY STRINGS "x86-64;arm64")

set(ISPC_ISA "avx2" CACHE STRING "Instruction set")
set_property(CACHE ISPC_ISA PROPERTY STRINGS "avx2;avx;sse4")

set(ISPC_PLATFORM "windows" CACHE STRING "platform")
set_property(CACHE ISPC_PLATFORM PROPERTY STRINGS "windows;linux")

set(EXE_NAME ISPC_TEST)

file(GLOB_RECURSE CPP_SOURCE CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/cpp_source/*.cpp")
file(GLOB_RECURSE CPP_HEADER CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/cpp_source/include/*.h")
file(GLOB_RECURSE ISPC_SOURCE CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/ispc_source/*.ispc")

set(ISPC_HEADER_DIR "${PROJECT_BINARY_DIR}/ispc_generated/include")
set(ISPC_OBJECT_DIR "${PROJECT_BINARY_DIR}/ispc_generated/object")
file(MAKE_DIRECTORY ${ISPC_HEADER_DIR})
file(MAKE_DIRECTORY ${ISPC_OBJECT_DIR})

set(GENERATED_ISPC_HEADERS)
set(GENERATED_ISPC_OBJECTS)

foreach(ISPC_SOURCE_FILE ${ISPC_SOURCE})
    get_filename_component(ISPC_BASENAME ${ISPC_SOURCE_FILE} NAME_WE)
    get_filename_component(ISPC_SOURCE_ABS ${ISPC_SOURCE_FILE} ABSOLUTE)

    set(ISPC_HEADER_FILE "${ISPC_HEADER_DIR}/${ISPC_BASENAME}.h")
    set(ISPC_OBJECT_FILE "${ISPC_OBJECT_DIR}/${ISPC_BASENAME}.o")
    
    # -g -O0 for ispc debug.
    # COMMAND ${ISPC_EXECUTABLE} --target=${ISPC_ISA} --target-os=${ISPC_PLATFORM} --arch=${ISPC_TARGET_ARCH} ${ISPC_SOURCE_FILE} -g  -I "${PROJECT_SOURCE_DIR}/cpp_source/include" --header-outfile=${ISPC_HEADER_FILE} -o ${ISPC_OBJECT_FILE} -O0
    add_custom_command(
        OUTPUT ${ISPC_HEADER_FILE} ${ISPC_OBJECT_FILE}
        COMMAND ${ISPC_EXECUTABLE} --target=${ISPC_ISA} --target-os=${ISPC_PLATFORM} --arch=${ISPC_TARGET_ARCH} ${ISPC_SOURCE_FILE} -g  -O0 -I "${PROJECT_SOURCE_DIR}/cpp_source/include" --header-outfile=${ISPC_HEADER_FILE} -o ${ISPC_OBJECT_FILE} 
        DEPENDS ${ISPC_SOURCE_FILE}
    )

    list(APPEND GENERATED_ISPC_HEADERS ${ISPC_HEADER_FILE})
    list(APPEND GENERATED_ISPC_OBJECTS ${ISPC_OBJECT_FILE})
endforeach()

set_source_files_properties(${ISPC_SOURCE} PROPERTIES HEADER_FILE_ONLY TRUE LANGUAGE "NONE")
set_source_files_properties(${CPP_HEADER} PROPERTIES HEADER_FILE_ONLY TRUE LANGUAGE "NONE")
set_source_files_properties(${GENERATED_ISPC_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE LANGUAGE "NONE")

source_group("ispc_include" FILES ${GENERATED_ISPC_HEADERS})
source_group("ispc_sources" FILES ${ISPC_SOURCE})
source_group("cpp_include" FILES ${CPP_HEADER})
source_group("cpp_sources" FILES ${CPP_SOURCE})

add_executable(${EXE_NAME} ${CPP_SOURCE} ${GENERATED_ISPC_OBJECTS}
    ${ISPC_SOURCE} ${CPP_HEADER} ${GENERATED_ISPC_HEADERS}
)

target_include_directories(${EXE_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/cpp_source/include" "${ISPC_HEADER_DIR}")

