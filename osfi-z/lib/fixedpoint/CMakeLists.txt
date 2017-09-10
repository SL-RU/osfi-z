#project (Fixedpoint)
set (fixedpoint_src
  ${CMAKE_CURRENT_SOURCE_DIR}/fixedpoint.c)
#  PARENT_SCOPE)
#TARGET_LINK_LIBRARIES(fixedpoint rockbox_sys)

set (fixedpoint_h ${CMAKE_CURRENT_SOURCE_DIR})# PARENT_SCOPE)

add_library(fixedpoint ${fixedpoint_src})
target_include_directories(fixedpoint INTERFACE ${fixedpoint_h})
