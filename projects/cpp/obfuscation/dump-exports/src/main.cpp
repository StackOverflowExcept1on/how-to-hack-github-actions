#include <windows.h>

__forceinline void WriteUnchecked(HANDLE hStdout, const char *buffer) {
    WriteFile(hStdout, buffer, lstrlenA(buffer), nullptr, nullptr);
}

extern "C" DWORD mainCRTStartup() {
    auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    WriteUnchecked(
        hStdout,
        "#ifndef FAKE_KERNEL32_H\n"
        "#define FAKE_KERNEL32_H\n"
        "\n"

        "#include <loader.h>\n"
        "\n"

        "#define RESOLVE_FN(function) \\\n"
        //"reinterpret_cast<decltype(&function)>(GetProcAddress(GetModuleHandleW(L\"KERNEL32.DLL\"), #function))\n"
        //"loader::GetExportByHash<decltype(&function)>(loader::GetKernel32Base(), adler32::hash_fn_compile_time(#function))\n"
        "loader::GetExportByHash<decltype(&function)>(loader::GetModuleBaseByHash(adler32::hash_fn_compile_time(L\"KERNEL32.DLL\")), adler32::hash_fn_compile_time(#function))\n"
        "\n"
    );

    auto base = reinterpret_cast<BYTE *>(GetModuleHandleW(L"KERNEL32.DLL"));

    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
    auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS *>(base + dosHeader->e_lfanew);

    auto exportDataDirectory = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto exportDirectory = reinterpret_cast<IMAGE_EXPORT_DIRECTORY *>(base + exportDataDirectory->VirtualAddress);

    auto names = reinterpret_cast<DWORD *>(base + exportDirectory->AddressOfNames);

    for (SIZE_T i = 0; i < exportDirectory->NumberOfNames; ++i) {
        auto name = reinterpret_cast<CHAR *>(base + names[i]);
        WriteUnchecked(hStdout, "#ifndef ");
        WriteUnchecked(hStdout, name);
        WriteUnchecked(hStdout, "\n");

        WriteUnchecked(hStdout, "#define ");
        WriteUnchecked(hStdout, name);

        WriteUnchecked(hStdout, " RESOLVE_FN(");
        WriteUnchecked(hStdout, name);
        WriteUnchecked(hStdout, ")\n");

        WriteUnchecked(hStdout, "#endif\n");
    }

    WriteUnchecked(hStdout, "\n");
    WriteUnchecked(hStdout, "#endif //FAKE_KERNEL32_H\n");

    return 0;
}
