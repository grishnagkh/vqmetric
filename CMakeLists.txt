cmake_minimum_required(VERSION 3.16)

project(vqmetric)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


include_directories(include/)
set(VQMETRIC_SRC 
	src/Metric.cpp
	src/PSNR.cpp
	src/SSIM.cpp
	src/VQM.cpp
	src/VideoCaptureReader.cpp
	src/VideoReader.cpp
	src/Y4MReader.cpp
	src/main.cpp
)

add_executable(vqmetric ${VQMETRIC_SRC})
target_link_libraries(vqmetric ${OpenCV_LIBS})
