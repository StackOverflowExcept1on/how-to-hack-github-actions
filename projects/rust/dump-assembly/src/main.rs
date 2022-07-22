use argh::FromArgs;

use std::ffi::OsStr;
use std::fs::{self, File};
use std::io::{self, Write};
use std::iter::once;
use std::os::windows::ffi::OsStrExt;
use std::path::PathBuf;

#[derive(FromArgs, Debug)]
/// Generates header file with packed csharp assembly
struct Args {
    /// path to target directory
    #[argh(positional)]
    target_directory: PathBuf,

    /// path to output packed csharp assembly as header file
    #[argh(option)]
    header: Option<PathBuf>,

    /// path to output packed csharp assembly as binary file
    #[argh(option)]
    output: Option<PathBuf>,
}

fn main() -> io::Result<()> {
    let args: Args = argh::from_env();

    let mut count: usize = 0;
    let mut stream = Vec::new();

    for entry in fs::read_dir(args.target_directory)? {
        let entry = entry?;
        let path = entry.path();

        if path.extension() == Some(OsStr::new("pdb")) {
            continue;
        }

        let filename = entry.file_name();
        let content = fs::read(path)?;

        let chunk = filename
            .as_os_str()
            .encode_wide()
            .chain(once(0))
            .flat_map(|c| c.to_ne_bytes())
            .collect::<Vec<_>>();

        stream.write_all(&chunk.len().to_le_bytes())?;
        stream.write_all(&chunk)?;

        stream.write_all(&content.len().to_le_bytes())?;
        stream.write_all(&content)?;

        count += 1;
    }

    let mut payload = Vec::new();
    payload.write_all(&count.to_le_bytes())?;
    payload.write_all(&stream)?;

    if let Some(header_path) = args.header {
        let output = payload
            .iter()
            .map(|byte| format!("0x{:02X}", byte))
            .collect::<Vec<_>>()
            .join(", ");

        let mut header = File::create(header_path)?;
        write!(header, include_str!("assembly_header.txt"), output)?;
    }

    if let Some(path) = args.output {
        File::create(path)?.write_all(&payload)?;
    }

    Ok(())
}
