if(${IS_ARM})
  file(STRINGS SOURCES_arm src)
else(${IS_ARM})
  file(STRINGS SOURCES src)
endif(${IS_ARM})
  

add_library (libffmpegFLAC ${src})

target_include_directories (libffmpegFLAC PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
target_include_directories (libffmpegFLAC INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
set_target_properties(libffmpegFLAC PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(libffmpegFLAC codecs)
