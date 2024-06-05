const fs = require('fs');
const path = require('path');

const ASSET_FILE_NAME = 'asset.cfg';

function getAssetDirConfig(dirname) {
    const configPath = dirname + '/' + ASSET_FILE_NAME;
    if(fs.existsSync(configPath)) {
        return JSON.parse(fs.readFileSync(configPath));
    }
    return undefined;
}

function getAssetConfig(filename) {
    const filepath = path.parse(filename);
    const filedir = filepath.dir;
    const dirConfig = getAssetDirConfig(filedir);
    if(dirConfig) {
        return dirConfig[filepath.base];
    }
    return undefined;
}

module.exports.getAssetConfig = getAssetConfig;
module.exports.getAssetDirConfig = getAssetDirConfig;