FILE(GLOB_RECURSE app_sources *.c *.cpp *.h *.S)
idf_component_register(SRCS ${app_sources})
