const fs = require('fs');
const parser = require('xml2json');
const zlib = require('zlib');

const argv = require('minimist')(process.argv.slice(2));
const inFileName = argv._[0];
const outFileName = argv._.length == 2 ?
    argv._[1] :
    inFileName.split(".").slice(0, -1).join(".") + ".bin";

fs.readFile(inFileName, function(err, data) {
    const json = parser.toJson(data, {
        trim: false
    });
    const jsonObj = JSON.parse(json);
    const levels = jsonObj.SokobanLevels.LevelCollection.Level;
    let totalSize = 0;
    const convertedLevels = levels.flatMap((level, index) => {
        if(totalSize < 16384) { 
            if(level.Width <= 16 && level.Height <= 16) {
                const widthDiff = 16 - level.Width;
                const heightDiff = 16 - level.Height;
                const leftPad = Math.floor(widthDiff/2);

                const padStringLeft = "".padStart(leftPad, " ");

                const topPad = Math.floor(heightDiff/2);
                const levelBuf = Buffer.alloc(256);
                const paddedLevel = 
                    [ ...Array.from(Array(topPad).keys()).map(()=>"".padStart(16, " ")),
                    ...level.L.map((row) => (padStringLeft + row).padEnd(16)),
                    ...Array.from(Array(heightDiff - topPad).keys()).map(()=>"".padStart(16, " ")) ].join("");
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