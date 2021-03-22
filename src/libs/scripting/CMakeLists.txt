project(scripting)

add_library(${PROJECT_NAME} STATIC
        src/message.cpp
        src/message.hpp
        src/compatibility.hpp
        src/string_codec.cpp
        src/string_codec.h
        src/teho_string_codec.h
        )
get_filename_component(${PROJECT_NAME}_INCLUDE_DIRECTORY src ABSOLUTE)
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${${PROJECT_NAME}_INCLUDE_DIRECTORY}>
        )

add_executable(${PROJECT_NAME}-test
        test/main.cpp
        test/message.test.cpp
        )
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME} CONAN_PKG::catch2)
