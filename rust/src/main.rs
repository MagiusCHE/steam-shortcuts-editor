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
use std::{
    fmt::Display,
    fs::File,
    io::{BufReader, Read},
    iter::once,
    path::Path,
};

mod shortcuts;
use shortcuts::Shortcuts;

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
        /// Show key for each value in table output
        keys: bool,

        #[arg(value_names(&["format"]),long,ignore_case = true, default_value_t = ListColumnsModes::None)]
        /// Shows Index with specified format
        index: ListColumnsModes,

        #[arg(value_names(&["format"]),long,ignore_case = true, default_value_t = ListColumnsModes::Plain)]
        /// Shows AppID with specified format
        appid: ListColumnsModes,

        #[arg(value_names(&["format"]),long, ignore_case = true, default_value_t = ListColumnsModes::Plain)]
        /// Shows AppName with specified format
        appname: ListColumnsModes,

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

fn main() -> Result<(), Error> {
    // https://blog.logrocket.com/command-line-argument-parsing-rust-using-clap/
    let args = Cli::parse();

    match &args.command {
        Commands::List { shortcuts_path, .. } => {
            let scs = load_shortcuts(shortcuts_path)?;
            output_list(&args, &scs);
        }
        Commands::Version =>println!(
                    "{} {} by {}, {}",
                    env!("CARGO_PKG_NAME"),
                    env!("CARGO_PKG_VERSION"),
                    env!("CARGO_PKG_AUTHORS"),
                    env!("CARGO_PKG_HOMEPAGE")
                ),
    }

    // if args.shortcuts_path.is_none() {
    //     match args.command {
    //         Commands::Version => {
    //             println!(
    //                 "{} {} by {}, {}",
    //                 std::env::current_exe()
    //                     .unwrap()
    //                     .file_name()
    //                     .unwrap()
    //                     .to_str()
    //                     .unwrap(),
    //                 env!("CARGO_PKG_VERSION"),
    //                 env!("CARGO_PKG_AUTHORS"),
    //                 env!("CARGO_PKG_HOMEPAGE")
    //             );
    //             return Ok(());
    //         }
    //         _ => return Err(Error::InvalidInputFile(String::from(
    //         "<SHORTCUTS_PATH> is missing. It must be an existining file or folder contains .../<shortcuts>.vdf"))),
    //     }
    // }

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
                    let val =
                        TryInto::<$t>::try_into($sc.props.get($c).unwrap_or_default()).unwrap();
                    Some(if *$keys {
                        format!(concat!(stringify!($b), " = ", $d), val)
                    } else {
                        format!($d, val)
                    })
                }
                _ => None,
            },
        )
    };
}

macro_rules! format_column_output_ex {
    ($keys:expr,$all:expr,$b:expr,$c:expr,$d:expr) => {
        once(
            match if $all != &ListColumnsModes::None {
                $all
            } else {
                $b
            } {
                ListColumnsModes::Plain => {
                    let val = $c;
                    Some(if *$keys {
                        format!(concat!(stringify!($b), " = ", $d), val)
                    } else {
                        format!($d, val)
                    })
                }
                _ => None,
            },
        )
    };
}

fn output_list(args: &Cli, scs: &Shortcuts) {
    match &args.command {
        Commands::List {
            separator,
            index,
            appid,
            appname,
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
            keys,..
        } => println!(
            "{}",
            scs.iter()
                .map(|sc| {
                    format_column_output!(sc, keys, all, index, String, "index", "{}")
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
                            sc, keys, all, appid, u32, "appid", "{}"
                        ))
                        .chain(format_column_output!(
                            sc, keys, all, appname, String, "appname", "\"{}\""
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
        ),
        _ => panic!("impossible"),
    }
}

#[derive(Debug)]
enum Error {
    InvalidInputFile(String),
}
