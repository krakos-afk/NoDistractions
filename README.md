# nodistactions

A minimal anime-themed GTK work timer with session logging, character panel, and editable history.
Made this for personal session tracking, feel free to use, additional productivity features will be added overtime. 

## Requirements
- C++17 compiler (tested with g++)
- GTKmm 3.x development packages
  - Debian/Ubuntu: `sudo apt install libgtkmm-3.0-dev`
  - Fedora: `sudo dnf install gtkmm30-devel`

## Build
```bash
make
```
This produces the `nodistactions` binary.

## Run
```bash
./nodistactions
```

## Install
```bash
sudo make install
```
Installs the binary to `/usr/local/bin/nodistactions` and assets (theme + character images) to `/usr/local/share/nodistactions` by default (override with `PREFIX=/some/path`).

## Uninstall
```bash
sudo make uninstall
```
Removes the installed binary from the prefix.

## Notes
- Session logs are written to `work_log.txt` in the working directory.
- Character images are loaded in order if present: `character0/1/2.(png|jpg)` then `character.(png|jpg)`.
- UI theme and layout are defined in `style.css`.
- At runtime the app looks for assets in the current working directory first, then in the installed data dir (`/usr/local/share/nodistactions` by default). This lets you run the binary from anywhere while still picking up the packaged theme/images.
