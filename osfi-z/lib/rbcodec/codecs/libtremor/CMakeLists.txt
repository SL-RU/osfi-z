file(STRINGS SOURCES src)
  

add_library (libtremor ${src})

target_include_directories (libtremor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libtremor INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libtremor PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libtremor codecs)
