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

function ignoreFilter(x) { return ignoreList.indexOf(x.name)===-1; }
function bmpFilter(x) { return x.ext === "bmp"; }

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
                return [...Array(fileCount).keys()].map((num) => (num === 0) ? filename : nameOnly + '_' + num + '.bmp').map((s, i) => ({
                    name: s,
                    size: [Math.ceil(dims.width / 128), Math.ceil(dims.height / 128)],
                    sequence: i,
                    ext
                }));
            }
        }
        return [{
            name: filename,
            ext
        }];
    }
}

function generateAssetsAssemblyFile(dir) {
    const nameList = fs.readdirSync(dir).flatMap(sliceLargeBitmaps(dir));
    console.log(nameList);
    const path = dir.split('/');
    const dirName = path[path.length - 1];

    const segmentLine = `    .segment "${dirName}"`;
    const exportLines = [];
    const incbinLines = [];

    nameList.filter(ignoreFilter).forEach((x) => {
        const symName = filenameToSymbolName(dirName, x.name);
        exportLines.push(`    .export ${symName}_ptr`);
        incbinLines.push(`${symName}_ptr:`)
        incbinLines.push(`    .incbin "build/assets/${dirName}/${transformFilename(x.name)}"`);
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

    nameList.filter(ignoreFilter).forEach((assetObj) => {
        const symName = filenameToSymbolName(dirName, assetObj.name).substring(1);
        externLines.push(`extern const unsigned char* ${symName}_ptr;`);
        externLines.push(`#define ${symName}_bank ${bankNumber}`);
        externLines.push(`#define ${symName} ${symName}_ptr,${symName}_bank`);
        if(assetObj.ext === "bmp") {
            externLines.push(`extern const SpritePage ${symName}_load_list;`);
        }
    });

    return [
        `//@generated`,
        '//Editing this manually is not recommended, run "make import" instead!',
        `#ifndef ${gate_define}`,
        `#define ${gate_define}`,
        '',
        '#include "sprites.h"',
        '',
        `#define BANK_${dirName} ${bankNumber}`,
        '',
        ...externLines,
        `#endif`
    ].join('\n');
}

function generateAssetsCFile(dir, bankNumber, folder) {
    const bmpGroups = fs.readdirSync(dir).map(sliceLargeBitmaps(dir));
    const path = dir.split('/');
    const dirName = path[path.length - 1];
    const externLines = [];

    bmpGroups.forEach((group) => {
        const names = group.map((assetObj) => assetObj.name);
        group.filter(ignoreFilter).filter(bmpFilter).forEach((assetObj) => {
            const symName = filenameToSymbolName(dirName, assetObj.name).substring(1);
            let nextPtr = '0';
            if(group.length > 1) {
                if(assetObj.sequence != (group.length-1)) {
                    nextPtr = `&${filenameToSymbolName(dirName, names[assetObj.sequence+1]).substring(1)}_load_list`;
                }
            }
            externLines.push(`const SpritePage ${symName}_load_list = {
                &${symName}_ptr, ${bankNumber}, ${nextPtr}
            };`);
        });
    });
    

    return [
        `//@generated`,
        '//Editing this manually is not recommended, run "make import" instead!',
        '',
        '#include "sprites.h"',
        `#include "${folder}.h"`,
        '#pragma rodata-name (push, "PROG0_RO")',
        '',
        ...externLines,
        '#pragma rodata-name (pop)'
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
            const assetsCFile = generateAssetsCFile(`./${assetsDir}/${folder}`, folderBankMap[folder],
            folder);
            const assetsHeaderName = `${folder}.h`
            const assetsCName = `_${folder}__loaders.c`
            fs.writeFileSync(srcGenDir + '/' + assetsFileName, assetsAsm);
            fs.writeFileSync(srcGenDir + '/' + assetsHeaderName, assetsHeader);
            fs.writeFileSync(srcGenDir + '/' + assetsCName, assetsCFile);
        }
    });
}

module.exports = {
    generateAssetAssemblyFiles
};
