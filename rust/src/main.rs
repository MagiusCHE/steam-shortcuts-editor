/*
 * Copyright (c) 2022, Magius(CHE)
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 * Read the LICENSE file for more details.
 *
 * @author: Magius(CHE) - magiusche@magius.it
 */

use chrono::{DateTime, NaiveDateTime, Utc};
use clap::{Parser, Subcommand, ValueEnum};
use iter_tools::Itertools;
use std::{
    fmt::Display,
    fs::File,
    io::{BufReader, Read},
    iter::once,
    path::Path,
};

mod shortcuts;
use shortcuts::{ShortcutProp, Shortcuts, SHORTCUT_PROP_INFO};

/// VDF Shortcuts Editor for Steam Client
#[derive(Parser, Debug)]
#[command(long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    /// List entries summary info
    List {
        /// Path to "shortcuts.vdf"
        shortcuts_path: String,

        #[arg(long, default_value = " ")]
        /// Table output columns separator
        separator: String,

        #[arg(long)]
        /// Export list in JSON format. This will ignore "--separator", "--keys", "--last_play_time_*".
        json: bool,

        #[arg(long)]
        /// Show key for each value in table output
        keys: bool,

        #[arg(value_names(&["format"]),long,ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows Index with specified format
        index: ListColumnsModes,

        #[arg(value_names(&["format"]),long,ignore_case = true, default_value_t = ListColumnsModes::Plain)]
        /// Shows AppID with specified format
        app_id: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::Plain)]
        /// Shows AppName with specified format
        app_name: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows Exe with specified format
        exe: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows Icon with specified format
        icon: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows AllowDesktopConfig with specified format
        allow_desktop_config: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows AllowOverlay with specified format
        allow_overlay: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows Devkit with specified format
        devkit: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows DevkitGameId with specified format
        devkit_game_id: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows DevkitOverrideAppId with specified format
        devkit_override_app_id: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows FlatpakAppId with specified format
        flatpak_app_id: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows IsHidden with specified format
        is_hidden: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows LastPlayTime with specified format
        last_play_time: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows LastPlayTime in "YYYY/MM/DD, hh:mm:ss UTC" with specified format
        last_play_time_utc: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows LastPlayTime in "YYYY/MM/DD, hh:mm:ss" (Localtime) with specified format
        last_play_time_fmt: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows LastPlayTime in ISO with specified format
        last_play_time_iso: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows LaunchOptions with specified format
        launch_options: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows OpenVR with specified format
        open_vr: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows ShortcutPath with specified format
        shortcut_path: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows StartDir with specified format
        start_dir: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows Tags with specified format
        tags: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Override all columns format with the specified one
        all: ListColumnsModes,
    },
    /// Update entries structure recreating .vdf shortcuts file
    Edit {
        /// Path to input (or/and eventually output) file "shortcuts.vdf"
        shortcuts_path: Option<String>,

        /// Path to json contains the entries. It will ignore --idx, --key, --val. If <SHORTCUTS_PATH> not exists, --json-path will be required.
        #[arg(long)]
        json_path: Option<String>,

        /// Index of the entry to operate on (requires --key and --val) if the entry does not exist idx will be ignored and a new one will be created to the end of the list.
        #[arg(long)]
        idx: Option<u32>,

        /// Single key to change on Shortcuts[idx] (requires --idx and --val)
        #[arg(long)]
        key: Option<String>,

        /// New value for Shortcuts[idx].key=? (requires --idx and --key)
        #[arg(long)]
        val: Option<String>,

        /// Output file destination for generated vdf. If <SHORTCUTS_PATH> is missing --out will be used as output.
        #[arg(long)]
        out: Option<String>,

        /// Overwrite destination (--out) if exists.
        #[arg(long)]
        force: bool,
    },
    /// Print version information
    Version,
}

