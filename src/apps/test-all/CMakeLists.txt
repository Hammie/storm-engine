project(test-all)

auto_setup_executable(${PROJECT_NAME} DIRECTORY test)

list(APPEND TEST_LIBRARIES
        common-test
        scripting-test
        )

target_link_libraries(${PROJECT_NAME} PUBLIC ${TEST_LIBRARIES} CONAN_PKG::catch2)