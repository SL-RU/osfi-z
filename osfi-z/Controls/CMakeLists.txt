set (h
  ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(controls controls_gpio.c)
target_include_directories(controls
  INTERFACE ${h})
target_include_directories(controls
  PUBLIC ${h})

target_link_libraries(controls hardware)
target_compile_definitions(controls PUBLIC ${GLOBAL_DEFS})