#[derive(ValueEnum, Clone, Debug, Eq, PartialEq)]
enum ListColumnsModes {
    None,
    Plain,
    //Padded,
}
impl Display for ListColumnsModes {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

fn main() {
    match handle_commandline() {
        Ok(_) => std::process::exit(exitcode::OK),
        Err(e) => {
            match e {
                Error::InvalidInputFile(m) => eprintln!("Error! Invalid input file: {}", m),
                Error::InvalidOutputFile(m) => eprintln!("Error! Invalid output file: {}", m),
                Error::IvalidUInt32Passed(m) => 
                    eprintln!("Error! Cannot convert string to uint32: {}", m),
                
                Error::IvalidStringsPassed(m) => eprintln!("Error! Cannot convert string to string array: {}", m),
            };
            eprintln!("Program aborted.");
            std::process::exit(1);
        }
    }
}

fn handle_commandline() -> Result<(), Error> {
    // https://blog.logrocket.com/command-line-argument-parsing-rust-using-clap/
    let args = Cli::parse();

    match &args.command {
        Commands::List { .. } => list_shortcuts(&args)?,
        Commands::Version => println!(
            "{} {} by {}, {}",
            env!("CARGO_PKG_NAME"),
            env!("CARGO_PKG_VERSION"),
            env!("CARGO_PKG_AUTHORS"),
            env!("CARGO_PKG_HOMEPAGE")
        ),
        Commands::Edit { .. } => edit_shortcuts(&args)?,
    };

    Ok(())
}

fn edit_shortcuts(args: &Cli) -> Result<(), Error> {
    if let Commands::Edit {
        shortcuts_path,
        out,
        json_path,
        key,
        val,
        idx,
        force,
        ..
    } = &args.command
    {
        if shortcuts_path.is_none() && out.is_none() {
            return Err(Error::InvalidInputFile(String::from(
                "Missing required <SHORTCUTS_PATH> or --out. Check the usage.",
            )));
        }
        if shortcuts_path.is_none() && json_path.is_none() {
            return Err(Error::InvalidInputFile(String::from(
                "Missing required <SHORTCUTS_PATH> or --json-path. Check the usage.",
            )));
        }
        if json_path.is_none() && (key.is_none() || val.is_none() || idx.is_none()) {
            return Err(Error::InvalidInputFile(String::from(
                "Missing required --json-path or --idx + --key + --val. Check the usage.",
            )));
        }
        let mut scs = if let Some(path) = shortcuts_path {
            load_shortcuts(path.as_str())?
        } else {
            Shortcuts::empty()
        };

        if let (Some(i), Some(k), Some(v)) = (idx, key, val) {
            //let mkey = Box::new(k.clone());
            scs.at_or_new(i, |_, sc: &mut shortcuts::Shortcut| -> Result<(), Error> {
                //let p = mkey.clone();
                //sc.props.entry(String::from(mkey.clone().as_str()));
                //let j = String::from(mkey.as_str());
                match &sc.props[k] {
                    ShortcutProp::UInt32(_) => {
                        if let Ok(tou32) = v.parse::<u32>() {
                            *sc.props.entry(k.clone()).or_default() = ShortcutProp::UInt32(tou32);
                            Ok(())
                        } else {
                            Err(Error::IvalidUInt32Passed(format!(
                                "Cannot convert from {} to UInt32",
                                v
                            )))
                        }
                    }
                    ShortcutProp::String(_) => {
                        *sc.props.entry(k.clone()).or_default() = ShortcutProp::String(v.clone());
                        Ok(())
                    }
                    ShortcutProp::Strings(_) => {
                        //Try deserialize string array
                        if let Ok(arr) = serde_json::from_str::<Vec<String>>(v) {                            
                            *sc.props.entry(k.clone()).or_default() =
                                ShortcutProp::Strings(arr);
                            Ok(())
                        } else {
                            Err(Error::IvalidStringsPassed(format!(
                                "Cannot deserialize `{}` as JsonStringArray. Espected something like [\"str1\",\"str2\"].",
                                v
                            )))
                        }
                    }
                    ShortcutProp::None => panic!("Shortcut[{}][{}] contains a None propertry. This is not acceptable.", i, k),
                }
            })?;
        } else if let Some(jpath) = json_path {
            let jpathfile = Path::new(jpath);
            if !jpathfile.exists(){
                return Err(Error::InvalidInputFile(format!("JSON Path is invalid. Missing file at {}",jpath)))
            }

            match File::open(jpathfile){
                Ok(mut file) => {
                    let mut buf = String::new();
                    if let Ok(_) = file.read_to_string(&mut buf) {
                        match scs.update_from_json(&buf) {
                            Ok(_) => Ok(()),
                            Err(err) => Err(Error::InvalidInputFile(format!("JSON Input file is invalid: {:?}",err)))
                        }
                    } else {
                        Err(Error::InvalidInputFile(format!("Canno read from JSON Input file {}",jpath)))
                    }
                },
                Err(_) => todo!(),
            }?;

            
        }

        let destination = Path::new( if let Some(p) = out { p } else { shortcuts_path.as_ref().unwrap() } );
        
        // Check if destination esists, is exists test force!
        if destination.exists() && !force{
            return Err(Error::InvalidOutputFile(format!("Shortcuts file already exists at: \"{}\". Use --force to overwire it.", destination.to_str().unwrap())))
        }

        println!("Write to file: {}", destination.to_str().unwrap());
        
        match File::create(destination) {
            Ok(mut file) =>  match scs.store_into(&mut file){
                Ok(_) => Ok(()),
                Err(err) => Err(Error::InvalidOutputFile(format!("Unable to create file {}. {:?}", destination.to_str().unwrap(),err))),
            },
            Err(err) => Err(Error::InvalidOutputFile(format!("Unable to create file {}. {:?}", destination.to_str().unwrap(),err)))
        }?;
    } else {
        unreachable!();
    }
    Ok(())
}

fn load_shortcuts(shortcuts_path: &str) -> Result<Shortcuts, Error> {
    // Ensure shortcuts_vdf path
    let path_joined = Path::new(&shortcuts_path).join("shortcuts.vdf");
    let path_raw = Path::new(&shortcuts_path);

    let shortcuts_vdf = match path_raw.file_name() {
        Some(_) if path_raw.exists() && path_raw.is_file() => path_raw.clone(),
        Some(_) if path_joined.exists() && path_joined.is_file() => Path::new(&path_joined),
        _ => return Err(Error::InvalidInputFile(String::from(
            "<SHORTCUTS_PATH> must be an existining file or folder contains .../<shortcuts>.vdf",
        ))),
    };

    //println!("Analyze {:?}:", shortcuts_vdf);

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
        return Ok(scs);
    }
    Err(Error::InvalidInputFile(format!(
        "Error while loading file {}",
        shortcuts_path
    )))
}

