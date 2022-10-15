/*
 * Copyright (c) 2022, Magius(CHE)
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 * Read the LICENSE file for more details.
 *
 * @author: Magius(CHE) - magiusche@magius.it
 */

use std::{collections::HashMap, fs::File, io::Write, str::from_utf8};

#[derive(Debug)]
pub enum ShortcutProp {
    UInt32(u32),
    String(String),
    Strings(Vec<String>),
    None,
}

impl Default for ShortcutProp {
    fn default() -> Self {
        Self::None
    }
}

impl Default for &ShortcutProp {
    fn default() -> Self {
        &ShortcutProp::None
    }
}

impl TryFrom<&ShortcutProp> for u32 {
    type Error = String;

    fn try_from(u: &ShortcutProp) -> Result<Self, Self::Error> {
        match u {
            ShortcutProp::UInt32(n) => Ok(*n),
            _ => Err(format!(
                "Value out of range. Expected ShortcutProp::UInt32(n) but got {:?}",
                u
            )),
        }
    }
}
impl TryFrom<&ShortcutProp> for String {
    type Error = String;

    fn try_from(u: &ShortcutProp) -> Result<Self, Self::Error> {
        match u {
            ShortcutProp::String(n) => Ok(n.clone()),
            _ => Err(format!(
                "Value out of range. Expected ShortcutProp::String(n) but got {:?}",
                u
            )),
        }
    }
}
impl TryFrom<&ShortcutProp> for Vec<String> {
    type Error = String;

    fn try_from(u: &ShortcutProp) -> Result<Self, Self::Error> {
        match u {
            ShortcutProp::Strings(n) => Ok(n.iter().map(|s| s.clone()).collect()),
            _ => Err(format!(
                "Value out of range. Expected ShortcutProp::Strings(n) but got {:?}",
                u
            )),
        }
    }
}

#[derive(Debug)]
pub struct ShortcutPropInfo {
    pub switchname: &'static str,
    pub type_default: ShortcutProp,
    pub name: &'static str,
    pub pascalcase: &'static str,
    pub order: u32,
}

//pub type SPIForEach = fn(&ShortcutPropInfo);

impl ShortcutPropInfo {
    pub fn new(
        switchname: &'static str,
        pascalcase: &'static str,
        type_default: ShortcutProp,
        order: u32,
    ) -> Self {
        Self {
            switchname,
            type_default,
            name: Box::leak(switchname.to_lowercase().replace("_", "").into_boxed_str()),
            order,
            pascalcase,
        }
    }
}
use byteorder::{LittleEndian, WriteBytesExt};
use lazy_static::lazy_static;

lazy_static! {
    pub static ref SHORTCUT_PROP_INFO: [ShortcutPropInfo; 18] = [
        ShortcutPropInfo::new("index", "Index", ShortcutProp::UInt32(0), 0),
        ShortcutPropInfo::new("app_id", "AppId", ShortcutProp::UInt32(0), 1),
        ShortcutPropInfo::new(
            "app_name",
            "AppName",
            ShortcutProp::String("ERROR".to_owned()),
            2
        ),
        ShortcutPropInfo::new("exe", "Exe", ShortcutProp::String("ERROR".to_owned()), 3),
        ShortcutPropInfo::new(
            "start_dir",
            "StartDir",
            ShortcutProp::String("".to_owned()),
            4
        ),
        ShortcutPropInfo::new("icon", "Icon", ShortcutProp::String("".to_owned()), 5),
        ShortcutPropInfo::new(
            "shortcut_path",
            "ShortcutPath",
            ShortcutProp::String("".to_owned()),
            6
        ),
        ShortcutPropInfo::new(
            "launch_options",
            "LaunchOptions",
            ShortcutProp::String("".to_owned()),
            7
        ),
        ShortcutPropInfo::new("is_hidden", "IsHidden", ShortcutProp::UInt32(0), 8),
        ShortcutPropInfo::new(
            "allow_desktop_config",
            "AllowDesktopConfig",
            ShortcutProp::UInt32(0),
            9
        ),
        ShortcutPropInfo::new("allow_overlay", "AllowOverlay", ShortcutProp::UInt32(0), 10),
        ShortcutPropInfo::new("open_vr", "OpenVR", ShortcutProp::UInt32(0), 11),
        ShortcutPropInfo::new("devkit", "Devkit", ShortcutProp::UInt32(0), 12),
        ShortcutPropInfo::new(
            "devkit_game_id",
            "DevkitGameID",
            ShortcutProp::String("ERROR".to_owned()),
            13
        ),
        ShortcutPropInfo::new(
            "devkit_override_app_id",
            "DevkitOverrideAppID",
            ShortcutProp::UInt32(0),
            14
        ),
        ShortcutPropInfo::new(
            "last_play_time",
            "LastPlayTime",
            ShortcutProp::UInt32(0),
            15
        ),
        ShortcutPropInfo::new(
            "flatpak_app_id",
            "FlatpakAppID",
            ShortcutProp::String("".to_owned()),
            16
        ),
        ShortcutPropInfo::new("tags", "Tags", ShortcutProp::Strings(vec![]), 17),
    ];
}

