#ifndef LARGE_PAGES_HPP_
#define LARGE_PAGES_HPP_

#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#else
#endif

class LargePages
{
public:
    static void initialize()
    {
        allowedToUse = false;
        inUse = false;
    }

    static void* malloc(uint64_t size, uint64_t alignment)
    {
        void* memory = nullptr;
        inUse = false;

        if (allowedToUse)
        {
            memory = VirtualAlloc(NULL, size, MEM_LARGE_PAGES | MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (memory)
            {
                inUse = true;
            }
            else
            {
                memory = _aligned_malloc(size, alignment);
            }
        }
        else
        {
            memory = _aligned_malloc(size, alignment);
        }

        return memory;
    }

    static void free(void* memory)
    {
        if (!memory)
        {
            return;
        }
        if (!inUse)
        {
            _aligned_free(memory);
            return;
        }
        VirtualFree(memory, 0, MEM_RELEASE);
    }

    static void setAllowedToUse(bool allowed)
    {
        allowedToUse = allowed;
        changeLargePagePrivileges(allowed);
    }
private:
    static bool allowedToUse;
    static bool inUse;

    static void changeLargePagePrivileges(BOOL enabled)
    {
#ifdef _WIN32
        HANDLE           hToken;
        TOKEN_PRIVILEGES tp;

        OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
        LookupPrivilegeValue(NULL, TEXT("SeLockMemoryPrivilege"), &tp.Privileges[0].Luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = (enabled ? SE_PRIVILEGE_ENABLED : 0);
        AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, 0);
        CloseHandle(hToken);
#endif
    }
};

#endif
