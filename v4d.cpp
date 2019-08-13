#include "v4d.h"

#ifdef _DEBUG
	#include "tests.cxx"
#endif

#ifdef _WINDOWS
    #include <windows.h>
    extern "C" BOOL WINAPI DllMain (
        HINSTANCE const instance,  // handle to DLL module
        DWORD     const reason,    // reason for calling function
        LPVOID    const reserved)  // reserved
    {
        // Perform actions based on the reason for calling.
        switch (reason) {
        case DLL_PROCESS_ATTACH:
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
            // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
            // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
            // Perform any necessary cleanup.
            break;
        }
        return TRUE;  // Successful DLL_PROCESS_ATTACH.
    }
#endif

