# Function that turns off C++ runtime for target
function(set_embedded_options target)
    # Remove /EHsc from CMAKE_CXX_FLAGS
    string(REPLACE " /EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/EHsc " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)

    # Turn off buffer security check, exception handling
    target_compile_options(${target} PRIVATE /GS- /EHs-c-)
    # Some flags to optimize for binary file size
    target_link_options(${target} PRIVATE
            /DRIVER
            #/SECTION:.text,,ALIGN=16
            /MANIFEST:NO
            /ALIGN:16
            /FILEALIGN:1
            #/MERGE:.rdata=.text
            #/MERGE:.pdata=.text
            /NODEFAULTLIB
            /EMITPOGOPHASEINFO
            /DEBUG:NONE
            /STUB:${CMAKE_SOURCE_DIR}/link/stub.exe)

    # Apply pedantic flags
    target_compile_options(${target} PRIVATE /W4 /permissive- /WX)
endfunction()
