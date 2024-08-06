cmake_minimum_required(VERSION 3.22)
project(/*$(Name)*/)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  $<1:${CMAKE_SOURCE_DIR}>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  $<1:${CMAKE_SOURCE_DIR}>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  $<1:${CMAKE_SOURCE_DIR}>)

add_executable(/*$(Name)*/)
target_compile_features(/*$(Name)*/ PRIVATE cxx_std_17)
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

target_include_directories(/*$(Name)*/ PRIVATE ./)
target_include_directories(/*$(Name)*/ PRIVATE ./imgui/)
/*$(if:DX12.AgilitySDKRequired:true)*/target_include_directories(/*$(Name)*/ PRIVATE ./AgilitySDK/include/)/*$(endif)*/
target_include_directories(/*$(Name)*/ PRIVATE ./DX12Utils/tinyexr/)
target_include_directories(/*$(Name)*/ PRIVATE ./DX12Utils/tinyexr/deps/miniz/)
target_include_directories(/*$(Name)*/ PRIVATE ./WinPixEventRuntime.1.0.230302001/Include/)

target_link_directories(/*$(Name)*/ PRIVATE ./WinPixEventRuntime.1.0.230302001/bin/x64/)

target_link_libraries(/*$(Name)*/ d3d12.lib dxgi.lib d3dcompiler.lib WinPixEventRuntime.lib)

target_sources(/*$(Name)*/ PUBLIC ${SOURCE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

set_source_files_properties(${SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT /*$(Name)*/)
