#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <wow64apiset.h>

DWORD getPID(std::string processName)
{
    auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        std::cout << "Invalid handle" << std::endl;
        return 0;
    }

    PROCESSENTRY32 entry{ sizeof(PROCESSENTRY32) };
    if (!Process32First(hSnap, &entry))
    {
        CloseHandle(hSnap);
        std::cout << "Process32First error: " << GetLastError() << std::endl;
        return 0;
    }

    DWORD pid = 0;

    do
    {
        //std::cout << entry.szExeFile << std::endl;
        if (std::string(entry.szExeFile).find(processName) != std::string::npos)
        {
            pid = entry.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnap, &entry));

    CloseHandle(hSnap);
    return pid;
}

bool threadsControl(bool resume, DWORD pid)
{
    auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
    if (hSnap == INVALID_HANDLE_VALUE)
    {
        std::cout << "invalid handle" << std::endl;
        return false;
    }

    THREADENTRY32 entry{ sizeof(THREADENTRY32) };
    if (!Thread32First(hSnap, &entry))
    {
        CloseHandle(hSnap);
        std::cout << "Thread32First failed! " << GetLastError() << std::endl;
        return false;
    }

    do
    {
        if (entry.th32OwnerProcessID != pid) continue;

        const auto handle = OpenThread(THREAD_ALL_ACCESS, TRUE, entry.th32ThreadID);
        if (!handle)
        {
            std::cout << "Failed to open handle to thread: 0x" << std::hex << entry.th32ThreadID << std::endl;
            std::cout << "GetLastError: " << std::hex << GetLastError() << std::endl;
            CloseHandle(hSnap);
            return false;
        }

        if (resume)
        {
            if (ResumeThread(handle) == -1)
            {
                CloseHandle(handle);
                CloseHandle(hSnap);
                std::cout << "ResumeThread failed!" << std::endl;
                return false;
            }
        }
        else
        {
            if (SuspendThread(handle) == -1)
            {
                std::cout << "SuspendThread failed! Trying Wow64SuspendThread" << std::endl;
                std::cout << GetLastError() << std::endl;
                if (Wow64SuspendThread(handle) == -1)
                {
                    std::cout << "Failed SuspendThread(x64) and Wow64SuspendThread(x86)" << std::endl;
                    std::cout << GetLastError() << std::endl;
                    CloseHandle(handle);
                    CloseHandle(hSnap);
                    return false;
                }
            }
        }
    } while (Thread32Next(hSnap, &entry));

    CloseHandle(hSnap);
    return true;
}

int main(int argc, char* argv[])
{
    SetConsoleTitleA("Suspend Process");

    if (argc <= 1)
    {
        std::cout << "SuspendProcess.exe [program name].exe" << std::endl;
        return 1;
    }

    std::string processName = std::string(argv[1]);

    std::cout << "Waiting " << processName << "..." << std::endl;

    DWORD pid = 0;
    do
    {
        pid = getPID(processName);
    } while (!pid);

    std::cout << processName << " found: [0x" << std::hex << pid << "]" << std::endl;

    std::cout << "Suspending threads..." << std::endl;

    if (!threadsControl(false, pid))
    {
        std::cout << "stop threads failed!\n" << std::endl;
        return 1;
    }

    std::cout << "success pause threads!" << std::endl;

    std::cout << "press enter to resume" << std::endl;
    
    getchar();

    std::cout << "Resuming threads..." << std::endl;

    if (!threadsControl(true, pid))
    {
        std::cout << "resume threads failed!\n" << std::endl;
        return 1;
    }

    std::cout << "success resume threads!" << std::endl;

    return 0;
}