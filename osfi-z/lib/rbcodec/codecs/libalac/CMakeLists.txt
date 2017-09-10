add_library (libalac alac.c)

target_include_directories (libalac PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libalac INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libalac PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libalac codecs)
