# Function that loads subdirectories with CMakeLists.txt
function(load_subdirectories)
    file(GLOB cmake_files
            LIST_DIRECTORIES true
            RELATIVE ${CMAKE_CURRENT_LIST_DIR}
            CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_LIST_DIR}/*/CMakeLists.txt)
    foreach (file ${cmake_files})
        get_filename_component(file_path ${file} DIRECTORY)
        add_subdirectory(${file_path})
    endforeach ()
endfunction()
