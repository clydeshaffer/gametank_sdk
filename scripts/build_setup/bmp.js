const BMP = require('bitmap-manipulation');

function getBmpFileDimensions(filename) {
    const bmpFile = BMP.BMPBitmap.fromFile(filename);
    return {
        width : bmpFile.width,
        height : bmpFile.height
    };
}

module.exports = {
    getBmpFileDimensions
}