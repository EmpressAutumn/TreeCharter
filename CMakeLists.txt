cmake_minimum_required(VERSION 3.16)
project(TreeCharter LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(TGUI_BACKEND SFML_GRAPHICS)

include(FetchContent)
FetchContent_Declare(sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x)
FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(sfml json)

add_executable(TreeCharter main.cpp)

target_link_libraries(TreeCharter PRIVATE sfml-graphics nlohmann_json)
target_compile_features(TreeCharter PRIVATE cxx_std_17)

install(TARGETS TreeCharter)
