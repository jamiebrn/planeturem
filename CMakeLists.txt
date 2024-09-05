cmake_minimum_required(VERSION 3.16)
project(spacebuild LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

include_directories(include/)
include_directories(steam/)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

link_directories(steam/lib/win64)

add_executable(spacebuild ${SRC_FILES})
target_link_libraries(spacebuild PRIVATE sfml-graphics)
target_link_libraries(spacebuild PRIVATE sfml-audio)
target_link_libraries(spacebuild PRIVATE steam_api64)
target_link_options(spacebuild PRIVATE -static)
target_compile_features(spacebuild PRIVATE cxx_std_20)
target_sources(spacebuild PRIVATE "icon/icon-data.rc")

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data
)
add_dependencies(spacebuild copy_assets)

if(WIN32)
    add_custom_command(
        TARGET spacebuild
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:spacebuild>
        VERBATIM)
endif()
