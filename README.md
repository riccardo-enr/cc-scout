# CC Scout

<img src="assets/cc-scout.svg" alt="CC Scout Trail Compass icon" width="96">

A pet project for learning Linux MIDI programming in C on my Ubuntu PC and
Arch Linux laptop. I use an Akai MPK Mini to try it out. The first milestone
lists ALSA sequencer sources and monitors notes and control changes from one or
more selected ports.

I'm deliberately reimplementing some existing functionality to learn the
ALSA sequencer API. [aseqdump](https://github.com/alsa-project/alsa-utils/tree/master/seq/aseqdump)
already monitors ALSA MIDI events. [mididings](https://das.nasophon.de/mididings/)
and [RouteMIDI](https://github.com/gbevin/RouteMIDI) cover routing and event
transforms.

```text
MPK Mini -> ALSA sequencer -> cc-scout -> Terminal
```

## Build

On Ubuntu, install the build dependencies with:

```sh
sudo apt install build-essential cmake libasound2-dev
```

On Arch Linux, install them with:

```sh
sudo pacman -S --needed base-devel cmake alsa-lib
```

Then configure and build:

```sh
cmake -S . -B build
cmake --build build
```

## Try it with the MPK Mini

Connect the controller and list sources. Use the addresses shown on your system.

```sh
./build/cc-scout list
./build/cc-scout monitor 24:0
```

On the MPK mini IV, keys and pads can arrive on the main MIDI port while knobs
in DAW mode send CC on the DAW port. To watch both in one session:

```sh
./build/cc-scout monitor 24:0 24:1
```

Each line starts with its source address. Press a key or pad to see note events,
or turn a knob to see its controller number and value. Channels are displayed as 1-16. Press Ctrl+C
to stop. The ALSA sequencer must be accessible at `/dev/snd/seq`.

## Future GUI

The GUI will live in a separate repository and follow the desktop's appearance
settings. The core stays independent of GTK and theming. Night Hike is the
project icon. The monitor palette can be decided when the GUI is built.

MIT license. See [LICENSE](LICENSE).
