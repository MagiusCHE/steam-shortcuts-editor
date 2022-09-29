use clap::{Parser, Subcommand};
use std::{
    fs::File,
    io::{BufReader, Read},
    path::Path,
};

mod shortcuts;
use shortcuts::Shortcuts;

/// Simple program to greet a person
#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
struct Args {
    #[clap(subcommand)]
    command: Commands,

    /// Path to "shortcuts.vdf"
    shortcuts_path: String,
}

#[derive(Subcommand, Debug)]
enum Commands {
    /// List entries summary info
    List,
}

fn main() -> Result<(), Error> {
    // https://blog.logrocket.com/command-line-argument-parsing-rust-using-clap/
    let args = Args::parse();

    // Ensure shortcuts_vdf path
    let path_joined = Path::new(&args.shortcuts_path).join("shortcuts.vdf");
    let path_raw = Path::new(&args.shortcuts_path);
    let shortcuts_vdf = match path_raw.file_name() {
        Some(_) if path_raw.exists() && path_raw.is_file() => path_raw,
        Some(_) if path_joined.exists() && path_joined.is_file() => Path::new(&path_joined),
        _ => return Err(Error::InvalidInputFile(String::from(
            "<SHORTCUTS_PATH> must be an existining file or folder contains .../<shortcuts>.vdf",
        ))),
    };

    println!("Analyze {:?}:", shortcuts_vdf);

    let mut shortcuts: Vec<Shortcuts> = vec![];

    let ret = File::open(shortcuts_vdf.as_os_str().to_str().unwrap());
    if ret.is_err() {
        return Err(Error::InvalidInputFile(String::from(format!(
            "{:?} cannot be opened due to: {:?}",
            shortcuts_vdf, ret
        ))));
    }

    let f = ret.unwrap();

    let mut reader = BufReader::new(f);
    let mut buffer = Vec::new();

    // Read file into vector.
    if reader.read_to_end(&mut buffer).unwrap_or(0) == 0 {
        return Err(Error::InvalidInputFile(String::from(format!(
            "{:?} cannot be read or is empty.",
            shortcuts_vdf
        ))));
    }

    let mut index = 0;

    if let Some(scs) = Shortcuts::from(&buffer, &mut index) {
        let sc = scs.at(0);
        println!("- {}: {:?}",0,sc);
        // match args.command {
        //     Commands::List => println!("- {}: {}", sc.get("appid").unwrap_or("n/a".to_owned()), sc.get("AppName").unwrap_or("".to_owned())),
        // }
       // println!("{:?}", &sc);
    
    }

    Ok(())
}

#[derive(Debug)]
enum Error {
    InvalidInputFile(String),
}
