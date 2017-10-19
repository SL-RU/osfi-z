 file(STRINGS SOURCES src)  

add_library (libfaad ${src})

target_include_directories (libfaad PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/codebook
  )
target_include_directories (libfaad INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/codebook
  )
set_target_properties(libfaad PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libfaad codecs)
