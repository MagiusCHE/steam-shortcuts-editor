use std::{collections::HashMap, str::from_utf8};

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

// impl TryInto<u32> for Value {
//     type Error = String;

//     fn try_into(self) -> Result<u32, Self::Error> {
//         match self {
//             Value::UInt32(u) => Ok(u),
//             _ => Err(format!("Cannot convert non-UInt32 {:?} into u32", self)),
//         }
//     }
// }

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
    shortcuts: Value,
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

            return Some(Shortcuts {
                shortcuts: shortcuts.remove("shortcuts").unwrap(),
            });
        }
        None
    }
    pub fn at(&self, index: usize) -> Option<Shortcut> {
        if let Value::Map(m) = &self.shortcuts {
            if let Ok(ret) = TryInto::<Shortcut>::try_into((index.to_string().as_str(), m)) {
                return Some(ret);
            }
        }
        None
    }
    pub fn len(&self) -> usize {
        if let Value::Map(m) = &self.shortcuts {
            return m.len();
        }
        0
    }
}

#[derive(Debug)]
pub struct Shortcut {
    pub index: String,
    pub devkit_game_id: String,
    pub open_vr: u32,
    pub launch_options: String,
    pub exe: String,
    pub icon: String,
    pub devkit: u32,
    pub flatpak_app_id: String,
    pub start_dir: String,
    pub allow_desktop_config: u32,
    pub appname: String,
    pub appid: u32,
    pub shortcut_path: String,
    pub is_hidden: u32,
    pub allow_overlay: u32,
    pub devkit_override_app_id: u32,
    pub tags: Vec<String>,
    pub last_play_time: u32,
}

impl Shortcuts {
    pub fn iter(&self) -> impl Iterator<Item = Shortcut> + '_ {
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
    type Item = Shortcut;

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
            return self.shortcuts.at(act);
        }
        None
    }
}

macro_rules! copy_shortcut_param {
    ($m:expr,$l:ty,$ll:expr,$n:expr,$d:expr) => {
        TryInto::<$l>::try_into($m.get($n).unwrap_or(&$ll($d))).unwrap()
    };
}

impl TryFrom<(&str, &VdfMap)> for Shortcut {
    type Error = String;

    fn try_from((index, mainmap): (&str, &VdfMap)) -> Result<Self, Self::Error> {
        //println!("From {:?}", mainmap);
        let map;
        if let Some(Value::Map(submap)) = mainmap.get(index) {
            map = submap
        } else {
            return Err(format!("Missing shortcut at {}", index));
        }
        Ok(Self {
            index: index.to_owned(),
            devkit_game_id: copy_shortcut_param!(
                map,
                String,
                Value::String,
                "DevkitGameID",
                "".to_owned()
            ),
            open_vr: copy_shortcut_param!(map, u32, Value::UInt32, "OpenVR", 0),
            launch_options: copy_shortcut_param!(
                map,
                String,
                Value::String,
                "LaunchOptions",
                "".to_owned()
            ),
            exe: copy_shortcut_param!(map, String, Value::String, "exe", "".to_owned()),
            icon: copy_shortcut_param!(map, String, Value::String, "icon", "".to_owned()),
            devkit: copy_shortcut_param!(map, u32, Value::UInt32, "Devkit", 0),
            flatpak_app_id: copy_shortcut_param!(
                map,
                String,
                Value::String,
                "FlatpakAppID",
                "".to_owned()
            ),
            start_dir: copy_shortcut_param!(map, String, Value::String, "StartDir", "".to_owned()),
            allow_desktop_config: copy_shortcut_param!(
                map,
                u32,
                Value::UInt32,
                "AllowDesktopConfig",
                0
            ),
            appname: copy_shortcut_param!(
                map,
                String,
                Value::String,
                "appname",
                "ERROR".to_owned()
            ),
            appid: copy_shortcut_param!(map, u32, Value::UInt32, "appid", 0),
            shortcut_path: copy_shortcut_param!(
                map,
                String,
                Value::String,
                "ShortcutPath",
                "".to_owned()
            ),
            is_hidden: copy_shortcut_param!(map, u32, Value::UInt32, "IsHidden", 0),
            allow_overlay: copy_shortcut_param!(map, u32, Value::UInt32, "AllowOverlay", 0),
            devkit_override_app_id: copy_shortcut_param!(
                map,
                u32,
                Value::UInt32,
                "DevkitOverrideAppID",
                0
            ),
            tags: copy_shortcut_param!(map, Vec<String>, Value::Map, "tags", HashMap::new()),
            last_play_time: copy_shortcut_param!(map, u32, Value::UInt32, "LastPlayTime", 0),
        })
    }
}

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
        map.insert(key, value);
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
                    word.push(c as char)
                }
            }
            _ if word.len() > 0 => break,
            _ => return None,
        }
    }
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
