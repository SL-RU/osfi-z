add_library (libasf asf)

target_include_directories (libasf PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libasf INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libasf PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libasf codecs)
