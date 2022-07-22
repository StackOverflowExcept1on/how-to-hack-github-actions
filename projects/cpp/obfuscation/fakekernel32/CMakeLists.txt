project(fakekernel32 LANGUAGES CXX)

add_custom_target(
    ${PROJECT_NAME}_codegen
    COMMAND dump-exports > ${CMAKE_CURRENT_SOURCE_DIR}/include/fakekernel32.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/fakekernel32.h
)

add_library(${PROJECT_NAME} INTERFACE)
add_dependencies(${PROJECT_NAME} dump-exports ${PROJECT_NAME}_codegen)
target_link_libraries(${PROJECT_NAME} INTERFACE loader)
target_include_directories(${PROJECT_NAME} INTERFACE include)
