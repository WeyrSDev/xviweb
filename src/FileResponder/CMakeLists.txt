set(SRCS
	FileResponder.cpp
)
add_library(FileResponder MODULE ${SRCS})

target_link_libraries(FileResponder xviweb)
