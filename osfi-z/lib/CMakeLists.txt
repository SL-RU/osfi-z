add_subdirectory (rbcodec)
add_subdirectory (tlsf)
add_subdirectory (fixedpoint)
add_subdirectory (rockbox_sys)
add_subdirectory (libsetjmp)

set(lib_src
  ${rbcodec_src}
  ${tlsf_src}
  ${fixedpoint_src}
  ${rockbox_sys_src}
  PARENT_SCOPE)

set(lib_h
  ${rbcodec_h}
  ${tlsf_h}
  ${fixedpoint_h}
  ${rockbox_sys_h}
  PARENT_SCOPE)
