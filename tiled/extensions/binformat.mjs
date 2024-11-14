var customMapFormat = {
    name: "Binary Map Format",
    extension: "bin",

    write: function(map, fileName) {        

        var outBin = new BinaryFile(fileName, BinaryFile.WriteOnly);

        var layer = map.layerAt(0);
        if (layer.isTileLayer) {
            var binBuf = new ArrayBuffer(layer.width * layer.height);
            var binView = new Uint8Array(binBuf);
            var i = 0;
            for (var y = 0; y < layer.height; ++y) {
                for (var x = 0; x < layer.width; ++x) {
                    var cell = layer.cellAt(x, y);
                    binView[i] = ((cell == null) ? 0 : cell.tileId);
                    i++;
                }
            }
        }
        
        outBin.write(binBuf);
        outBin.commit();
    },
}

tiled.registerMapFormat("bin", customMapFormat);