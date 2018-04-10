file(STRINGS SOURCES_lib src_fls)

message("codecs library")

add_library (codecs ${src_fls})

target_include_directories (codecs PUBLIC
  ../
  ../dsp
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  )
target_include_directories (codecs INTERFACE
  ../
  ../dsp
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  )
#set_target_properties(codecs PROPERTIES COMPILE_FLAGS ${CODECS_CFLAGS})
TARGET_LINK_LIBRARIES(codecs rockbox_sys stddeflib metadata)
#TARGET_LINK_LIBRARIES(codecs fixedpoint)
#TARGET_LINK_LIBRARIES(codecs tlsf)
#TARGET_LINK_LIBRARIES(codecs rbcodec)

  
add_subdirectory(libffmpegFLAC)
add_subdirectory(libpcm)
add_subdirectory(libmad)
#add_subdirectory(libalac)
#add_subdirectory(libm4a)
#add_subdirectory(libwma)
#add_subdirectory(libasf)
#add_subdirectory(liba52)
#add_subdirectory(librm)
#add_subdirectory(libspc)
#add_subdirectory(libmusepack)
#add_subdirectory(libtremor)
#add_subdirectory(libwavpack)

TARGET_LINK_LIBRARIES(codecs libpcm)
TARGET_LINK_LIBRARIES(codecs libmad)
TARGET_LINK_LIBRARIES(codecs libffmpegFLAC)
