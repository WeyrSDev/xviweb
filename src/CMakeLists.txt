set(SRCS
	Address.cpp
	Connection.cpp
	FileResponder.cpp
	HttpConnection.cpp
	HttpRequest.cpp
	Responder.cpp
	Server.cpp
	String.cpp
	Util.cpp
	main.cpp
)
add_executable(xviweb ${SRCS})

install(
	TARGETS xviweb
	RUNTIME DESTINATION bin
)