#[derive(Debug)]
#[repr(u8)]
enum VdfMapItemType {
    Map = 0x00,
    String = 0x01,
    UInt32 = 0x02,
    MapEnd = 0x08,
}

impl TryFrom<u8> for VdfMapItemType {
    type Error = String;

    fn try_from(u: u8) -> Result<Self, Self::Error> {
        match u {
            0x00 => Ok(Self::Map),
            0x01 => Ok(Self::String),
            0x02 => Ok(Self::UInt32),
            0x08 => Ok(Self::MapEnd),
            _ => Err("Value out of range".to_owned()),
        }
    }
}

pub type VdfMap = HashMap<String, Value>;

#[derive(Debug)]
pub enum Value {
    String(String),
    UInt32(u32),
    Map(VdfMap),
}

impl TryFrom<&Value> for u32 {
    type Error = String;

    fn try_from(value: &Value) -> Result<Self, Self::Error> {
        //println!("Convert from {:?}", value);
        match value {
            Value::UInt32(u) => Ok(*u),
            _ => Err(format!("Cannot convert non-UInt32 {:?} into u32", value)),
        }
    }
}

impl TryFrom<&Value> for String {
    type Error = String;

    fn try_from(value: &Value) -> Result<Self, Self::Error> {
        //println!("Convert from {:?}", value);
        match value {
            Value::String(u) => Ok(u.clone()),
            _ => Err(format!("Cannot convert non-String {:?} into String", value)),
        }
    }
}

// impl TryFrom<&Value> for Shortcut {
//     type Error = String;

//     fn try_from(value: &Value) -> Result<Self, Self::Error> {
//         match value {
//             Value::Map(u) => Ok(
//                 TryInto::<Shortcut>::try_into(u).unwrap()),
//             _ => Err(format!("Cannot convert non-Map {:?} into Shortcut", value)),
//         }
//     }
// }

impl TryFrom<&Value> for HashMap<u32, Shortcut> {
    type Error = String;

    fn try_from(value: &Value) -> Result<Self, Self::Error> {
        match value {
            Value::Map(u) => Ok(u
                .iter()
                .map(|(k, v)| match v {
                    Value::Map(s) => (
                        k.parse().unwrap(),
                        TryInto::<Shortcut>::try_into((k.parse().unwrap(), s)).unwrap(),
                    ),
                    _ => todo!(),
                })
                .collect()),
            _ => Err(format!(
                "Cannot convert non-Map {:?} into HashMap<u32, Shortcut>",
                value
            )),
        }
    }
}

impl TryFrom<(u32, &VdfMap)> for Shortcut {
    type Error = String;

    fn try_from((index, map): (u32, &VdfMap)) -> Result<Self, Self::Error> {
        let mut props: HashMap<String, ShortcutProp> = map
            .iter()
            .map(|(k, v)| {
                (
                    // here nees switchanme (and we have vdfname lowercase)
                    String::from(
                        SHORTCUT_PROP_INFO
                            .iter()
                            .find(|e| e.name == k)
                            .unwrap()
                            .switchname,
                    ),
                    match v {
                        Value::String(s) => ShortcutProp::String(s.clone()),
                        Value::UInt32(u) => ShortcutProp::UInt32(*u),
                        Value::Map(_) => {
                            ShortcutProp::Strings(TryInto::<Vec<String>>::try_into(v).unwrap())
                        }
                    },
                )
            })
            .collect();
        props.insert("index".to_owned(), ShortcutProp::UInt32(index));
        Ok(Self { props })
    }
}

impl TryFrom<&Value> for Vec<String> {
    type Error = String;

