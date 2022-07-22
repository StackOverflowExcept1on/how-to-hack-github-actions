use pelite::pe64::{exception::Function, Pe, PeFile};

pub fn find_entrypoint(file: PeFile) -> pelite::Result<Function<PeFile>> {
    let entrypoint = file.optional_header().AddressOfEntryPoint;
    let exception = file.exception()?;

    exception
        .functions()
        .find(|function| function.image().BeginAddress == entrypoint)
        .ok_or(pelite::Error::Invalid)
}

pub fn get_entrypoint_bytes(file: PeFile) -> pelite::Result<&[u8]> {
    find_entrypoint(file)?.bytes()
}
