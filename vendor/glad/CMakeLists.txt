cmake_minimum_required(VERSION 3.20)

project(Glad VERSION 1.0.0 LANGUAGES C)

add_library(${PROJECT_NAME}
	"gl.c"
)
target_include_directories(${PROJECT_NAME} PUBLIC
	"include"
)
