const fs = require('fs');
const path = require('path');
const argv = require('minimist')(process.argv.slice(2));
const BMP = require('bitmap-manipulation');

const inFileName = argv._[0];

if(inFileName == undefined || inFileName.toLowerCase() == 'help') {
    console.log("Usage: node bitfont.js myfont.bmp");
    console.log("Prints to the console (or file pipe) a replacement for src/gt/feature/bittext/font.c");
    console.log("Requires an indexed bitmap. Color zero becomes background, all other colors foreground");
    return;
}

const fontImage = BMP.BMPBitmap.fromFile(inFileName);

const arrayBytes = [];
const height = Math.min(fontImage.height, 8);

for(var x = 0; x < fontImage.width; ++x) {
    var outbyte = 0;
    for(var y = height-1; y >= 0; y--) {
        outbyte = outbyte << 1;
        if(fontImage.getPixel(x, y) != 0) {
            outbyte += 1;
        }
    }
    arrayBytes.push(outbyte);
}

console.log(`#include "../../../gen/modules_enabled.h"

#ifdef ENABLE_MODULE_BITTEXT

#pragma rodata-name("sdk_default")

const unsigned char system_font_monochrome[${arrayBytes.length}] = {${
    arrayBytes.map((byte, idx) => {
    if(idx % 12 == 0) return "\n\t0x" + byte.toString(16).padStart(2,'0');
    else return "0x" + byte.toString(16).padStart(2, '0');
}).join(", ")
}
};
#endif`
);
