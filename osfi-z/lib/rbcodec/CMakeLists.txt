message("rbcodec library")

file(STRINGS SOURCES s)

if(${IS_ARM})
  #file(STRINGS SOURCES_arm s_a)
  #set_source_files_properties(${s_a} "-x assembler-with-cpp")
  #set(s ${s} ${s_a})
endif(${IS_ARM})
#message(${src_fls})

set (src "")
FOREACH(f ${s})
  LIST(APPEND src "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
ENDFOREACH(f)
set (rbcodec_src ${src})
#message(${src})


set (rbcodec_h
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/dsp
  ${CMAKE_CURRENT_SOURCE_DIR}/metadata
  ${CMAKE_CURRENT_SOURCE_DIR}/codecs)
#  PARENT_SCOPE)


add_library(rbcodec ${rbcodec_src})
target_include_directories(rbcodec
  INTERFACE ${rbcodec_h})

target_include_directories(rbcodec
  PUBLIC ${rbcodec_h})


TARGET_LINK_LIBRARIES(rbcodec rockbox_sys)
TARGET_LINK_LIBRARIES(rbcodec fixedpoint)
TARGET_LINK_LIBRARIES(rbcodec tlsf)

add_subdirectory (codecs)
