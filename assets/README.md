# Assets folder

Create a subfolder and add .bmp, .mid., or .json files to import them into the project.

Files placed inside the same folder together will be grouped inside a 16K memory bank.
If the combined size after conversion of the files is larger than 16K you will need to move some into a new folder.

A 2MB cartridge has 128 16K memory banks, remember to save some for your program code!

## Default Assets

The "sdk_default" folder contains assets used by SDK functions:

### bios8.bmp
Used for the text drawing features in gt/feature/text. It can be removed if the "TEXT" module is disabled in `project.json`