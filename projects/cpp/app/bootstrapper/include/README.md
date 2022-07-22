All these files comes from dotnet sources
1. [hostfxr.h](https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostfxr.h)
2. [coreclr_delegates.h](https://github.com/dotnet/runtime/blob/main/src/native/corehost/coreclr_delegates.h)

It should contain:
- `assembly.h` (see dump-assembly target for codegen details)

Script to update it
```bash
curl https://raw.githubusercontent.com/dotnet/runtime/main/src/native/corehost/coreclr_delegates.h > coreclr_delegates.h
curl https://raw.githubusercontent.com/dotnet/runtime/main/src/native/corehost/hostfxr.h > hostfxr.h
dos2unix *.h
```
