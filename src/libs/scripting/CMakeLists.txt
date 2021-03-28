project(scripting)

auto_setup_library(${PROJECT_NAME} SOURCE_DIRECTORY src/source INCLUDE_DIRECTORY src/include)

auto_setup_executable(${PROJECT_NAME}-test DIRECTORY test)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME} CONAN_PKG::catch2)
