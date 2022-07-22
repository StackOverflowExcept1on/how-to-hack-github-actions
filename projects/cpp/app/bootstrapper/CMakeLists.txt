project(bootstrapper LANGUAGES CXX)

option(${PROJECT_NAME}_WITHOUT_IMPORT_TABLE "build ${PROJECT_NAME} without import table" ON)

add_custom_target(
    ${PROJECT_NAME}_assembly_codegen
    COMMAND ${CMAKE_BINARY_DIR}/projects/rust/dump-assembly/target/release/dump-assembly.exe ${CMAKE_BINARY_DIR}/projects/csharp/patcher/patcher --header ${CMAKE_CURRENT_SOURCE_DIR}/include/assembly.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/assembly.h
)
add_dependencies(${PROJECT_NAME}_assembly_codegen patcher dump-assembly)

add_executable(${PROJECT_NAME} src/main.cpp)
set_embedded_options(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_assembly_codegen)
target_link_libraries(${PROJECT_NAME} loader)

if (${PROJECT_NAME}_WITHOUT_IMPORT_TABLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_WITHOUT_IMPORT_TABLE)
    target_link_libraries(${PROJECT_NAME} fakekernel32)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE include)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin/cpp/app)
