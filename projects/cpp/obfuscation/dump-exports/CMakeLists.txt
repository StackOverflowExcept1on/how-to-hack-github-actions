project(dump-exports LANGUAGES CXX)

add_executable(${PROJECT_NAME} src/main.cpp)
set_embedded_options(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin/cpp/obfuscation)
