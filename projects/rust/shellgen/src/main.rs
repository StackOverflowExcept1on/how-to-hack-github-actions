use argh::FromArgs;
use iced_x86::code_asm::CodeAssembler;
use pelite::pe64::{Pe, PeFile};
use pretty_hex::{HexConfig, PrettyHex};

use std::error::Error;
use std::fs::{self, File};
use std::io::Write;
use std::path::PathBuf;

mod exception_directory;

#[derive(FromArgs, Debug)]
/// Generates shellcode
struct Args {
    /// path to target binary file
    #[argh(positional)]
    filename: PathBuf,

    /// path to output shellcode as header file
    #[argh(option)]
    header: Option<PathBuf>,

    /// path to output shellcode as binary file
    #[argh(option)]
    output: Option<PathBuf>,
}

fn main() -> Result<(), Box<dyn Error>> {
    let args: Args = argh::from_env();

    let content = fs::read(&args.filename)?;
    let pe_file = PeFile::from_bytes(&content)?;

    let entrypoint = pe_file.optional_header().AddressOfEntryPoint;

    let section = pe_file
        .section_headers()
        .by_name(".text")
        .expect("failed to find .text section");

    let range = section.virtual_range();

    println!("range: 0x{:016X} - 0x{:016X}", range.start, range.end);
    println!("start: 0x{:016X}", entrypoint);
    println!();

    let cfg = HexConfig {
        group: 8,
        ..HexConfig::default()
    };

    if range.start == entrypoint {
        todo!()
    }

    let data_len = entrypoint - range.start;

    let mut assembler = CodeAssembler::new(64)?;

    let data = pe_file.derva_slice::<u8>(range.start, data_len as usize)?;

    println!("data, {:?}", data.hex_conf(cfg));
    println!();

    let mut skip_data = assembler.create_label();
    assembler.jmp(skip_data)?;

    assembler.db(data)?;
    assembler.set_label(&mut skip_data)?;

    let shellcode = exception_directory::get_entrypoint_bytes(pe_file)?;
    assembler.db(shellcode)?;

    println!("entrypoint code, {:?}", shellcode.hex_conf(cfg));
    println!();

    let payload = assembler.assemble(0)?;

    println!("payload, {:?}", payload.hex_conf(cfg));
    println!();

    if let Some(header_path) = args.header {
        let output = payload
            .iter()
            .map(|byte| format!("0x{:02X}", byte))
            .collect::<Vec<_>>()
            .join(", ");

        let mut header = File::create(header_path)?;
        write!(header, include_str!("shellcode_header.txt"), output)?;
    }

    if let Some(path) = args.output {
        File::create(path)?.write_all(&payload)?;
    }

    Ok(())
}
