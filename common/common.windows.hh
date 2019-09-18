#ifdef _WINDOWS
	#include <winsock2.h>
	#include <direct.h>
	#include <ws2tcpip.h>
	#include <windows.h>

	#define MAXFLOAT std::numeric_limits<float>::max()

	#include "common.hh"
#endif
