file(STRINGS SOURCES src)

if(${IS_ARM})
  file(STRINGS SOURCES_arm s_a)
  set(src ${src} ${s_a})
endif(${IS_ARM})

add_library (libmad ${src})

target_include_directories (libmad PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libmad INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libmad PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libmad codecs)