macro_rules! format_column_output {
    ($sc:expr,$keys:expr,$all:expr,$b:expr,$t:ty,$c:expr,$d:expr) => {
        once(
            match if $all != &ListColumnsModes::None {
                $all
            } else {
                $b
            } {
                ListColumnsModes::Plain => {
                    //println!("format_column_output {},{:?}",stringify!($b), $sc);
                    let p = SHORTCUT_PROP_INFO.iter().find(|s|s.switchname == stringify!($b)).unwrap();
                    //println!("found format_column_output {}, {:?}",stringify!($b),$sc.props.get($c));
                    let val =
                        TryInto::<$t>::try_into($sc.props.get($c).unwrap_or_default()).unwrap();
                    Some(if *$keys {
                        (p, format!(concat!(stringify!($b), " = ", $d), val))
                    } else {
                        (p,format!($d, val))
                    })
                }
                _ => None,
            },
        )
    };
}

macro_rules! format_column_output_ex {
    ($keys:expr,$all:expr,$b:expr,$prop:expr,$c:expr,$d:expr) => {
        once(
            match if $all != &ListColumnsModes::None {
                $all
            } else {
                $b
            } {
                ListColumnsModes::Plain => {
                    let p = SHORTCUT_PROP_INFO
                        .iter()
                        .find(|s| s.switchname == stringify!($prop))
                        .unwrap();
                    let val = $c;
                    Some(if *$keys {
                        (p, format!(concat!(stringify!($b), " = ", $d), val))
                    } else {
                        (p, format!($d, val))
                    })
                }
                _ => None,
            },
        )
    };
}

