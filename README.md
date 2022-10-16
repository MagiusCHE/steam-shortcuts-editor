# Steam Shortcuts Editor
## Usage
```
steam-shortcuts-editor list ./shortcuts.vdf
```
will output:
```
4173909563 "Super Mario Odyssey"
3808027925 "Minecraft"
2900228243 "Garden Paws"
3342111099 "Animal Crossing: New Horizons"
3134436784 "DRAGON BALL FighterZ"
3702413278 "Mario Kart 8 Deluxe"
2817442984 "Mario Party Superstars"
3047893182 "Minecraft: Nintendo Switch Edition"
...
```
```
steam-shortcuts-editor edit ./shortcuts.vdf --json-path ./shortcuts.json --out ./shortcuts_dest.vdf --force
```
will load `./shortcuts.json` and use it to update `./shortcuts.vdf` and write the results to `./shortcuts_dest.vdf` (overwriting)
```
steam-shortcuts-editor edit ./shortcuts.vdf --idx 0 --key app_name --val "My Custom Game" --out ./shortcuts_dest.vdf
```
will update `./shortcuts.vdf` modifying the entry 0 and change `appname` into `My Custom Game`. It writes the results into `./shortcuts_dest.vdf` (it fails if `./shortcuts_dest.vdf` already exists).
## Commands
- **list**: List entries summary info. Features:
  - Table output with selectable columns.
  - JSON output.
- **edit**: Update entries structure recreating .vdf shortcuts file. Features:
  - Load existing shortucts.vdf and modify it
  - Edit via multiple invokations to modify one single entry.prop.value per invoke.
  - Load JSON file as source to update or add new entries.
  - Save output into vdf file.
  - Overwriting protection

## Help
Execute with `help` for more info.

## Build / Develop
- Cpp: [cpp/README.md](cpp/README.md)
- Rust: [rust/README.md](rust/README.md)
