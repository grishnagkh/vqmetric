if (NOT EXISTS "${CMAKE_SOURCE_DIR}/3rd/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The git submodule for googletest has not been downloaded!")
endif ()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/3rd/googletest")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" "${CMAKE_SOURCE_DIR}/3rd/googletest"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/3rd/googletest"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/3rd/googletest"
)
set(GTEST_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/3rd/googletest/googletest/include"
    "${CMAKE_SOURCE_DIR}/3rd/googletest/googlemock/include"
)
set(GTEST_LIBRARY_PATH  "${CMAKE_BINARY_DIR}/3rd/googletest/lib")
set(GTEST_LIBRARY       "${GTEST_LIBRARY_PATH}/libgtest.a")
set(GTEST_MAIN_LIBRARY  "${GTEST_LIBRARY_PATH}/libgtest_main.a")
set(GMOCK_LIBRARY       "${GTEST_LIBRARY_PATH}/libgmock.a")
set(GMOCK_MAIN_LIBRARY  "${GTEST_LIBRARY_PATH}/libgmock_main.a")
set(GTEST_ROOT          "${CMAKE_SOURCE_DIR}/3rd/googletest")
