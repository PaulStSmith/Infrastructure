#ifndef __PLATFORM
#define __PLATFORM
#endif

enum PLATFORM
{
    PLATFORM_WIN32 = 1,
    PLATFORM_WIN32_NT,
    PLATFORM_WIN32_2000,
    PLATFORM_WIN32_XP
};

PLATFORM getPlatform();