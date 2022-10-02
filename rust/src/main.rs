use chrono::{DateTime, NaiveDateTime, Utc};
use clap::{arg_enum, Parser, Subcommand};
use std::{
    fs::File,
    io::{BufReader, Read},
    iter::once,
    path::Path,
};

mod shortcuts;
use shortcuts::Shortcuts;

/// VDF Shortcuts Editor for Steam Client
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
    List {
        //, default_value_t = " "
        #[clap(long, default_value = " ")]
        /// Table output columns separator
        separator: String,
        #[clap(value_names(&["format"]),long,case_insensitive = true, default_value_t = ListColumnsModes::Plain, possible_values(ListColumnsModes::variants()))]
        /// AppID with specified format
        app_id: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::Plain, possible_values(ListColumnsModes::variants()))]
        /// AppName with specified format
        app_name: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// Exe with specified format
        exe: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// Icon with specified format
        icon: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// AllowDesktopConfig with specified format
        allow_desktop_config: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// AllowOverlay with specified format
        allow_overlay: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// Devkit with specified format
        devkit: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// DevkitGameId with specified format
        devkit_game_id: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// DevkitOverrideAppId with specified format
        devkit_override_app_id: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// FlatpakAppId with specified format
        flatpak_app_id: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// IsHidden with specified format
        is_hidden: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// LastPlayTime with specified format
        last_play_time: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// LastPlayTime in "YYYY/MM/DD, hh:mm:ss UTC" with specified format
        last_play_time_utc: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// LastPlayTime in "YYYY/MM/DD, hh:mm:ss" (Localtime) with specified format
        last_play_time_fmt: ListColumnsModes,
        
        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// LastPlayTime in ISO with specified format
        last_play_time_iso: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// LaunchOptions with specified format
        launch_options: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// OpenVR with specified format
        open_vr: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// ShortcutPath with specified format
        shortcut_path: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// StartDir with specified format
        start_dir: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// Tags with specified format
        tags: ListColumnsModes,

        #[clap(value_names(&["format"]),long, case_insensitive = true, default_value_t = ListColumnsModes::None, possible_values(ListColumnsModes::variants()))]
        /// Override all columns format with the specified one
        all: ListColumnsModes,
    },
}

arg_enum! {
    #[derive(Debug, PartialEq, Eq)]
    enum ListColumnsModes {
        None,
        Plain,
        //Padded,
    }
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
        match &args.command {
            Commands::List { .. } => output_list(&args, &scs),
        }
    }

    Ok(())
}

macro_rules! format_column_output {
    ($a:expr,$b:expr,$c:expr,$d:expr) => {
        once(
            match if $a != &ListColumnsModes::None {
                $a
            } else {
                $b
            } {
                ListColumnsModes::Plain => Some(format!($d, $c)),
                _ => None,
            },
        )
    };
}

fn output_list(args: &Args, scs: &Shortcuts) {
    let Commands::List {
        separator,
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
    } = &args.command;

    println!(
        "{}",
        scs.iter()
            .map(|sc| {
                format_column_output!(all, app_id, sc.appid, "{}")
                    .chain(format_column_output!(all, app_name, sc.appname, "{:?}"))
                    .chain(format_column_output!(all, exe, sc.exe, "{:?}"))
                    .chain(format_column_output!(all, icon, sc.icon, "{:?}"))
                    .chain(format_column_output!(
                        all,
                        allow_desktop_config,
                        sc.allow_desktop_config,
                        "{}"
                    ))
                    .chain(format_column_output!(
                        all,
                        allow_overlay,
                        sc.allow_overlay,
                        "{}"
                    ))
                    .chain(format_column_output!(all, devkit, sc.devkit, "{}"))
                    .chain(format_column_output!(
                        all,
                        devkit_override_app_id,
                        sc.devkit_override_app_id,
                        "{}"
                    ))
                    .chain(format_column_output!(
                        all,
                        flatpak_app_id,
                        sc.flatpak_app_id,
                        "{}"
                    ))
                    .chain(format_column_output!(
                        all,
                        devkit_game_id,
                        sc.devkit_game_id,
                        "{}"
                    ))
                    .chain(format_column_output!(all, is_hidden, sc.is_hidden, "{}"))
                    .chain(format_column_output!(
                        all,
                        last_play_time,
                        sc.last_play_time,
                        "{}"
                    ))
                    .chain(format_column_output!(
                        all,
                        last_play_time_utc,
                        DateTime::<Utc>::from_utc(NaiveDateTime::from_timestamp(sc.last_play_time as i64, 0),Utc),
                        "\"{}\""
                    ))
                    .chain(format_column_output!(
                        all,
                        last_play_time_fmt,
                        NaiveDateTime::from_timestamp(sc.last_play_time as i64, 0),
                        "\"{}\""
                    ))
                    .chain(format_column_output!(
                        all,
                        last_play_time_iso,
                        NaiveDateTime::from_timestamp(sc.last_play_time as i64, 0),
                        "{:?}"
                    ))
                    .chain(format_column_output!(
                        all,
                        launch_options,
                        sc.launch_options,
                        "{:?}"
                    ))
                    .chain(format_column_output!(all, open_vr, sc.open_vr, "{}"))
                    .chain(format_column_output!(
                        all,
                        shortcut_path,
                        sc.shortcut_path,
                        "{:?}"
                    ))
                    .chain(format_column_output!(all, start_dir, sc.start_dir, "{:?}"))
                    .chain(format_column_output!(all, tags, sc.tags, "{:?}"))
                    // shortcut_path,
                    // start_dir,
                    // tags
                    .filter_map(|e| e)
                    .collect::<Vec<String>>()
                    .join(separator)
            })
            .collect::<Vec<String>>()
            .join("\n")
    );
}

#[derive(Debug)]
enum Error {
    InvalidInputFile(String),
}