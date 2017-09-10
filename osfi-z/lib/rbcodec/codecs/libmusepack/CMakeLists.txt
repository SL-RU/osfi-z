file(STRINGS SOURCES src)
  

add_library (libmusepack ${src})

target_include_directories (libmusepack PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libmusepack INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libmusepack PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libmusepack codecs)
