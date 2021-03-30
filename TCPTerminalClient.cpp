#include "socket_include.h"
#include <iostream>
#include <string>
#ifdef _WIN32
#include <conio.h>
#endif 


int main(int argc, char* argv[])
{
	// Check if all input arguments are given
	if (argc < 3)
	{
		std::cerr << "Too few inputs arguments. A IP address and a port must be given.\n";
		return 1;
	}

	// Initialize winsock if on windows
#	ifdef _WIN32
	WSADATA d;
	if (auto failure = WSAStartup(MAKEWORD(2, 2), &d); failure)
	{
		std::cerr << "Failure to initalize winsock library.\n";
		return 1;
	}
#	endif


	// Resolve address and port into a addrinfo object
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	addrinfo* remoteAddr;
	if (auto failure = getaddrinfo(argv[1], argv[2], &hints, &remoteAddr); failure)
	{
		std::cerr << "Failure to resolve address:\n" << GET_SOCKET_ERRNO() << '\n';
		WSACleanup();
		return 1;
	}

	// If in debug mode, then print the connecting address
#	ifdef _DEBUG
	char addressBuffer[100];
	char serviceBuffer[100];
	getnameinfo(
		remoteAddr->ai_addr,
		remoteAddr->ai_addrlen,
		addressBuffer,
		sizeof(addressBuffer),
		serviceBuffer,
		sizeof(serviceBuffer),
		NI_NUMERICHOST
	);
	std::cout << "Address: " << addressBuffer << "\nService: " << serviceBuffer << '\n';
#	endif

	// Construct socket
	SOCKET peerSocket = socket(remoteAddr->ai_family, remoteAddr->ai_socktype, remoteAddr->ai_protocol);
	if (!IS_VALID_SOCKET(peerSocket))
	{
		std::cerr << "Failure to create socket:\n" << GET_SOCKET_ERRNO() << '\n';
		freeaddrinfo(remoteAddr);
		WSACleanup();
		return 1;
	}

	// Connect socket to server
	if (auto failure = connect(peerSocket, remoteAddr->ai_addr, remoteAddr->ai_addrlen); failure)
	{
		std::cerr << "Failure to connect socket to server:\n" << GET_SOCKET_ERRNO() << '\n';
		freeaddrinfo(remoteAddr);
		CLOSE_SOCKET(peerSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(remoteAddr);
	std::cout << "Connected.\nTo send data, enter text followed by enter.\nType !QUIT to exit the program.\n";

	auto isRunning = true;
	while (isRunning)
	{
		// Construct set of reading sockets
		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(peerSocket, &reads);
#		ifndef _WIN32
		FD_SET(0, &reads);	// If not on windows, then add stdin file descriptor to the set.
#		endif

		// Construt timer
		timeval timer;
		timer.tv_sec = 0;
		timer.tv_usec = 100'000;

		// Wait for data or for timer to run out
		if (select(peerSocket + 1, &reads, nullptr, nullptr, &timer) < 0)
		{
			CLOSE_SOCKET(peerSocket);
			std::cerr << "select() failed:\n" << GET_SOCKET_ERRNO() << '\n';
			WSACleanup();
			return 1;
		}

		// Check if any data has been recived
		if (FD_ISSET(peerSocket, &reads))
		{
			char read[4096];
			auto bytes_received = recv(peerSocket, read, sizeof(read), 0);
			if (bytes_received < 1)
			{
				std::cout << "Connection closed by remote server.\n";
				break;
			}
			else
			{
				printf("Received (%d bytes):\n %.*s\n",
					bytes_received, bytes_received, read);
			}
		}

		// Check for terminal input
#		ifdef _WIN32
		if (_kbhit())
#		else
		if (FD_ISSET(0, &reads))
#		endif 
		{
			if (std::string input; !std::getline(std::cin, input))
			{
				std::cout << "CORRUPTED INPUT\n";
				break;
			}
			else
			{
				// Check for quit command.
				if (!input.empty() &&
					input.front() == '!' &&
					std::equal_to<std::string>()(input.substr(1, input.size() - 1), "QUIT"))
				{
					break;
				}
				auto bytes_send = send(peerSocket, input.c_str(), input.size(), 0);
				std::cout << "Send " << bytes_send << " bytes\n";
			}

		}

	}


	// Clean up5
	CLOSE_SOCKET(peerSocket);
#	ifdef _WIN32
	WSACleanup();
#	endif

}