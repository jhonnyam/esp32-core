
idf_component_register(
  SRC_DIRS "."
  INCLUDE_DIRS "."
  REQUIRES
    unity
    log_wifi
)

# COMPONENT COMPILE FLAGS
target_compile_options(${COMPONENT_LIB} PRIVATE
  -Wall
#  -Werror
)

# SRC SPECIFIC COMPILE FLAGS
#set_source_files_properties(
#  test.c
#  PROPERTIES COMPILE_FLAGS
#    -Wno-unused-variable
#)