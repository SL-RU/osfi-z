file(STRINGS SOURCES src)
  

add_library (libwavpack ${src})

target_include_directories (libwavpack PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libwavpack INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libwavpack PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libwavpack codecs)
