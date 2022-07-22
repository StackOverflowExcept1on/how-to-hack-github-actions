#ifndef LOADER_H
#define LOADER_H

#include <adler32.h>
#include <winternl.h>

namespace loader {
    __forceinline PVOID GetModuleBaseByHash(uint32_t hash) {
        auto peb = NtCurrentTeb()->ProcessEnvironmentBlock;
        auto ldr = peb->Ldr;

        auto head = &ldr->InMemoryOrderModuleList;
        auto current = head->Flink;

        do {
            auto entry = CONTAINING_RECORD(current, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
            auto dllName = (&entry->FullDllName) + 1;

            if (adler32::hash_fn(dllName->Buffer) == hash) {
                return entry->DllBase;
            }

            current = current->Flink;
        } while (current != head);

        return nullptr;
    }

    __forceinline PVOID GetKernel32Base() {
        auto peb = NtCurrentTeb()->ProcessEnvironmentBlock;
        auto ldr = peb->Ldr;

        auto head = &ldr->InMemoryOrderModuleList;
        auto current = head->Flink;

        //current -> ntdll -> kernel32
        return CONTAINING_RECORD(current->Flink->Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks)->DllBase;
    }

    template<typename functionPtrType>
    __forceinline functionPtrType GetExportByHash(PVOID moduleBase, uint32_t hash) {
        auto base = reinterpret_cast<BYTE *>(moduleBase);

        auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
        auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS *>(base + dosHeader->e_lfanew);

        auto exportDataDirectory = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        auto exportDirectory = reinterpret_cast<IMAGE_EXPORT_DIRECTORY *>(base + exportDataDirectory->VirtualAddress);

        auto functions = reinterpret_cast<DWORD *>(base + exportDirectory->AddressOfFunctions);
        auto names = reinterpret_cast<DWORD *>(base + exportDirectory->AddressOfNames);
        auto nameOrdinals = reinterpret_cast<WORD *>(base + exportDirectory->AddressOfNameOrdinals);

        for (SIZE_T i = 0; i < exportDirectory->NumberOfNames; ++i) {
            auto name = reinterpret_cast<CHAR *>(base + names[i]);
            auto function = functions[nameOrdinals[i]];

            if (adler32::hash_fn(name) == hash) {
                return reinterpret_cast<functionPtrType>(reinterpret_cast<SIZE_T>(base) + function);
            }
        }

        return nullptr;
    }
}

#endif //LOADER_H