    fn try_from(value: &Value) -> Result<Self, Self::Error> {
        //println!("Convert from {:?}", value);

        match value {
            Value::Map(u) => Ok(u
                .values()
                .map(|v| TryInto::<String>::try_into(v).unwrap_or("".to_owned()))
                .collect()),
            _ => Err(format!(
                "Cannot convert non-Map {:?} into Vec<String>",
                value
            )),
        }
    }
}

#[derive(Debug)]
pub struct Shortcuts {
    shortcuts: HashMap<u32, Shortcut>,
}

// {
//   "shortcuts": [
//     {
//       "AppName": "Minecraft - FTB",
//       "exe": "\"C:\\Path\\With Space\\To\\some.exe\"",
//       "StartDir": "\"C:\\Path\\With Space\\To\\\"",
//       "IsHidden": false,
//       "icon": null,
//       "AllowDesktopConfig": true,
//       "OpenVR": false,
//       "tags": [
//         "favorite"
//       ]
//     },
//     //.. more shortcut objects
//   ]
// }

impl Shortcuts {
    pub fn from(buffer: &[u8], index: &mut usize) -> Option<Self> {
        if let Some(mut shortcuts) = consume_map(buffer, index) {
            //println!("Next word is {:?}", appid_label);
            //println!("[{index:x}] add shortcut {:?}", &value);

            if !shortcuts.contains_key("shortcuts") {
                println!("Missing header \"shortcuts\"");
                return None;
            }

            if let Some(shortcuts_val) = shortcuts.remove("shortcuts") {
                return Some(Shortcuts {
                    shortcuts: TryInto::<HashMap<u32, Shortcut>>::try_into(&shortcuts_val).unwrap(),
                });
            } else {
                return None;
            }
        }
        None
    }
    pub fn at(&self, index: &u32) -> Option<&Shortcut> {
        self.shortcuts.get(index)
    }

    pub fn at_or_new<F, T>(&mut self, index: &u32, fun: F) -> Result<bool, T>
    where
        F: Fn(bool, &mut Shortcut) -> Result<(), T>,
    {
        let mut new = false;
        if let Some(sc) = self.shortcuts.get_mut(index) {
            fun(new, sc)?;
        } else {
            // Create new one.
            let mut sc = Shortcut::empty();
            new = true;
            fun(new, &mut sc)?;
            self.shortcuts.insert(*index, sc);
        }

        Ok(new)
    }
    pub fn len(&self) -> usize {
        self.shortcuts.len()
    }

    pub fn empty() -> Self {
        Self {
            shortcuts: HashMap::new(),
        }
    }

    pub fn store_into(&self, file: &mut File) -> Result<(), String> {
        write_type(file, VdfMapItemType::Map)?;
        write_string(file, "shortcuts")?;
        for (_, sc) in &self.shortcuts {
            sc.write_into(file)?;
        }
        write_type(file, VdfMapItemType::MapEnd)?;
        write_type(file, VdfMapItemType::MapEnd)?;
        Ok(())
    }
}

fn write_string(file: &mut File, string: &str) -> Result<(), String> {
    match write!(file, "{}\0", string) {
        Err(err) => Err(format!("Error while writing string {}. {:?}", string, err)),
        _ => Ok(()),
    }
}

fn write_u32(file: &mut File, num: &u32) -> Result<(), String> {
    match file.write_u32::<LittleEndian>(*num) {
        Err(err) => Err(format!("Error while writing u32 {}. {:?}", num, err)),
        _ => Ok(()),
    }
}

fn write_type(file: &mut File, mtype: VdfMapItemType) -> Result<(), String> {
    match file.write_all(&[mtype as u8]) {
        Err(err) => Err(format!("Error while writing byte {:?}. {:?}", mtype, err)),
        _ => Ok(()),
    }
}

#[derive(Debug)]
pub struct Shortcut {
    pub props: HashMap<String, ShortcutProp>,
}

impl Shortcuts {
    pub fn iter<'a>(&'a self) -> impl Iterator<Item = &'a Shortcut> + '_ {
        ShortcutIter {
            shortcuts: self,
            index: None,
            size: self.len(),
        }
    }
}

pub struct ShortcutIter<'a> {
    shortcuts: &'a Shortcuts,
    index: Option<usize>,
    size: usize,
}

impl<'a> Iterator for ShortcutIter<'a> {
    type Item = &'a Shortcut;

