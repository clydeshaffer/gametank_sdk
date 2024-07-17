const fs = require('fs');
const path = require('path');
const argv = require('minimist')(process.argv.slice(2));
const BMP = require('bitmap-manipulation');

const inFileName = argv._[0];
const outFileName = argv._.length == 2 ?
    argv._[1] :
    inFileName.split(".").slice(0, -1).join(".") + ".gtg";

//Input must either be 128 or 256 px wide
function splitV(inputImage) {
    if(inputImage.width <= 128) {
        return [inputImage];
    }
    var outImages = [
        new BMP.BMPBitmap(128, inputImage.height, 1),
        new BMP.BMPBitmap(128, inputImage.height, 1)
    ];
    outImages[0].drawBitmap(inputImage, 0, 0, null, 0, 0, 128, inputImage.height);
    outImages[1].drawBitmap(inputImage, 0, 0, null, 128, 0, 128, inputImage.height);
    return outImages;
}

//Input can be at most 256 pixels high
function splitH(inputImage) {
    if(inputImage.height <= 128) {
        return [inputImage];
    }
    var outImages = [
        new BMP.BMPBitmap(inputImage.width, 128, 1),
        new BMP.BMPBitmap(inputImage.width, inputImage.height - 128, 1)
    ];
    outImages[0].drawBitmap(inputImage, 0, 0, null, 0,                   0, outImages[0].width, outImages[0].height);
    outImages[1].drawBitmap(inputImage, 0, 0, null, 0, outImages[0].height, outImages[1].width, outImages[1].height);
    return outImages;
}

//Take a 256xN image and split into two or four sections
function splitImage(inputImage) {
    return [inputImage].flatMap(splitH).flatMap(splitV);
}

function flipV(inputImage) {
    let flippedImage = new BMP.BMPBitmap(inputImage.width, inputImage.height, 1);
    for(var i = 0; i < spriteSheet.getHeight(); i++) {
        flippedImage.drawBitmap(inputImage, 0, i, null, 0, inputImage.height - i - 1, inputImage.width, inputImage.height);
    }
    return flippedImage;
}

const randchars = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNNM1234567890";
function rndStr(len) {
    var out = "";
    for(var i = 0; i < len; i++) {
        out += randchars[Math.floor(Math.random() * randchars.length)];
    }
    return out;
}

function saveHeadless(img, filename) {
    var tmpname = rndStr(16) + ".tmp";
    var pixelCount = img.width * img.height;
    img.save(tmpname);
    const tmpFileSize = fs.statSync(tmpname).size;

    const parentDir = path.dirname(filename);
    if(!fs.existsSync(parentDir)) {
        fs.mkdirSync(parentDir, {recursive : true});
    }

    const outputStream = fs.createWriteStream(filename);
    outputStream.on("finish", () => {
        fs.unlinkSync(tmpname);
    });
    fs.createReadStream(tmpname, { start : (tmpFileSize - pixelCount)}).pipe(outputStream);
}

let spriteSheet = BMP.BMPBitmap.fromFile(inFileName);
let outSheets = splitImage(spriteSheet);
if(outSheets.length == 1) {
    saveHeadless(flipV(outSheets[0]), outFileName);
    console.log(outFileName);
} else {
    var sheetNameTuples = outSheets.map((os, index) => {
        const nameNoExt = outFileName.split('.').slice(0, -1).join('.');
        return {
            name : (index === 0) ? outFileName : nameNoExt + '_' + index + '.gtg',
            sheet : os
        };
    });
    sheetNameTuples.forEach((tup) => {
        saveHeadless(flipV(tup.sheet), tup.name);
    });
    console.log(sheetNameTuples.map((tup) => tup.name).join(' '));
}
