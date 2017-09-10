add_library (librm rm.c)

target_include_directories (librm PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (librm INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(librm PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(librm codecs)
