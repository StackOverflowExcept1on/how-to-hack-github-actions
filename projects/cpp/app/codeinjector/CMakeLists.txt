project(codeinjector LANGUAGES CXX)

option(${PROJECT_NAME}_WITHOUT_IMPORT_TABLE "build ${PROJECT_NAME} without import table" ON)

add_custom_target(
    ${PROJECT_NAME}_codegen
    COMMAND ${CMAKE_BINARY_DIR}/projects/rust/shellgen/target/release/shellgen.exe ${CMAKE_BINARY_DIR}/projects/cpp/app/bootstrapper/MinSizeRel/bootstrapper.exe --header ${CMAKE_CURRENT_SOURCE_DIR}/include/shellcode.h > NUL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/shellcode.h
)
add_dependencies(${PROJECT_NAME}_codegen bootstrapper shellgen)

add_executable(${PROJECT_NAME} src/main.cpp)
set_embedded_options(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_codegen)

if (${PROJECT_NAME}_WITHOUT_IMPORT_TABLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_WITHOUT_IMPORT_TABLE)
    target_link_libraries(${PROJECT_NAME} fakekernel32)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE include)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin/cpp/app)
