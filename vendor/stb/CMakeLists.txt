cmake_minimum_required(VERSION 3.20)

project(Stb VERSION 1.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
	"stb.cpp"
)
target_include_directories(${PROJECT_NAME} PUBLIC
	"include"
)
