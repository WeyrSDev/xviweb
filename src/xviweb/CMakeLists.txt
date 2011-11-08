set(SRCS
	Address.cpp
	Connection.cpp
	HttpConnection.cpp
	HttpRequestImpl.cpp
	HttpResponseImpl.cpp
	Responder.cpp
	ResponderModule.cpp
	Server.cpp
	String.cpp
	Util.cpp
	main.cpp
)
add_executable(xviweb ${SRCS})

set_target_properties(xviweb PROPERTIES ENABLE_EXPORTS ON)

target_link_libraries(xviweb dl)

install(
	TARGETS xviweb
	RUNTIME DESTINATION bin
)
