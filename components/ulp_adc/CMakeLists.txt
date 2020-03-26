idf_component_register(
  SRCS
    ulp.c
  INCLUDE_DIRS
    include
  REQUIRES
    ulp
    soc
    driver
    spo2
)

set(ulp_app_name ulp_adc)
set(ulp_s_sources ulp/ulp_program.S)
set(ulp_exp_dep_srcs "ulp.c")

ulp_embed_binary(${ulp_app_name} "${ulp_s_sources}" "${ulp_exp_dep_srcs}")