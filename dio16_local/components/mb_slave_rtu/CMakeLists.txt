set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "mb_slave.c")

set(COMPONENT_REQUIRES freemodbus board_dio16v1 indicator)

register_component()
