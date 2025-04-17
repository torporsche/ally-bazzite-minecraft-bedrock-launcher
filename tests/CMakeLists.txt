add_executable(TestSuite
    TestSuite.cpp
)

target_link_libraries(TestSuite PRIVATE
    Qt6::Test
    ${PROJECT_NAME}
)

target_include_directories(TestSuite PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME TestSuite COMMAND TestSuite)