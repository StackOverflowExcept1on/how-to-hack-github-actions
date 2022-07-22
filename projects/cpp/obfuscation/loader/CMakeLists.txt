project(loader LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE hash)
target_include_directories(${PROJECT_NAME} INTERFACE include)
