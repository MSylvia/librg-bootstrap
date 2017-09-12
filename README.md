# librg-bootstrap
This is a ready-to-use bootstrap project to give a quickstart to librg freshmen.

## Setup

### Automatic (highly recommended)

1. Clone the repository and execute `npm install` in the repo dir.
2. Create build directory and execute `cmake <flags> ../` inside.
3. The rest is up to you. ;)

### Manual

1. Clone this repository.
2. Clone all dependencies into a subdirectory (let's say, "vendors").
3. Edit `CMakeLists.txt` (or pass cmake flags equivalently) and edit `LIBRG_VENDOR_FOLDER` and `LIBRG_POSTFIX` to match your vendors's folder structure.
4. Create build directory and execute `cmake <flags> ../` inside.
5. Once again, the rest is up to you. ;)

## License

This repository is dual-licensed to either the public domain or under the following: `you are granted a perpetual, irrevocable license to copy, modify,
    publish, and distribute this repository as you see fit. NO WARRANTY IS IMPLIED, USE AT YOUR OWN RISK!`
