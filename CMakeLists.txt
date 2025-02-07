cmake_minimum_required(VERSION 3.8)
project(SecureMemory)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GenerateExportHeader)

# User settable settings
set(OPTION_SECURE_UNIQUEPTR ON CACHE BOOL "Erase memory on unique ptr deletion")

# Glob includes and sources
file(GLOB_RECURSE SECUREMEMORY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SECUREMEMORY_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

# Create the library
add_library(SecureMemory SHARED ${SECUREMEMORY_SOURCES} ${SECUREMEMORY_HEADERS})

# Generate export header in the secure_memory include directory
generate_export_header(SecureMemory
    BASE_NAME SECUREMEMORY
    EXPORT_MACRO_NAME SECUREMEMORY_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/secure_memory/securememory_export.h
)

# Add this near the top after project()
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# Windows-specific settings
if(WIN32)
    target_compile_definitions(SecureMemory
        PRIVATE SECUREMEMORY_EXPORTS
        PUBLIC SECUREMEMORY_SHARED_LIBS)
    target_link_libraries(SecureMemory PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(SecureMemory PRIVATE /W4)
else()
    target_compile_options(SecureMemory PRIVATE -Wall -Wextra)
endif()

if(OPTION_SECURE_UNIQUEPTR)
    target_compile_definitions(SecureMemory PUBLIC SECUREMEMORY_OPTION_SECURE_UNIQUEPTR)
endif()

# Include directories
target_include_directories(SecureMemory 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Install rules
install(TARGETS SecureMemory
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(
    DIRECTORY include/
    DESTINATION include
)