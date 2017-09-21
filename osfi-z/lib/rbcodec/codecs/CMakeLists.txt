file(STRINGS SOURCES_lib src_fls)

message("codecs library")
#message(${src_fls})

set(CODECS_CFLAGS "")
set(CODECS_LINKER "")


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
TARGET_LINK_LIBRARIES(codecs rockbox_sys)
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
#add_custom_target(codecs.dir ALL
#  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/codecs/)


function(add_codec name src libs)
  ADD_EXECUTABLE(${name}.elf ${src} codec_crt0.c)
  set_target_properties(${name}.elf
    PROPERTIES
    COMPILE_FLAGS "${CODECS_CFLAGS}"
    LINK_FLAGS "${CODECS_LINKER} -Wl,-Map,${PROJECT_BINARY_DIR}/codecs/${name}.map"
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/codecs/)
  TARGET_LINK_LIBRARIES(${name}.elf codecs)
  add_dependencies(${name}.elf codecs.dir)
  foreach(l ${libs})
    TARGET_LINK_LIBRARIES(${name}.elf ${l})
  endforeach(l)
  ADD_CUSTOM_TARGET(${name}.codec ALL
    DEPENDS ${name}.elf codecs.dir
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_BINARY_DIR}/codecs/${name}.elf  ${PROJECT_BINARY_DIR}/codecs/${name}.codec)
endfunction()

#add_codec(flac     flac.c     libffmpegFLAC)
#add_codec(wav      wav.c      libpcm       )
#add_codec(aiff     aiff.c     libpcm       )
#add_codec(smaf     smaf.c     libpcm       )
#add_codec(au       au.c       libpcm       )
#add_codec(vox      vox.c      libpcm       )
#add_codec(wav64    wav64.c    libpcm       )
#add_codec(mpa      mpa.c      libmad       )
#add_codec(alac     alac.c     "libalac;libm4a")
#add_codec(aac      aac.c      "libfaad;libm4a")
#add_codec(wma      wma.c      "libasf;libwma")
#add_codec(a52      a52.c      liba52       )
#add_codec(a52_rm      a52_rm.c      "liba52;librm"  )
#add_codec(spc      spc.c      libspc       )
#add_codec(mpc      mpc.c      libmusepack       )
#add_codec(vorbis    vorbis.c   "libtremor;tlsf;libsetjmp"       )
#add_codec(wavpack      wavpack.c      libwavpack       )

