#https://cliutils.gitlab.io/modern-cmake/chapters/features/utilities.html

cmake_minimum_required(VERSION 3.16)

project(vqmetric)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

option(EXPORT_COMPILE_COMMANDS "export compile commands" on)
if(${EXPORT_COMPILE_COMMANDS})
	message(STATUS "eport compile commands")
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(WITH_CLANG_TIDY "build with clang tidy support" on)
if(${WITH_CLANG_TIDY}) 
	message(STATUS "build with clang tidy support")
	find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy)
endif()

option(WITH_CPPCHECK "build with cppcheck support" on)
if(${WITH_CPPCHECK}) 
	find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
	if (CMAKE_CXX_CPPCHECK)
		message(STATUS "build with cppcheck support")
		list(APPEND CMAKE_CXX_CPPCHECK 
			"--enable=warning"
			"--inconclusive"
			"--force" 
			"--inline-suppr"
			#"--suppressions-list=${CMAKE_SOURCE_DIR}/CppCheckSuppressions.txt"
		)
	endif()
endif()

option(ADDRESS_SANITIZER "Enable Clang AddressSanitizer" on)
if (${ADDRESS_SANITIZER})
	message(STATUS "AddressSanitizer enabled for debug build")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fno-omit-frame-pointer -fsanitize=address")
endif()

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
