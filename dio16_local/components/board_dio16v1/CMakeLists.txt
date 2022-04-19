set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "board.c")

set(COMPONENT_REQUIRES bus xl9555 button indicator)

register_component()