fn list_shortcuts(args: &Cli) -> Result<(), Error> {
    if let Commands::List {
        shortcuts_path,
        separator,
        index,
        app_id,
        app_name,
        exe,
        icon,
        allow_desktop_config,
        allow_overlay,
        devkit,
        devkit_game_id,
        devkit_override_app_id,
        flatpak_app_id,
        is_hidden,
        last_play_time,
        last_play_time_fmt,
        last_play_time_utc,
        last_play_time_iso,
        launch_options,
        open_vr,
        shortcut_path,
        start_dir,
        tags,
        all,
        keys,
        json,
        ..
    } = &args.command
    {
        let scs = load_shortcuts(shortcuts_path)?;

        if *json {
            println!(
                "[{}]",
                scs.iter()
                    .map(|sc| format!(
                        "{{{}}}",
                        sc.props
                            .keys()
                            .sorted()
                            .map(|k| format!(
                                "{:?}:{}",
                                k,
                                match sc.props.get(k) {
                                    Some(shortcuts::ShortcutProp::UInt32(u)) => u.to_string(),
                                    Some(shortcuts::ShortcutProp::String(s)) => format!("{:?}", s),
                                    Some(shortcuts::ShortcutProp::Strings(s)) => format!("{:?}", s),
                                    _ => "".to_owned(),
                                }
                            ))
                            .collect::<Vec<String>>()
                            .join(",")
                    ))
                    .collect::<Vec<String>>()
                    .join(",")
            )
        } else {
            println!(
                "{}",
                scs.iter()
                    .map(|sc| {
                        format_column_output!(sc, keys, all, index, u32, "index", "{}")
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                allow_desktop_config,
                                u32,
                                "allow_desktop_config",
                                "{}"
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                allow_overlay,
                                u32,
                                "allow_overlay",
                                "{}"
                            ))
                            .chain(format_column_output!(
                                sc, keys, all, app_id, u32, "app_id", "{}"
                            ))
                            .chain(format_column_output!(
                                sc, keys, all, app_name, String, "app_name", "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc, keys, all, devkit, u32, "devkit", "{}"
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                devkit_game_id,
                                String,
                                "devkit_game_id",
                                "{}"
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                devkit_override_app_id,
                                u32,
                                "devkit_override_app_id",
                                "{}"
                            ))
                            .chain(format_column_output!(
                                sc, keys, all, exe, String, "exe", "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                flatpak_app_id,
                                String,
                                "flatpak_app_id",
                                "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc, keys, all, icon, String, "icon", "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                is_hidden,
                                u32,
                                "is_hidden",
                                "{}"
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                last_play_time,
                                u32,
                                "last_play_time",
                                "{}"
                            ))
                            .chain(format_column_output_ex!(
                                keys,
                                all,
                                last_play_time_fmt,
                                last_play_time,
                                NaiveDateTime::from_timestamp(
                                    TryInto::<u32>::try_into(
                                        sc.props.get("last_play_time").unwrap_or_default()
                                    )
                                    .unwrap() as i64,
                                    0
                                ),
                                "\"{}\""
                            ))
                            .chain(format_column_output_ex!(
                                keys,
                                all,
                                last_play_time_iso,
                                last_play_time,
                                NaiveDateTime::from_timestamp(
                                    TryInto::<u32>::try_into(
                                        sc.props.get("last_play_time").unwrap_or_default()
                                    )
                                    .unwrap() as i64,
                                    0
                                ),
                                "\"{}\""
                            ))
                            .chain(format_column_output_ex!(
                                keys,
                                all,
                                last_play_time_utc,
                                last_play_time,
                                DateTime::<Utc>::from_utc(
                                    NaiveDateTime::from_timestamp(
                                        TryInto::<u32>::try_into(
                                            sc.props.get("last_play_time").unwrap_or_default()
                                        )
                                        .unwrap() as i64,
                                        0
                                    ),
                                    Utc
                                ),
                                "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                launch_options,
                                String,
                                "launch_options",
                                "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc, keys, all, open_vr, u32, "open_vr", "{}"
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                shortcut_path,
                                String,
                                "shortcut_path",
                                "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                start_dir,
                                String,
                                "start_dir",
                                "\"{}\""
                            ))
                            .chain(format_column_output!(
                                sc,
                                keys,
                                all,
                                tags,
                                Vec<String>,
                                "tags",
                                "{:?}"
                            ))
                            .filter_map(|e| e)
                            .sorted_by(|(a, _), (b, _)| Ord::cmp(&a.order, &b.order))
                            .map(|(_, v)| v)
                            .collect::<Vec<String>>()
                            .join(
                                /*if *keys {
                                    format!(stringify!(",{}"),separator)
                                } else {
                                    format!("{}",separator)
                                }.as_str()*/
                                separator,
                            )
                    })
                    .collect::<Vec<String>>()
                    .join("\n")
            )
        }
    } else {
        unreachable!();
    }

    Ok(())
}

#[derive(Debug)]
enum Error {
    InvalidInputFile(String),
    InvalidOutputFile(String),
    IvalidUInt32Passed(String),
    IvalidStringsPassed(String),
}
