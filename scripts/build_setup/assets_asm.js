const fs = require('fs');
const BMPUtil = require('./bmp');
const srcGenDir = './src/gen/assets';
const assetsDir = 'assets';

const extensionMap = {
    'bmp' : 'gtg.deflate',
    'bmpx': 'gtgx',
    'mid' : 'gtm2',
    'json' : 'gsi'
};

const ignoreList = [
    "asset.cfg"
];

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
    return '_ASSET__' + dirName + '__' + fileName.replace(/\.|\-/g, '_');
}

function sliceLargeBitmaps(dir) {
    return (filename) => {
        const ext = filename.split('.').slice(-1)[0];
        const nameOnly = filename.split('.').slice(0, -1).join('.');
        if(ext === 'bmp') {
            let fileCount = 1;
            const fullPath = dir + '/' + filename;
            const dims = BMPUtil.getBmpFileDimensions(fullPath);
            if(dims.width > 128) fileCount *= 2;
            if(dims.height > 128) fileCount *= 2;
            if(fileCount > 1) {
                return [...Array(fileCount).keys()].map((num) => (num === 0) ? filename : nameOnly + '_' + num + '.bmp');
            }
        }
        return [filename];
    }
}

function generateAssetsAssemblyFile(dir) {
    const nameList = fs.readdirSync(dir)
                       .flatMap(sliceLargeBitmaps(dir))
                       .filter((fname) => !ignoreList.includes(fname));
    console.log(nameList);
    const path = dir.split('/');
    const dirName = path[path.length - 1];

    const segmentLine = `    .segment "${dirName}"`;
    const exportLines = [];
    const incbinLines = [];

    nameList.forEach((fname) => {
        const symName = filenameToSymbolName(dirName, fname);
        exportLines.push(`    .export ${symName}_ptr`);
        incbinLines.push(`${symName}_ptr:`)
        incbinLines.push(`    .incbin "build/assets/${dirName}/${transformFilename(fname)}"`);
        incbinLines.push('');
    });

    return [
        '; @generated',
        '; Editing this manually is not recommended, run "make import" instead!',
        ...exportLines, '', segmentLine, '', ...incbinLines].join('\n');
}

function generateAssetsHeaderFile(dir, bankNumber) {

    const nameList = fs.readdirSync(dir).flatMap(sliceLargeBitmaps(dir));
    const path = dir.split('/');
    const dirName = path[path.length - 1];

    const gate_define = `ASSETS__${dirName}_H`;
    const externLines = [];

    nameList.forEach((fname) => {
        const symName = filenameToSymbolName(dirName, fname).substring(1);
        externLines.push(`extern const unsigned char* ${symName}_ptr;`);
        externLines.push(`#define ${symName}_bank ${bankNumber}`);
        externLines.push(`#define ${symName} ${symName}_ptr,${symName}_bank`);
    });

    return [
        `//@generated`,
        '//Editing this manually is not recommended, run "make import" instead!',
        `#ifndef ${gate_define}`,
        `#define ${gate_define}`,
        '',
        `#define BANK_${dirName} ${bankNumber}`,
        '',
        ...externLines,
        `#endif`
    ].join('\n');
}

function generateAssetAssemblyFiles(assetFolderNames, folderBankMap) {

    if (fs.existsSync(srcGenDir)){
        fs.rmSync(srcGenDir, { recursive: true });
    }
    fs.mkdirSync(srcGenDir, { recursive: true });

    assetFolderNames.forEach((folder) => {
        if(fs.lstatSync(`./${assetsDir}/${folder}`).isDirectory()) {
            const assetsAsm = generateAssetsAssemblyFile(`./${assetsDir}/${folder}`);
            const assetsFileName = `${folder}.s.asset`;

            const assetsHeader = generateAssetsHeaderFile(`./${assetsDir}/${folder}`, folderBankMap[folder]);
            const assetsHeaderName = `${folder}.h`
            fs.writeFileSync(srcGenDir + '/' + assetsFileName, assetsAsm);
            fs.writeFileSync(srcGenDir + '/' + assetsHeaderName, assetsHeader);
        }
    });
}

module.exports = {
    generateAssetAssemblyFiles
};