    fn next(&mut self) -> Option<Self::Item> {
        self.index = Some(match self.index {
            None => 0,
            Some(n) => n + 1,
        });
        if let Some(act) = self.index {
            //println!("Scan iterator {}",act);
            if act >= self.size {
                return None;
            }
            return self.shortcuts.at(&(act as u32));
        }
        None
    }
}

// impl TryFrom<(u32, &VdfMap)> for Shortcut {
//     type Error = String;

//     fn try_from((index, mainmap): (u32, &VdfMap)) -> Result<Self, Self::Error> {
//         //println!("From {:?}", mainmap);
//         let map;
//         if let Some(Value::Map(submap)) = mainmap.get(format!("{}", index).as_str()) {
//             map = submap
//         } else {
//             return Err(format!("Missing shortcut at {}", index));
//         }

//         let mut props: HashMap<String, ShortcutProp> = HashMap::new();

//         props.insert("index".to_owned(), ShortcutProp::UInt32(index));

//         SHORTCUT_PROP_INFO.iter().for_each(|info| {
//             if info.name == "index" {
//                 return;
//             }
//             let val = match &info.type_default {
//                 ShortcutProp::UInt32(def) => ShortcutProp::UInt32(
//                     TryInto::<u32>::try_into(map.get(info.name).unwrap_or(&Value::UInt32(*def)))
//                         .unwrap(),
//                 ),
//                 ShortcutProp::String(def) => ShortcutProp::String(
//                     TryInto::<String>::try_into(
//                         map.get(info.name).unwrap_or(&Value::String(def.to_owned())),
//                     )
//                     .unwrap(),
//                 ),
//                 ShortcutProp::Strings(_) => ShortcutProp::Strings(
//                     TryInto::<Vec<String>>::try_into(
//                         map.get(info.name).unwrap_or(&Value::Map(HashMap::new())),
//                     )
//                     .unwrap(),
//                 ),
//                 _ => unreachable!(),
//             };
//             props.insert(info.switchname.to_owned(), val);
//             //*props.entry(info.switchname).or_insert(val) = val;
//         });

//         Ok(Self { props })
//     }
// }

fn consume_map_item(buffer: &[u8], index: &mut usize) -> Option<(String, Value)> {
    //println!("[{index:x}] consume_map_item");
    let btype = consume_byte(buffer, index)?;
    //println!("[{index:x}]  - type {:x}", btype);
    let btypeinto = btype.try_into();
    match btypeinto {
        Err(_) => {
            println!("[{index:x}]  - type {:x} is invalid.", btype);
            return None;
        }
        Ok(VdfMapItemType::MapEnd) => return None,
        _ => (),
    };

    let name = consume_string(buffer, index)?;
    //println!("[{index:x}]  - Key = {:?}", name);
    match btypeinto {
        Ok(VdfMapItemType::Map) => Some((name, Value::Map(consume_map(buffer, index)?))),
        Ok(VdfMapItemType::String) => Some((name, Value::String(consume_string(buffer, index)?))),
        Ok(VdfMapItemType::UInt32) => Some((name, Value::UInt32(consume_u32(buffer, index)?))),
        _ => None,
    }
}

fn consume_map(buffer: &[u8], index: &mut usize) -> Option<VdfMap> {
    let mut map = VdfMap::new();
    while let Some((key, value)) = consume_map_item(buffer, index) {
        map.insert(key.to_lowercase(), value);
    }
    Some(map)
}

fn consume_u32(buffer: &[u8], index: &mut usize) -> Option<u32> {
    Some(
        ((consume_byte(buffer, index)? as u32) << 0)
            + ((consume_byte(buffer, index)? as u32) << 8)
            + ((consume_byte(buffer, index)? as u32) << 16)
            + ((consume_byte(buffer, index)? as u32) << 24),
    )
}

