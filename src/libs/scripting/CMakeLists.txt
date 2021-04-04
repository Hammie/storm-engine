project(scripting)

auto_setup_library(${PROJECT_NAME} SOURCE_DIRECTORY src/source INCLUDE_DIRECTORY src/include)
target_link_libraries(${PROJECT_NAME} PUBLIC common)

auto_setup_library(${PROJECT_NAME}-test DIRECTORY test TYPE OBJECT)
target_link_libraries(${PROJECT_NAME}-test PUBLIC ${PROJECT_NAME} CONAN_PKG::catch2)
