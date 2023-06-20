const fs = require('fs');
const argv = require('minimist')(process.argv.slice(2));

const inFileName = argv._[0];
const outFileName = argv._.length == 2 ?
    argv._[1] :
    inFileName.split(".").slice(0, -1).join(".") + ".gsi";

const spriteInfo = JSON.parse(fs.readFileSync(inFileName));

//offset X "VXO", offset Y "VYO", width, height, GX, GY
//offsets relative to original frame center

function aseprite2gametank(asframe) {
    return {
        vxo: asframe.spriteSourceSize.x - (asframe.sourceSize.w / 2),
        vyo: asframe.spriteSourceSize.y - (asframe.sourceSize.h / 2),
        w: asframe.frame.w,
        h: asframe.frame.h,
        gx: asframe.frame.x,
        gy: asframe.frame.y
    };
}

function frame2Buf(gtFrameObj) {
    const buf = Buffer.alloc(8);
    buf.writeInt8(gtFrameObj.vxo, 0);
    buf.writeInt8(gtFrameObj.vyo, 1);
    buf.writeUInt8(gtFrameObj.w, 2);
    buf.writeUInt8(gtFrameObj.h, 3);
    buf.writeUInt8(gtFrameObj.gx, 4);
    buf.writeUInt8(gtFrameObj.gy, 5);
    buf.writeUInt8(0, 6);
    buf.writeUInt8(0, 7);
    return buf;
}

console.log(spriteInfo.meta);

console.log(spriteInfo.frames.length + " frames read");

const outBuf = Buffer.concat(
    spriteInfo.frames
        .map(aseprite2gametank)
        .map(frame2Buf)
);

fs.writeFileSync(outFileName, outBuf);