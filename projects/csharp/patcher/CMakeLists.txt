project(patcher)

add_custom_target(
    ${PROJECT_NAME}
    ALL
    COMMAND dotnet publish -c Release -o "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}" DESTINATION ${CMAKE_BINARY_DIR}/bin PATTERN "*.pdb" EXCLUDE)
