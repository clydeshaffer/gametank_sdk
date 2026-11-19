"use strict";

const fs = require("fs");
const structFu = require("struct-fu");
const Bitmap = require("./Bitmap");
const Canvas = require("./canvas/Canvas");
const Endianness = require("./Endianness");
const ColorDepthChanger = require("./ColorDepthChanger");

/**
 * Data structure for bitmap files (extension <code>.bmp</code>), taken from the Windows API
 * (BITMAPFILEHEADER and BITMAPINFOHEADER).
 *
 * @private
 */
const BitmapFileHeader = structFu.struct([
    structFu.uint16le("signature"),
    structFu.uint32le("fileSize"),
    structFu.uint32le("reserved"),
    structFu.uint32le("dataOffset"),
    structFu.uint32le("bitmapInfoHeaderSize"),
    structFu.int32le("width"),
    structFu.int32le("height"),
    structFu.uint16le("planes"),
    structFu.uint16le("bitsPerPixel"),
    structFu.uint32le("compression"),
    structFu.uint32le("numberOfDataBytes"),
    structFu.int32le("pixelsPerMeterX"),
    structFu.int32le("pixelsPerMeterY"),
    structFu.uint32le("numberOfUsedColors"),
    structFu.uint32le("numberOfImportantColors")
]);
const BITMAP_FILE_SIGNATURE = new Buffer("BM").readUInt16LE(0);
const SIZE_OF_FIRST_PART_OF_BITMAP_FILE_HEADER = (16 + 32 + 32 + 32) / 8; // before "bitmapInfoHeaderSize"
const RGB_BITMAP_COMPRESSION = 0;
const BIT_FIELDS_BITMAP_COMPRESSION = 3;

const DEFAULT_PALETTE = [0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080,
    0xc0c0c0, 0xc0dcc0, 0xa6caf0, 0x402000, 0x602000, 0x802000, 0xa02000, 0xc02000, 0xe02000,
    0x004000, 0x204000, 0x404000, 0x604000, 0x804000, 0xa04000, 0xc04000, 0xe04000, 0x006000,
    0x206000, 0x406000, 0x606000, 0x806000, 0xa06000, 0xc06000, 0xe06000, 0x008000, 0x208000,
    0x408000, 0x608000, 0x808000, 0xa08000, 0xc08000, 0xe08000, 0x00a000, 0x20a000, 0x40a000,
    0x60a000, 0x80a000, 0xa0a000, 0xc0a000, 0xe0a000, 0x00c000, 0x20c000, 0x40c000, 0x60c000,
    0x80c000, 0xa0c000, 0xc0c000, 0xe0c000, 0x00e000, 0x20e000, 0x40e000, 0x60e000, 0x80e000,
    0xa0e000, 0xc0e000, 0xe0e000, 0x000040, 0x200040, 0x400040, 0x600040, 0x800040, 0xa00040,
    0xc00040, 0xe00040, 0x002040, 0x202040, 0x402040, 0x602040, 0x802040, 0xa02040, 0xc02040,
    0xe02040, 0x004040, 0x204040, 0x404040, 0x604040, 0x804040, 0xa04040, 0xc04040, 0xe04040,
    0x006040, 0x206040, 0x406040, 0x606040, 0x806040, 0xa06040, 0xc06040, 0xe06040, 0x008040,
    0x208040, 0x408040, 0x608040, 0x808040, 0xa08040, 0xc08040, 0xe08040, 0x00a040, 0x20a040,
    0x40a040, 0x60a040, 0x80a040, 0xa0a040, 0xc0a040, 0xe0a040, 0x00c040, 0x20c040, 0x40c040,
    0x60c040, 0x80c040, 0xa0c040, 0xc0c040, 0xe0c040, 0x00e040, 0x20e040, 0x40e040, 0x60e040,
    0x80e040, 0xa0e040, 0xc0e040, 0xe0e040, 0x000080, 0x200080, 0x400080, 0x600080, 0x800080,
    0xa00080, 0xc00080, 0xe00080, 0x002080, 0x202080, 0x402080, 0x602080, 0x802080, 0xa02080,
    0xc02080, 0xe02080, 0x004080, 0x204080, 0x404080, 0x604080, 0x804080, 0xa04080, 0xc04080,
    0xe04080, 0x006080, 0x206080, 0x406080, 0x606080, 0x806080, 0xa06080, 0xc06080, 0xe06080,
    0x008080, 0x208080, 0x408080, 0x608080, 0x808080, 0xa08080, 0xc08080, 0xe08080, 0x00a080,
    0x20a080, 0x40a080, 0x60a080, 0x80a080, 0xa0a080, 0xc0a080, 0xe0a080, 0x00c080, 0x20c080,
    0x40c080, 0x60c080, 0x80c080, 0xa0c080, 0xc0c080, 0xe0c080, 0x00e080, 0x20e080, 0x40e080,
    0x60e080, 0x80e080, 0xa0e080, 0xc0e080, 0xe0e080, 0x0000c0, 0x2000c0, 0x4000c0, 0x6000c0,
    0x8000c0, 0xa000c0, 0xc000c0, 0xe000c0, 0x0020c0, 0x2020c0, 0x4020c0, 0x6020c0, 0x8020c0,
    0xa020c0, 0xc020c0, 0xe020c0, 0x0040c0, 0x2040c0, 0x4040c0, 0x6040c0, 0x8040c0, 0xa040c0,
    0xc040c0, 0xe040c0, 0x0060c0, 0x2060c0, 0x4060c0, 0x6060c0, 0x8060c0, 0xa060c0, 0xc060c0,
    0xe060c0, 0x0080c0, 0x2080c0, 0x4080c0, 0x6080c0, 0x8080c0, 0xa080c0, 0xc080c0, 0xe080c0,
    0x00a0c0, 0x20a0c0, 0x40a0c0, 0x60a0c0, 0x80a0c0, 0xa0a0c0, 0xc0a0c0, 0xe0a0c0, 0x00c0c0,
    0x20c0c0, 0x40c0c0, 0x60c0c0, 0x80c0c0, 0xa0c0c0, 0xfffbf0, 0xa0a0a4, 0x808080, 0xff0000,
    0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff];

