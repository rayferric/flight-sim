cmake_minimum_required(VERSION 3.20)
project(FlightSim VERSION 1.0.0 LANGUAGES CXX)

# find libs
find_package(assimp)
find_package(glfw3)
find_package(glm)

# add third-party modules
add_subdirectory("vendor/glad")
add_subdirectory("vendor/stb")

# define target
file(GLOB_RECURSE TARGET_SOURCES CONFIGURE_DEPENDS
    "src/*.hpp"
    "src/*.inl"
    "src/*.cpp"
)
add_executable(${PROJECT_NAME} ${TARGET_SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE
	"src/pch.hpp"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "flight-sim"
)
target_include_directories(${PROJECT_NAME} PRIVATE
	"src"
)
target_link_libraries(${PROJECT_NAME}
    assimp::assimp glfw glm
    Stb Glad
)
