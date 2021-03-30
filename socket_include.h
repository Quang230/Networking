#pragma once
#ifndef SOCKET_INCLUDE
#define SOCKET_INCLUDE

// Include header files
#	ifdef _WIN32
#		ifndef _WIN32_WINNT
#			define _WIN32_WINNT 0x0600
#		endif
#		include <WinSock2.h>
#		include <WS2tcpip.h>
#		pragma comment(lib, "ws2_32.lib")
#	else
#		include <sys/types.h>
#		include <sys/socket.h>
#		include <netinet/in.h>
#		include <arpa/inet.h>
#		include <netdb.h>
#		include <unistd.h>
#		include <errno.h>
#	endif

//  Unit data type for sockets
#	ifndef _WIN32
#		define SOCKET int
#	endif

//  Socket validation macro
#	ifdef _WIN32
#		define IS_VALID_SOCKET(s) ((s) != INVALID_SOCKET)
#	else
#		define IS_VALID_SOCKET(s) ((s) >= 0)
#	endif

//  Closing socket macros
#	ifdef _WIN32
#		define CLOSE_SOCKET(s) (closesocket(s));
#	else
#		define CLOSE_SOCKET(s) (close(s))
#	endif

//  Get error code
#	ifdef _WIN32
#		define GET_SOCKET_ERRNO() (WSAGetLastError())
#	else
#		define GET_SOCKET_ERRNO() (errno)
#	endif
#endif


