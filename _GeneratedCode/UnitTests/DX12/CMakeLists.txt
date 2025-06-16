cmake_minimum_required(VERSION 3.22)
project(UnitTests)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  $<1:${CMAKE_SOURCE_DIR}>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  $<1:${CMAKE_SOURCE_DIR}>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  $<1:${CMAKE_SOURCE_DIR}>)

add_executable(UnitTests)
target_compile_features(UnitTests PRIVATE cxx_std_17)
set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SKIP_INSTALL_RULES True)

file(GLOB_RECURSE SOURCE_FILES
    *.h
    *.c
    *.cpp
    *.hlsl
)

file(GLOB_RECURSE SHADER_FILES
    *.hlsl
)

list(FILTER SOURCE_FILES EXCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles")

target_include_directories(UnitTests PRIVATE ./)
target_include_directories(UnitTests PRIVATE ./imgui/)
target_include_directories(UnitTests PRIVATE ./AgilitySDK/include/)
target_include_directories(UnitTests PRIVATE ./DX12Utils/tinyexr/)
target_include_directories(UnitTests PRIVATE ./DX12Utils/tinyexr/deps/miniz/)
target_include_directories(UnitTests PRIVATE ./WinPixEventRuntime.1.0.240308001/Include/)
target_include_directories(UnitTests PRIVATE ./DXC/inc/)

target_link_directories(UnitTests PRIVATE ./WinPixEventRuntime.1.0.240308001/bin/x64/)
target_link_directories(UnitTests PRIVATE ./DXC/lib/x64/)

target_link_libraries(UnitTests d3d12.lib dxgi.lib d3dcompiler.lib WinPixEventRuntime.lib dxcompiler.lib)

target_sources(UnitTests PUBLIC ${SOURCE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

set_source_files_properties(${SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UnitTests)