/**
 * Creates an in-memory bitmap with extensions for reading and writing BMP files.
 * Bitmaps with 1 byte per pixel are handled in conjunction with a palette.
 * BMPBitmap cannot be constructed with a custom canvas, it always uses a grayscale canvas
 * internally.
 *
 * @class
 * @param {number} width
 * @param {number} height
 * @param {number} [bytesPerPixel=1] Possible values: <code>1</code>, <code>2</code>,
 *                                    <code>4</code>
 * @param {Endianness} [endianness=BIG] Use big- or little-endian when storing multiple bytes per
 *                                      pixel
 */
module.exports = class BMPBitmap extends Bitmap {

    constructor(width, height, bytesPerPixel, endianness) {
        if (width instanceof Canvas) {
            throw new Error("Cannot use custom canvas with BMPBitmap");
        }
        super(width, height, bytesPerPixel, endianness);

        this._bytesPerPixel = bytesPerPixel || 1;
        this._endianess = endianness || Endianness.BIG;
        this._palette = this._bytesPerPixel === 1 ? [].concat(DEFAULT_PALETTE) : [];
    }

    /**
     * @returns {number[]} An array of RGB colors (<code>0xRRGGBB</code>) to indices. You can use
     *                      <code>indexOf()</code> to get a color for the other methods
     */
    get palette() {
        return this._palette;
    }

    /**
     * @method module:bitmap_manipulation.BMPBitmap#getPalette
     * @returns {number[]} An array of RGB colors (<code>0xRRGGBB</code>) to indices. You can use
     *                      <code>indexOf()</code> to get a color for the other methods
     * @deprecated
     */
    getPalette() {
        return this._palette;
    }

    /**
     * Reads a bitmap file (extension <code>.bmp</code>). Only those with 1 byte per pixel are
     * supported.
     *
     * @method module:bitmap_manipulation.BMPBitmap.fromFile
     * @param {string} filePath
     * @returns {Bitmap}
     */
    static fromFile(filePath) {
        let file = fs.openSync(filePath, "r");
        // Read header and validate file by means of it
        let fileBuffer = new Buffer(BitmapFileHeader.size);
        let numberOfBytesRead = fs.readSync(file, fileBuffer, 0, fileBuffer.length, null);
        let hasError = numberOfBytesRead !== BitmapFileHeader.size;
        let errorMessage = null;
        let header;
        let offsetAfterHeader;
        if (!hasError) {
            header = BitmapFileHeader.unpack(fileBuffer);
            offsetAfterHeader = SIZE_OF_FIRST_PART_OF_BITMAP_FILE_HEADER +
                header.bitmapInfoHeaderSize;
            let fileSize = fs.fstatSync(file).size;
            hasError =
                header.signature !== BITMAP_FILE_SIGNATURE ||
                header.fileSize !== fileSize ||
                header.dataOffset < offsetAfterHeader || header.dataOffset >= fileSize ||
                offsetAfterHeader < BitmapFileHeader.size ||
                header.width < 1 || header.height < 1 ||
                header.planes !== 1 ||
                [1, 4, 8, 16, 24, 32].indexOf(header.bitsPerPixel) === -1 ||
                header.compression !== RGB_BITMAP_COMPRESSION ||
                header.numberOfDataBytes > fileSize - offsetAfterHeader
            ;
            // Go a little further if header is longer than the structure
            if (BitmapFileHeader.size < offsetAfterHeader) {
                hasError |= fs.readSync(file, fileBuffer, 0, 1, offsetAfterHeader - 1) !== 1;
            }
        }
        if (!hasError && header.bitsPerPixel !== 8) {
            hasError = true;
            errorMessage = `Unsupported number of bits per pixel in file "${filePath}"`;
        }
        // Read palette
        let bitmap = null;
        if (!hasError) {
            bitmap = new BMPBitmap(header.width, header.height);
            let palette = bitmap.palette;
            let filePosition = offsetAfterHeader;
            fileBuffer = new Buffer(4);
            while (filePosition < header.dataOffset) {
                if (fs.readSync(file, fileBuffer, 0, fileBuffer.length, null) !== fileBuffer.length) {
                    hasError = true;
                    errorMessage = `Unexpected end of file in "${filePath}"`;
                    break;
                }
                palette.push(fileBuffer.readUInt32LE(0) & 0xffffff);
                filePosition += fileBuffer.length;
            }
            if (filePosition !== header.dataOffset) {
                // Palette bytes aren't a multiple of 4
                hasError = true;
            }
        }
        // Read pixels
        if (!hasError) {
            let bitmapData = bitmap.data();
            let numberOfBytesPerLine = Math.ceil(header.width * header.bitsPerPixel / 8/*bits*/ /
                    4/*bytes*/) * 4/*bytes*/;
            fileBuffer = new Buffer(numberOfBytesPerLine);
            for (let y = header.height - 1; y >= 0; y--) {
                if (fs.readSync(file, fileBuffer, 0, numberOfBytesPerLine, null) !== numberOfBytesPerLine) {
                    hasError = true;
                    errorMessage = `Unexpected end of file in "${filePath}"`;
                    break;
                }
                fileBuffer.copy(bitmapData, y * header.width, 0, header.width);
            }
        }
        // Finish
        fs.closeSync(file);
        if (hasError) {
            throw new Error(errorMessage || `Could not recognize the file "${filePath}"`);
        }

        return bitmap;
    }

    /**
     * Saves the bitmap to a file in the <code>.bmp</code> format.
     *
     * @method module:bitmap_manipulation.BMPBitmap#save
     * @param {string} filePath
     */
    save(filePath) {
        let header = {
            signature: BITMAP_FILE_SIGNATURE,
            fileSize: null /*later*/,
            reserved: 0,
            dataOffset: BitmapFileHeader.size,
            bitmapInfoHeaderSize: BitmapFileHeader.size -
            SIZE_OF_FIRST_PART_OF_BITMAP_FILE_HEADER,
            width: this.width,
            height: this.height,
            planes: 1,
            bitsPerPixel: this._bytesPerPixel * 8,
            compression: RGB_BITMAP_COMPRESSION,
            numberOfDataBytes: null /*later*/,
            pixelsPerMeterX: Math.round(96/*DPI*/ / 2.54/*cm/inch*/ * 100/*cm/m*/),
            numberOfUsedColors: 0,
            numberOfImportantColors: 0
        };
        header.pixelsPerMeterY = header.pixelsPerMeterX;

        let palette;
        switch (this._bytesPerPixel) {
            case 1: {
                // Make sure there are exactly 256 palette entries
                palette = this._palette.slice(0, 0xff + 1);
                for (let i = palette.length; i <= 0xff; i++) {
                    palette.push(0x000000/*black*/);
                }
                break;
            }
            case 2: {
                header.compression = BIT_FIELDS_BITMAP_COMPRESSION;
                // Masks indicating the used bits (5 bits red, 6 bits green, 5 bits blue)
                palette = [0b11111 << 6 << 5, 0b111111 << 5, 0b11111];
                break;
            }
            case 4: {
                palette = [];
                break;
            }
        }
        header.dataOffset += palette.length * 4/*bytes*/;

        let numberOfBytesPerLine = Math.ceil(this.width * this._bytesPerPixel / 4/*bytes*/) *
            4/*bytes*/;
        header.numberOfDataBytes = numberOfBytesPerLine * this.height;
        header.fileSize = header.dataOffset + header.numberOfDataBytes;

        // Write header to file buffer
        let fileBuffer = new Buffer(header.fileSize);
        BitmapFileHeader.pack(header, fileBuffer);

        // Write palette to file buffer
        let offset = BitmapFileHeader.size;
        for (let color of palette) {
            fileBuffer.writeUInt32LE(color, offset);
            offset += 4;
        }

        // Write pixels to file buffer
        for (let y = this.height - 1; y >= 0; y--) {
            // Ensure that padding bytes are zeroes
            fileBuffer.writeUInt32LE(0, offset + numberOfBytesPerLine - 4);
            for (let x = 0; x < this.width; x++) {
                let pixel = this.getPixel(x, y);
                switch (this._bytesPerPixel) {
                    case 1:
                        fileBuffer.writeUInt8(pixel, offset);
                        break;
                    case 2:
                        fileBuffer.writeUInt16LE(pixel, offset);
                        break;
                    case 4:
                        fileBuffer.writeUInt32LE(pixel, offset);
                        break;
                }

                offset += this._bytesPerPixel;
            }
        }

        // Write file
        fs.writeFileSync(filePath, fileBuffer);
    }

    /**
     * Converts the color depth of the pixels. Pixels are viewed as RGB values,
     * <code>0xRRGGBB</code> for 4 bytes per pixel and the same with 5 bits, 6 bits and 5 bits for 2
     * bytes per pixel. 1-byte pixels are handled in conjunction with a palette. When there are more
     * than 256 different colors in the source pixels, the rest is set to <code>0x00</code>.
     *
     * @method module:bitmap_manipulation.Bitmap#changeColorDepth
     * @param {number} bytesPerPixel
     * @throws {Error} Invalid parameter
     */
    changeColorDepth(bytesPerPixel) {
        // Parameter validation
        switch (bytesPerPixel) {
            case 1:
            case 2:
            case 4: {
                break;
            }
            default: {
                throw new Error(`Invalid number of bytes per pixel: ${bytesPerPixel}`);
            }
        }

        //noinspection Eslint
        let changer = new ColorDepthChanger(this._bytesPerPixel, bytesPerPixel, this._endianess);
        let result = changer.change(this._canvas, this._palette);

        this._bytesPerPixel = bytesPerPixel;
        this._canvas = result.newCanvas;
        this._palette = result.newPalette;
    }
};
