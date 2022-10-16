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
### List
Show all shourtcuts summary.
```
steam-shortcuts-editor ./shortcuts.vdf list --tags=Plain
```
will output:
```
4173909563 "Super Mario Odyssey" ["Nintendo Switch"]
3808027925 "Minecraft" []
2900228243 "Garden Paws" []
3342111099 "Animal Crossing: New Horizons" ["Nintendo Switch"]
3134436784 "DRAGON BALL FighterZ" ["Nintendo Switch"]
3702413278 "Mario Kart 8 Deluxe" ["Nintendo Switch"]
2817442984 "Mario Party Superstars" ["Nintendo Switch"]
3047893182 "Minecraft: Nintendo Switch Edition" ["Nintendo Switch"]
2946221396 "Super Mario™ 3D World + Bowser’s Fury" ["Nintendo Switch"]
...
```
```
steam-shortcuts-editor ./shortcuts.vdf list --app-name=None --last-play-time=Plain
```
will output ID and Unix Time:
```
4173909563 1663269322
3808027925 1663953835
2900228243 1663233136
3342111099 1664209048
3134436784 1663271614
3702413278 0
2817442984 0
3047893182 0
...
```
## Help
Execute with `help` for more info.

## Build / Develop
- Cpp: [cpp/README.md](cpp/README.md)
- Rust: [rust/README.md](rust/README.md)
