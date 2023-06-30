const fs = require('fs');
const parser = require('xml2json');
const zlib = require('zlib');

const argv = require('minimist')(process.argv.slice(2));
const inFileName = argv._[0];
const outFileName = argv._.length == 2 ?
    argv._[1] :
    inFileName.split(".").slice(0, -1).join(".") + ".bin";

function writeNumberInMiddle(targetStr, srcStr) {
    const header = targetStr.slice(0,-(16 + srcStr.length));
    const footer = targetStr.slice(-16);
    return header+srcStr+footer;
}

fs.readFile(inFileName, function(err, data) {
    const json = parser.toJson(data, {
        trim: false
    });
    const jsonObj = JSON.parse(json);
    const levels = jsonObj.SokobanLevels.LevelCollection.Level;
    let totalLevels = 0;
    let totalSize = 0;
    const convertedLevels = levels.flatMap((level, index) => {
        if(totalSize < 16384) { 
            if(level.Width <= 16 && level.Height <= 16) {
                ++totalLevels;
                const widthDiff = 16 - level.Width;
                const heightDiff = 16 - level.Height;
                const leftPad = Math.floor(widthDiff/2);

                const padStringLeft = "".padStart(leftPad, " ");

                const topPad = Math.floor(heightDiff/2);
                const levelBuf = Buffer.alloc(256);
                const levelNumStr = totalLevels.toString();
                const paddedLevel2 = 
                    [ ...Array.from(Array(topPad).keys()).map(()=>"".padStart(16, " ")),
                    ...level.L.map((row) => (padStringLeft + row).padEnd(16)),
                    ...Array.from(Array(heightDiff - topPad).keys()).map(()=>"".padStart(16, " ")) ].join("");
                const paddedLevel = writeNumberInMiddle(paddedLevel2, levelNumStr);
                for(let i = 0; i < paddedLevel.length; i++) {
                    const c = paddedLevel[i];
                    let outchar = 0;
                    switch(c) {
                        case '@':
                        case 'p':
                            outchar = 32;
                            break;
                        case '#':
                            outchar = 166;
                            break;
                        case '.':
                            outchar = 16;
                            break;
                        case '$':
                            outchar = 252;
                            break;
                        case 'B':
                        case '*':
                            outchar = 253;
                            break;
                        case 'P':
                        case '+':
                            outchar = 24;
                            break;
                        case '0':
                            outchar = 168;
                            break;
                        case '1':
                            outchar = 169;
                            break;
                        case '2':
                            outchar = 170;
                            break;
                        case '3':
                            outchar = 171;
                            break;
                        case '4':
                            outchar = 172;
                            break;
                        case '5':
                            outchar = 173;
                            break;
                        case '6':
                            outchar = 174;
                            break;
                        case '7':
                            outchar = 175;
                            break;
                        case '8':
                            outchar = 184;
                            break;
                        case '9':
                            outchar = 185;
                            break;
                        default:
                            outchar = 0;
                    }
                    levelBuf.writeUint8(outchar, i);
                }
                const sizeBuf = Buffer.alloc(1);
                const compressed = zlib.deflateRawSync(levelBuf);
                sizeBuf.writeUint8(compressed.length, 0);
                console.log(levelBuf.length + " -> " + compressed.length);
                totalSize += sizeBuf.length + compressed.length;
                return [sizeBuf, compressed];
            }
        }
        return [];
    });
    fs.writeFileSync(outFileName,
    Buffer.concat(convertedLevels));
    console.log(totalSize);
    console.log(`wrote ${convertedLevels.length} levels to ${outFileName}`);
});