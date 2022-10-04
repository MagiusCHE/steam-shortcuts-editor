# Steam Shortcuts Editor
## Usage
- see [../README.md](../README.md)
## Develop
### Dependencies: 
- (optional) `yarn`
Steam Deck actually has glibc 2.33 and we need 2.34.
- `rustup target add x86_64-unknown-linux-musl` 
- or `yarn install`
### Compile
- Debug: `cargo build` or `yarn build:debug`
  - Artifact will be generated into `target/debug/steam-shortcuts-editor`
  - Run target in debug: `cargo run` or `yarn debug`
- Release: `cargo build --release` or `yarn build:release`
  - Artifact will be generated into `target/x86_64-unknown-linux-musl/release/steam-shortcuts-editor`