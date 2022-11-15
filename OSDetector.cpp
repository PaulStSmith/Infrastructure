// includes
#include <windows.h>

#ifndef __PLATFORM
#include "OSDetector.h"
#endif

PLATFORM getPlatform()
{
    OSVERSIONINFO OSversion;

    OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OSversion);

    switch(OSversion.dwPlatformId)
    {
        case VER_PLATFORM_WIN32s:
            return PLATFORM_WIN32;
        case VER_PLATFORM_WIN32_WINDOWS:
            return PLATFORM_WIN32;
        case VER_PLATFORM_WIN32_NT:
            if (OSversion.dwMajorVersion == 5 && OSversion.dwMinorVersion == 0)
                return PLATFORM_WIN32_2000;
            else if(OSversion.dwMajorVersion == 5 &&   OSversion.dwMinorVersion == 1)
                return PLATFORM_WIN32_XP;
            else if (OSversion.dwMajorVersion <= 4)
                return PLATFORM_WIN32_NT;
            else
                //for unknown windows/newest windows version
                return PLATFORM_WIN32_NT;
    }
    return PLATFORM_WIN32;
}
