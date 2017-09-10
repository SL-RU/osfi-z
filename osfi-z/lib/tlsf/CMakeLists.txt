
add_library(tlsf ${CMAKE_CURRENT_SOURCE_DIR}/src/tlsf.c)

target_include_directories(tlsf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(tlsf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

TARGET_LINK_LIBRARIES(tlsf rockbox_sys)
