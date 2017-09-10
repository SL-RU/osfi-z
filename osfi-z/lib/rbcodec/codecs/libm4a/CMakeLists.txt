add_library (libm4a m4a.c demux.c)

target_include_directories (libm4a PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libm4a INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libm4a PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libm4a codecs)
