const fs = require('fs');

const srcGenDir = './src/gen/assets';
const assetsDir = 'assets';

const extensionMap = {
    'bmp' : 'gtg.deflate',
    'mid' : 'gtm2',
    'json' : 'gsi'
}

function transformFilename(filename) {
    const name = filename.split('.').slice(0, -1);
    const ext = filename.split('.').slice(-1)[0];
    if(extensionMap.hasOwnProperty(ext)) {
        return [...name, extensionMap[ext]].join('.');
    } else {
        return filename;
    }
}

function filenameToSymbolName(dirName, fileName) {
    return '_ASSET__' + dirName + '__' + fileName.replace('.', '_');
}

function generateAssetsAssemblyFile(dir) {
    const nameList = fs.readdirSync(dir);
    const path = dir.split('/');
    const dirName = path[path.length - 1];

    const segmentLine = `    .segment "${dirName}"`;
    const exportLines = [];
    const incbinLines = [];

    nameList.forEach((fname) => {
        const symName = filenameToSymbolName(dirName, fname);
        exportLines.push(`    .export ${symName}`);
        incbinLines.push(`${symName}:`)
        incbinLines.push(`    .incbin "build/assets/${dirName}/${transformFilename(fname)}"`);
        incbinLines.push('');
    });

    return [...exportLines, '', segmentLine, '', ...incbinLines].join('\n');
}

function generateAssetsHeaderFile(dir) {

    const nameList = fs.readdirSync(dir);
    const path = dir.split('/');
    const dirName = path[path.length - 1];

    const gate_define = `ASSETS__${dirName}_H`;
    const externLines = [];

    nameList.forEach((fname) => {
        const symName = filenameToSymbolName(dirName, fname).substring(1);
        externLines.push(`extern const unsigned char* ${symName};`);
    });

    return [
        `#ifndef ${gate_define}`,
        `#define ${gate_define}`,
        ...externLines,
        `#endif`
    ].join('\n');
}

function generateAssetAssemblyFiles(assetFolderNames) {

    if (fs.existsSync(srcGenDir)){
        fs.rmSync(srcGenDir, { recursive: true });
    }
    fs.mkdirSync(srcGenDir, { recursive: true });

    assetFolderNames.forEach((folder) => {
        const assetsAsm = generateAssetsAssemblyFile(`./${assetsDir}/${folder}`);
        const assetsFileName = `${folder}.s.asset`;

        const assetsHeader = generateAssetsHeaderFile(`./${assetsDir}/${folder}`);
        const assetsHeaderName = `${folder}.h`
        fs.writeFileSync(srcGenDir + '/' + assetsFileName, assetsAsm);
        fs.writeFileSync(srcGenDir + '/' + assetsHeaderName, assetsHeader);
    });
}

module.exports = {
    generateAssetAssemblyFiles
};