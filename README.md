# My GameTank Game

Build it with the "make" command

Import art/music by adding it to a folder inside the "assets" directory, and then run "make import" to update generated project files.

If you have pulled and built the GameTankEmulator repo in an adjacent folder, run "make emulate" to test the game.

src/main.c contains the code that will run after the console powers on


## project.json

This file contains configuration for your project, such as tile, romfile name, and enabled modules.

Several modules are enabled by default, but if you find yourself not using them and wanting more ROM space you can remove their entries from the modules array.

This takes effect after a `make clean && make import`

If you disable the music system and don't replace its functions with something, you should add `AUDIO_NONE` to the list.

Additionally, if you want to use the legacy MIDI-based music workflow you can replace `SDK_TRACKER_FM_BACKEND` with `ENABLE_MODULE_LEGACY_MIDI_FM_BACKEND`

## Build requirements:

EXPERIMENTAL: You can hopefully install cc65 and Zopfli into the SDK directory with 'make prereqs'. Use your package manager if you need to install nodeJS

* cc65 https://cc65.github.io/

  * recommended to use a snapshot build or build from source as their last tagged release and apt package are outdated

* NodeJS https://nodejs.org/

* Zopfli https://github.com/google/zopfli

* GNU Make

## Testing requirements:

* GameTank Emulator https://github.com/clydeshaffer/GameTankEmulator

## Other useful tools:

 * Aseprite for graphics

 * Multi-Firmware Tracker to make music for the new music system https://github.com/clydeshaffer/multi-firmware-tracker

 * gt-sound-sculptor to make sound effects [https://nickgirardo.com/gt-sound-sculptor/index.html](https://nickgirardo.com/gt-sound-sculptor/index.html?sfx=R1RTU3YwEACCAIuKiYkIkEjPB3dlVUQyIhEJ4ixKaEbkIzA*)


## Contributor modules:

Extensions to the SDK that can be dropped into the "modules" folder:

* [jaythomas/gttAudio](https://codeberg.org/jaythomas/gttAudio) - wavetable music and tracker