fn consume_string(buffer: &[u8], index: &mut usize) -> Option<String> {
    let mut word = String::new();
    //println!("[{:x}] Consume string starts", *index);
    loop {
        //we need to handle utf-8 here

        match consume_byte(buffer, index) {
            Some(0) => break,
            Some(c) => {
                if c >= 128 {
                    //utf-8
                    let mut utf8_arr = vec![c];
                    loop {
                        if let Some(b) = peek_byte(buffer, *index) {
                            if b < 128 {
                                break;
                            } else {
                                utf8_arr.push(b);
                                *index += 1;
                            }
                        } else {
                            println!(
                                "[{:x}] Invalid UTF8 chars. Word was: {:?}",
                                *index - 1,
                                &word
                            );
                            return None;
                        }
                    }

                    if utf8_arr.len() <= 1 {
                        println!(
                            "[{:x}] Invalid UTF8 chars. Word was: {:?}",
                            *index - 1,
                            &word
                        );
                        return None;
                    }
                    if let Ok(st) = from_utf8(&utf8_arr) {
                        word.push_str(st);
                    } else {
                        println!(
                            "[{:x}] Invalid UTF8 chars. Word was: {:?}",
                            *index - 1,
                            &word
                        );
                        return None;
                    }
                } else {
                    //println!("[{:x}] Consume string push {:x} to {:?}", *index, c, word);
                    word.push(c as char)
                }
            }
            _ if word.len() > 0 => break,
            _ => return None,
        }
    }
    //println!("[{:x}] Consumed string {:?}", *index,word);
    Some(word)
}

fn consume_byte(buffer: &[u8], index: &mut usize) -> Option<u8> {
    let c = peek_byte(buffer, *index);
    if c.is_some() {
        *index += 1;
        return c;
    }
    return None;
}

fn peek_byte(buffer: &[u8], index: usize) -> Option<u8> {
    if index >= buffer.len() {
        return None;
    }
    Some(buffer[index])
}

impl Shortcut {
    pub fn empty() -> Self {
        Self {
            props: HashMap::new(),
        }
    }

    pub fn prop_to_string(&self, key: &str) -> Option<String> {
        Some(match &self.props[key] {
            ShortcutProp::UInt32(n) => format!("{}", n),
            ShortcutProp::String(n) => n.clone(),
            ShortcutProp::Strings(n) => match serde_json::to_string(&n) {
                Ok(v) => v,
                Err(_) => return None,
            },
            ShortcutProp::None => unreachable!(),
        })
    }

    pub fn prop_to_u32(&self, key: &str) -> Option<u32> {
        match &self.props[key] {
            ShortcutProp::UInt32(n) => Some(*n),
            _ => None,
        }
    }

    fn write_into(&self, file: &mut File) -> Result<(), String> {
        write_type(file, VdfMapItemType::Map)?;
        write_string(
            file,
            self.prop_to_string("index")
                .map_or(Err("Missing property Index".to_owned()), |f| Ok(f))?
                .as_str(),
        )?;
        for i in 0..SHORTCUT_PROP_INFO.len() {
            let prop_opt = SHORTCUT_PROP_INFO.iter().find(|e| e.order == i as u32);
            if prop_opt.is_none() {
                return Err(format!("Missing SHORTCUT_PROP_INFO with order {}", i));
            }
            let prop = prop_opt.unwrap();
            if prop.switchname == "index" {
                continue;
            }
            match &prop.type_default {
                ShortcutProp::UInt32(_) => write_type(file, VdfMapItemType::UInt32),
                ShortcutProp::String(_) => write_type(file, VdfMapItemType::String),
                ShortcutProp::Strings(_) => write_type(file, VdfMapItemType::Map),
                ShortcutProp::None => unreachable!(),
            }?;

            write_string(file, prop.pascalcase)?;

            match &prop.type_default {
                ShortcutProp::UInt32(def) => write_u32(
                    file,
                    &self.prop_to_u32(prop.switchname).or(Some(*def)).unwrap(),
                ),
                ShortcutProp::String(def) => write_string(
                    file,
                    self.prop_to_string(prop.switchname)
                        .map_or(def.clone(), |s| s)
                        .as_str(),
                ),
                ShortcutProp::Strings(_) => {
                    if let ShortcutProp::Strings(arr) = &self.props[prop.switchname] {
                        for (n, s) in arr.iter().enumerate() {
                            write_type(file, VdfMapItemType::String)?;
                            write_string(file, format!("{}", n).as_str())?;
                            write_string(file, escape_json_string(s).as_str())?;
                        }
                        write_type(file, VdfMapItemType::MapEnd)?;
                    };
                    Ok(())
                }
                ShortcutProp::None => unreachable!(),
            }?;
        }

        write_type(file, VdfMapItemType::MapEnd)?;
        Ok(())
    }
}

fn escape_json_string(s: &str) -> String {
    s.replace("\\", "\\\\").replace("\"", "\\\"")
}
