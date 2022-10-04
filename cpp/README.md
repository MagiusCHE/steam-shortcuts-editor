# Steam Shortcuts Editor
## Usage
- see [../README.md](../README.md)
## Develop
### Dependencies: 
- (optional) `yarn`
- `boost 1.79.0+`
- `glibc 2.33+` 
- `cmake 3.24.1+` (yarn script will use `/usr/bin/cmake`)
- `clang 14.0.6+` (yarn script will use `/usn/bin/clang`)
### Compile
- Debug: `yarn build:debug`
  - Artifact will be generated into `build/Debug/steam-shortcuts-editor`
  - Run target in debug: `yarn debug`
- Release: `yarn build:release`
  - Artifact will be generated into `build/Release/steam-shortcuts-editor`

### Run Debug
- `yarn debug`