project(shellgen)

add_custom_target(
    ${PROJECT_NAME}
    ALL
    COMMAND cargo build --release --target-dir="${CMAKE_CURRENT_BINARY_DIR}/target"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/target/release/${PROJECT_NAME}.exe DESTINATION ${CMAKE_BINARY_DIR}/bin/rust)
