#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

DWORD getPID(std::string processName)
{

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
    } while (pid == 0);

    std::cout << processName << " found: [0x" << std::hex << pid << "]" << std::endl;

    getchar();

    return 0;
}