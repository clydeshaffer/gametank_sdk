const BMP = require('bitmap-manipulation');

let imageFromGIMP = BMP.BMPBitmap.fromFile("track_gimp.bmp");
imageFromGIMP.save("gimp_out.bmp");

let imageFromAseprite = BMP.BMPBitmap.fromFile("track_aseprite.bmp");
imageFromAseprite.save("aseprite_out.bmp");