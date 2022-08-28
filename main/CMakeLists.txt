file(GLOB_RECURSE SOURCES ./*.c)

file(GLOB_RECURSE SOURCESCPP ./*.cpp)

idf_component_register(SRCS  ${SOURCES}
                            ${SOURCESCPP}
                    INCLUDE_DIRS ".")
