project(core)

add_library(${PROJECT_NAME} STATIC
        src/compatibility.hpp
        src/resource_locator.cpp
        src/resource_locator.hpp
        )
get_filename_component(${PROJECT_NAME}_INCLUDE_DIRECTORY src ABSOLUTE)
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${${PROJECT_NAME}_INCLUDE_DIRECTORY}>
        )
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::spdlog)

#add_executable(${PROJECT_NAME}-test
#        test/main.cpp
#        test/message.test.cpp
#        )
#target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME} CONAN_PKG::catch2)
