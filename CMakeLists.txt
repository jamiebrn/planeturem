cmake_minimum_required(VERSION 3.16)
project(Planeturem LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)

FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.32.4
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.91.3
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  PlaneturemFramework
  GIT_REPOSITORY https://github.com/jamiebrn/planeturem-framework
  GIT_TAG main
)
FetchContent_MakeAvailable(PlaneturemFramework)

FetchContent_Declare(PlatformFolders
  GIT_REPOSITORY https://github.com/sago007/PlatformFolders
  GIT_TAG 4.2.0
)
FetchContent_MakeAvailable(PlatformFolders)

include_directories(include/)
include_directories(include/steam/)
include_directories(include/extlib/)
include_directories(${SDL2_SOURCE_DIR}/include)
include_directories(${imgui_SOURCE_DIR})
file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB IMGUI_FILES ${imgui_SOURCE_DIR}/*.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

if(WIN32)
  link_directories(include/extlib/steam/lib/win64)
elseif(UNIX)
  link_directories(include/extlib/steam/lib/linux64)
endif()

add_library(ImGui STATIC ${IMGUI_FILES})

add_executable(Planeturem ${SRC_FILES})
target_link_libraries(Planeturem PRIVATE PlaneturemFramework)
target_link_libraries(Planeturem PRIVATE SDL2::SDL2)
target_link_libraries(Planeturem PRIVATE SDL2::SDL2main)
target_link_libraries(Planeturem PRIVATE ImGui)
target_link_libraries(Planeturem PRIVATE platform_folders)
target_compile_features(Planeturem PRIVATE cxx_std_20)

if(WIN32)
  target_link_libraries(Planeturem PRIVATE steam_api64)
  target_link_options(Planeturem PRIVATE -static)
  set_target_properties(Planeturem PROPERTIES WIN32_EXECUTABLE TRUE)
  target_sources(Planeturem PRIVATE "icon/icon-data.rc")
elseif(UNIX)
  target_link_libraries(Planeturem PRIVATE steam_api)
endif()

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data
)
add_dependencies(Planeturem copy_assets)