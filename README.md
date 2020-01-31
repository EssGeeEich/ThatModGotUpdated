# ThatModGotUpdated
A program dedicated to Factorio players who want to know whether their mods have been updated lately.

## Requirements
- Qt 5+ with SSL support
- C++17 compiler

ᶦ ᵘˢᵉ ᵃʳᶜʰ ᵇᵗʷ

## Installation
```bash
git clone --recursive https://github.com/EssGeeEich/ThatModGotUpdated.git
cd ThatModGotUpdated
qmake ThatModGotUpdated.pro
make

# Optional. Might need root.
make install
```

## Usage

## Help Text
```./ThatModGotUpdated -h```
```
Usage: ./ThatModGotUpdated [options]
Factorio WebApi Commandline Tool

Options:
  -h, --help                           Displays help on commandline options.
  --help-all                           Displays help including Qt specific
                                       options.
  -u, --user <user>                    Placeholder for Username.
  -t, --token <token>                  Placeholder for Token.
  -p, --password <password>            Placeholder for Password.
  -m, --mod <mod>                      Look up this mod's last update. May use
                                       multiple values.
  -d, --disabled-file <disabled-file>  Look up the mods' last update time
                                       extracting a list from a mod-list.json
                                       file (disabled entries only).
  -e, --enabled-file <enabled-file>    Look up the mods' last update time
                                       extracting a list from a mod-list.json
                                       file (enabled entries only).
  -a, --after <after>                  List mods that have a release dated
                                       after this DateTime. Format:
                                       yyyy-MM-ddTHH:mm:ss.zzz000Z.
  -b, --before <before>                List mods that have a release dated
                                       before this DateTime. Format:
                                       yyyy-MM-ddTHH:mm:ss.zzz000Z.
  -q, --quiet                          Stop stdout. Ends program ASAP, might
                                       check fewer mods. Only useful for its
                                       return code. Overrides the verbose flag.
  -s, --single                         Change behavior of the program's exit
                                       code. Return exit code 0 if any mod
                                       matches the filter. Default: Require all
                                       mods to match filter.
  -v, --verbose                        List all the mods that have succeeded or
                                       failed the check.
```

## Examples:
Check if a mod has been updated after a certain date.
```bash
./ThatModGotUpdated -m space-exploration -a 2020-01-29T00:00:00.000000Z
```

Check if all your currently disabled mods have been updated after a certain date, listing even the mods that failed the check.
```bash
./ThatModGotUpdated -d ~/.factorio/mods/mod-list.json -v -a 2020-01-29T00:00:00.000000Z
```

Check if each and every mod of yours have been updated after a certain date. Quick true/false check.
```bash
./ThatModGotUpdated -d ~/.factorio/mods/mod-list.json -e ~/.factorio/mods/mod-list.json -a 2020-01-29T00:00:00.000000Z -q
# Expected output: 0 if ALL mods have been updated. 1 otherwise.
echo $?
```

Check if at least one mod of yours have been updated after a certain date. Quick true/false check.
```bash
./ThatModGotUpdated -d ~/.factorio/mods/mod-list.json -e ~/.factorio/mods/mod-list.json -a 2020-01-29T00:00:00.000000Z -q -s
# Expected output: 0 if ANY mod has been updated. 1 otherwise.
echo $?
```
