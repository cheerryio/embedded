set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "dio.c")

set(COMPONENT_REQUIRES xl9555 board_dio16v1)

register_component()

# target_compile_options(${COMPONENT_LIB} PRIVATE --coverage)