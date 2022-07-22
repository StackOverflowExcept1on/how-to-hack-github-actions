#include <windows.h>

#include <hostfxr.h>
#include <coreclr_delegates.h>
#include <fx_muxer.h>

#include <loader.h>

#ifdef bootstrapper_WITHOUT_IMPORT_TABLE
#include <fakekernel32.h>
#endif

/// This enums represents possible errors to hide it from others
/// useful for debugging
enum class InitializeResult : uint8_t {
    Success,
    GetRuntimeDelegateError,
    EntryPointError,
};

#pragma code_seg(".text")

#include <assembly.h>

extern "C" /*DWORD*/ InitializeResult mainCRTStartup() {
    //region loading files to disk
    auto stream = reinterpret_cast<const uint8_t *>(packedAssembly);

    auto countOfFiles = *reinterpret_cast<const size_t *>(stream);
    stream += sizeof(const size_t);

    for (size_t i = 0; i < countOfFiles; ++i) {
        auto filenameSize = *reinterpret_cast<const size_t *>(stream);
        stream += sizeof(const size_t);

        auto filename = reinterpret_cast<const wchar_t *>(stream);
        stream += filenameSize;

        auto bufferSize = *reinterpret_cast<const size_t *>(stream);
        stream += sizeof(const size_t);

        auto buffer = reinterpret_cast<const wchar_t *>(stream);
        stream += bufferSize;

        const auto dwBufferLength = MAX_PATH;
        wchar_t path[dwBufferLength];

        GetTempPathW(dwBufferLength, path);
        lstrcatW(path, filename);

        HANDLE hFile = CreateFileW(
            path,
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        WriteFile(hFile, buffer, static_cast<DWORD>(bufferSize), nullptr, nullptr);
        CloseHandle(hFile);
    }
    //endregion

    /// base address of `hostfxr.dll`
    auto base = loader::GetModuleBaseByHash(adler32::hash_fn_compile_time(L"hostfxr.dll"));

    /// function pointer to `hostfxr_get_runtime_delegate` (exported)
    auto get_runtime_delegate_fptr = loader::GetExportByHash<hostfxr_get_runtime_delegate_fn>(
        base,
        adler32::hash_fn_compile_time("hostfxr_get_runtime_delegate")
    );

    //region finding address of function pointer with cursed code

    /// function pointer to `hostfxr_get_runtime_properties` (exported)
    auto hostfxr_get_runtime_properties_fptr = loader::GetExportByHash<hostfxr_get_runtime_properties_fn>(
        base,
        adler32::hash_fn_compile_time("hostfxr_get_runtime_properties")
    );

    /// look for 2-nd x86_64 `call` instruction (opcode 0xE8, size 5 bytes)
    /// based on code xrefs
    auto buf = reinterpret_cast<BYTE *>(hostfxr_get_runtime_properties_fptr);

    uint8_t count = 0;
    while (count != 2) {
        if (*buf++ == 0xE8) {
            count++;
        }
    }

    auto instructionAddress = reinterpret_cast<SIZE_T>(buf);
    auto functionAddress = instructionAddress + *reinterpret_cast<DWORD *>(buf) + 5;

    /// function pointer to `fx_muxer_t::get_active_host_context`
    auto get_active_host_context_fptr = reinterpret_cast<decltype(&fx_muxer_t::get_active_host_context)>(functionAddress);

    //endregion

    /// @see https://github.com/dotnet/runtime/blob/main/src/native/corehost/fxr/hostfxr.cpp
    /// ctrl+f: "Hosting components context has not been initialized. Cannot get runtime properties."
    const host_context_t *context_maybe = get_active_host_context_fptr();
    const auto host_context_handle = reinterpret_cast<const hostfxr_handle>(const_cast<host_context_t *>(context_maybe));

    /// @see https://github.com/dotnet/runtime/blob/main/docs/design/features/hosting-layer-apis.md
    /// from docs: native function pointer to the requested runtime functionality
    void *delegate = nullptr;
    int ret = get_runtime_delegate_fptr(host_context_handle,
                                        hostfxr_delegate_type::hdt_load_assembly_and_get_function_pointer, &delegate);
    if (ret != 0 || delegate == nullptr) {
        return InitializeResult::GetRuntimeDelegateError;
    }

    /// `void *` -> `load_assembly_and_get_function_pointer_fn`, undocumented???
    auto load_assembly_fptr = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(delegate);

    typedef void (CORECLR_DELEGATE_CALLTYPE *custom_entry_point_fn)();
    custom_entry_point_fn custom = nullptr;

    const auto dwBufferLength = MAX_PATH;
    char_t assemblyPath[dwBufferLength];

    GetTempPathW(dwBufferLength, assemblyPath);
    lstrcatW(assemblyPath, assemblyFileName);

    ret = load_assembly_fptr(assemblyPath, typeName, methodName, UNMANAGEDCALLERSONLY_METHOD, nullptr,
                             reinterpret_cast<void **>(&custom));
    if (ret != 0 || custom == nullptr) {
        return InitializeResult::EntryPointError;
    }

    custom();

    return InitializeResult::Success;
}
