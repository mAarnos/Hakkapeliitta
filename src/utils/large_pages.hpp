#ifndef LARGE_PAGES_HPP_
#define LARGE_PAGES_HPP_

// To enable Large Pages in Windows do the following:
// 1. Run gpedit.msc (or search for "Group Policy").
// 2. Under "Computer Configuration", "Windows Settings", "Security Settings", "Local Policies" click on "User Rights Assignment".
// 3. In the right panel double-click the option "Lock Pages in Memory".
// 4. Click on "Add User or Group" and add either the current account or "Everyone".
// 5. Restart the computer
// The program must also be run with administrative rights. 
// If some other program runs this program then that program must be run with the same rights.
// The alternative is to disable UAC.

#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ipc.h>
#include <sys/shm.h>
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
#ifdef _WIN32
            memory = VirtualAlloc(NULL, size, MEM_LARGE_PAGES | MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (memory)
            {
                inUse = true;
            }
            else
            {
                memory = _aligned_malloc(size, alignment);
            }
#elif
            auto num = shmget (IPC_PRIVATE, size, IPC_CREAT | SHM_R | SHM_W | SHM_HUGETLB);
            if (num == -1)
            {
                memory = memalign(alignment, size);
            }
            else
            {
                inUse = true;
                memory = shmat(num, NULL, 0x0);
            }
#endif
        }
        else
        {
#ifdef _WIN32
            memory = _aligned_malloc(size, alignment);
#elif
            memory = memalign(alignment, size);
#endif
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
#ifdef _WIN32
            _aligned_free(memory);
#elif
            free(((void**)memory)[-1]);
#endif
            return;
        }
#ifdef _WIN32
        VirtualFree(memory, 0, MEM_RELEASE);
#elif
        shmdt(A);
        shmctl(num, IPC_RMID, NULL);
#endif
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
        HANDLE hToken;
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
