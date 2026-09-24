# My GameTank Game

Build it with the "make" command

Import art/music by adding it to a folder inside the "assets" directory, and then run "make import" to update generated project files.

If you have pulled and built the GameTankEmulator repo in an adjacent folder, run "make emulate" to test the game.

src/main.c contains the code that will run after the console powers on

## Build requirements:

EXPERIMENTAL: You can hopefully install cc65 and Zopfli into the SDK directory with 'make prereqs'. Use your package manager if you need to install nodeJS

* cc65 https://cc65.github.io/

  * recommended to use a snapshot build or build from source as their last tagged release and apt package are outdated

* NodeJS https://nodejs.org/

* Zopfli https://github.com/google/zopfli

* GNU Make


## Testing requirements:

* GameTank Emulator https://github.com/clydeshaffer/GameTankEmulator


## gt-tracker for music:

Adds support for music tracks created using gt-tracker (`gtt`). To enable, add `"GTTAUDIO"` to the `modules` list in `project.json` and run `make import`.

Export your track from gt-tracker (see its [README](https://github.com/dwbrite/gametank-sdk/blob/61755e530b12172f45243345dd515fd37f819feb/src/bin/gtt/README.md)). Each folder under `assets/` becomes its own ROM bank, so copy the track's `<name>-export/<name>.bin` file into a folder named after the track, e.g. `assets/mytrack/mytrack.bin`, and the `<name>-export/instruments` folder into `assets/`.

Run `make import` to regenerate `src/gen/assets/mytrack.h` and `src/gen/assets/instruments.h`, which declare the `ASSET__mytrack__*_ptr`/`ASSET__instruments__*_ptr` symbols and `BANK_mytrack`/`BANK_instruments`.

Wire it up in `src/main.c`:

```c
#include "gt/feature/gttAudio/wavetable_audio.h"
#include "gt/feature/gttAudio/track_sequencer.h"
#include "gen/assets/mytrack.h"

TrackSequencer sequencer;

void main() {
    init_wavetable_audio();

    track_sequencer_init(&sequencer, ASSET__mysong__mysong_bin_ptr, BANK_mysong);
    // Loads assets/instruments, points each voice at a default
    // wavetable, mutes them, and sets the sample rate register
    track_sequencer_init_voices();

    while (1) {
        track_sequencer_tick(&sequencer);
        // ...rest of the game loop
    }
}
```

Call `track_sequencer_tick()` each frame. It handles switching to `track_bank` while reading pattern data, so it's safe to call regardless of which ROM bank is active elsewhere in the game.
