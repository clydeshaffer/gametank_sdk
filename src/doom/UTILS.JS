function make_c_array(lut) {
    var arrayStr = "{\n";
    for(var i = 0; i <= lut.length; i++)
    {
        arrayStr += "\t";
        for(var k = 0; k < 10 && (typeof lut[i] != "undefined"); k++) {
            arrayStr += lut[i] + ", ";
            i++;
        }
        arrayStr += "\n";
    }
    return arrayStr.slice(0,-3) + "\n};";
}

function rad2bin(x) {
    return Math.round(x * 65536 / (Math.PI * 2));
}

function bin2rad(x) {
    var part = Math.PI;
    var result = 0;
    for(var i = 15; i >= 0; i --) {
        if(x & (1 << i)) {
            result += part;
        }
        part /= 2;
    }
    return result;
}

function generate_atan_lookup() {
    var atan_lookup = [];
    for(var i = 0; i <= 2048; i++)
    {
        atan_lookup[i] = rad2bin(Math.atan(i/2048));
    }
    return atan_lookup;
}

function generate_angle_to_screen(view_width_radians) {
    var view_width_bin = rad2bin(view_width_radians);
    for(var i = 0; i < view_width_bin; i++) {
        
    }
}