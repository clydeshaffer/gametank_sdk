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
            if((dims.height > 128) && (dims.width <= 128)) {
                //Special case for tall bmps
                return [
                    {
                        name: filename,
                        loadName: filename,
                        size: [Math.ceil(dims.width / 128), Math.ceil(dims.height / 128)],
                        sequence: 0,
                        ext,
                    },
                    {
                        name: nameOnly + '_1.bmp',
                        loadName : null,
                        size: [Math.ceil(dims.width / 128), Math.ceil(dims.height / 128)],
                        sequence: 1,
                        ext,
                    },
                    {
                        name: nameOnly + '_2.bmp',
                        loadName: nameOnly + '_1.bmp',
                        size: [Math.ceil(dims.width / 128), Math.ceil(dims.height / 128)],
                        sequence: 2,
                        ext,
                    },
                ];
            }
            if(fileCount > 1) {
                return [...Array(fileCount).keys()].map((num) => (num === 0) ? filename : nameOnly + '_' + num + '.bmp').map((s, i) => ({
                    name: s,
                    loadName : s,
                    size: [Math.ceil(dims.width / 128), Math.ceil(dims.height / 128)],
                    sequence: i,
                    ext,
                }));
            }
        }
        return [{
            name: filename, 
            loadName: filename,
            ext,
            sequence: 0,
        }];
    }
}

function generateAssetsAssemblyFile(assetBank) {
    const currentAssetsDir = (assetBank.module == null) ? assetsDir : `modules/${assetBank.module}/assets`;
    const dir = `${currentAssetsDir}/${assetBank.bank}`;

    const nameList = fs.readdirSync(dir).flatMap(sliceLargeBitmaps(dir));
    console.log(nameList);
    const path = dir.split('/');
    const dirName = path[path.length - 1];
    const segmentName = (assetBank.module == null) ? dirName : `${assetBank.module}_${assetBank.bank}`;

    const segmentLine = `    .segment "${segmentName}"`;
    const exportLines = [];
    const incbinLines = [];

    nameList.filter(ignoreFilter).filter((x)=>(x.loadName != null)).forEach((x) => {
        const symName = filenameToSymbolName(dirName, x.loadName);
        exportLines.push(`    .export ${symName}_ptr`);
        incbinLines.push(`${symName}_ptr:`)

        if(assetBank.module == null) {
            incbinLines.push(`    .incbin "build/assets/${dirName}/${transformFilename(x.loadName)}"`);
        } else {
            incbinLines.push(`    .incbin "build/modules/${assetBank.module}/assets/${assetBank.bank}/${transformFilename(x.loadName)}"`);
        }


        incbinLines.push('');
    });

    return [
        '; @generated',
        '; Editing this manually is not recommended, run "make import" instead!',
        ...exportLines, '', segmentLine, '', ...incbinLines].join('\n');
}

function generateAssetsHeaderFile(dir, bankNumber, pushFileHandles) {

    const nameList = fs.readdirSync(dir).flatMap(sliceLargeBitmaps(dir));
    const path = dir.split('/');
    const dirName = path[path.length - 1];

    const gate_define = `ASSETS__${dirName}_H`;
    const externLines = [];

    nameList.filter(ignoreFilter).filter((assetInfo) => assetInfo.loadName != null).forEach((assetObj) => {
        const symName = filenameToSymbolName(dirName, assetObj.loadName).substring(1);
        externLines.push(`//${dir}/${assetObj.name}`);

        const ptrName = `${symName}_ptr`;
        const bankName = `${symName}_bank`;
        const idName = `${symName}_ID`;

        const assetId = pushFileHandles(assetObj.ext, ptrName, bankName);

        externLines.push(`extern const unsigned char ${ptrName}[];`);
        externLines.push(`#define ${bankName} ${bankNumber}`);
        externLines.push(`#define ${symName} ${ptrName},${bankName}`);
        externLines.push(`#define ${idName} ${assetId}`);
        if((assetObj.ext === "bmp") && (assetObj.sequence == 0)) {
            externLines.push(`extern const SpritePage ${symName}_load_list;`);
        }
        if(fs.existsSync(`${dir}/${assetObj.name}`)) {
            externLines.push(`#define ${symName}_size ${fs.statSync(`${dir}/${assetObj.name}`).size}`);
        }
    });

    return [
        `//@generated`,
        '//Editing this manually is not recommended, run "make import" instead!',
        `#ifndef ${gate_define}`,
        `#define ${gate_define}`,
        '',
        '#include "gfx/sprites.h"',
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
            const data = assetObj.loadName != null ? `${filenameToSymbolName(dirName, assetObj.loadName).substring(1)}_ptr` : '0';
            let nextPtr = '0';
            if(group.length > 1) {
                if(assetObj.sequence != (group.length-1)) {
                    nextPtr = `&${filenameToSymbolName(dirName, names[assetObj.sequence+1]).substring(1)}_load_list`;
                }
            }
            externLines.unshift(`${assetObj.sequence == 0 ? '' : 'static '}const SpritePage ${symName}_load_list = {
                ${data}, ${bankNumber}, ${nextPtr}
            };`);
        });
    });
    

    return [
        `//@generated`,
        '//Editing this manually is not recommended, run "make import" instead!',
        '',
        '#include "gfx/sprites.h"',
        `#include "${folder}.h"`,
        '#pragma rodata-name (push, "LOADERS")',
        '',
        ...externLines,
        '#pragma rodata-name (pop)'
    ].join('\n');
}

function generateACertainAssetsIndex(assetCollection, includeList) {
    return [
        ...includeList.map((inc) => `#include "./${inc}"`),
        '#pragma rodata-name ("LOADERS")',
        ...Object.keys(assetCollection).map((ext) => `
const void* ASSET__${ext}_ptr_table[] = {
${assetCollection[ext].map((fileEntry) => fileEntry.ptrName).join(',')}
};

const unsigned char ASSET__${ext}_bank_table[] = {
${assetCollection[ext].map((fileEntry) => fileEntry.bankName).join(',')}
};
`)
    ].join('\n');
}

function generateACertainAssetsIndexHeader(assetCollection, includeList) {
    return [
        ...Object.keys(assetCollection).map((ext) => `
extern const void* ASSET__${ext}_ptr_table[];

extern const unsigned char ASSET__${ext}_bank_table[];
`)
    ].join('\n');
}

function generateAssetAssemblyFiles(assetFolderNames, moduleAssetBanks, folderBankMap) {

    if (fs.existsSync(srcGenDir)){
        fs.rmSync(srcGenDir, { recursive: true });
    }
    fs.mkdirSync(srcGenDir, { recursive: true });

    const assetsByExtension = {
        "sfx" : [],
        "bmp" : []
    };
    const allIncludes = [];
    const collectFunc = (ext, ptrName, bankName) => {
        if(!assetsByExtension.hasOwnProperty(ext)) {
            assetsByExtension[ext] = [];
        }
        assetsByExtension[ext].push({
            ptrName,
            bankName
        });
        return assetsByExtension[ext].length - 1;
    }

    const allAssetBanks = [...assetFolderNames.map((a) => ({module : null, bank : a})), ...moduleAssetBanks];

    moduleAssetBanks.forEach((mab) => {
        if(!fs.existsSync( `./modules/${mab.module}/src/gen/assets`)) {
            fs.mkdirSync( `./modules/${mab.module}/src/gen/assets`, {recursive : true});
        }
    });

    allAssetBanks.forEach((assetBank) => {
        const folder = assetBank.bank;
        const currentAssetsDir = (assetBank.module == null) ? assetsDir : `modules/${assetBank.module}/assets`;
        if(fs.lstatSync(`./${currentAssetsDir}/${folder}`).isDirectory()) {
            const assetsAsm = generateAssetsAssemblyFile(assetBank);
            const assetsFileName = `${folder}.s.asset`;

            const bankNumber = (assetBank.module == null) ? folderBankMap[folder] : folderBankMap[assetBank.module + '_' + folder];
            const assetsHeader = generateAssetsHeaderFile(`./${currentAssetsDir}/${folder}`, bankNumber, collectFunc);
            const assetsCFile = generateAssetsCFile(`./${currentAssetsDir}/${folder}`, folderBankMap[folder],
            folder);
            
            const assetsHeaderName = `${folder}.h`;
            const assetsCName = `${folder}__loaders.c`
            const genDir = (assetBank.module == null) ? srcGenDir : `./modules/${assetBank.module}/src/gen/assets`;
            fs.writeFileSync(genDir + '/' + assetsFileName, assetsAsm);
            fs.writeFileSync(genDir + '/' + assetsHeaderName, assetsHeader);
            fs.writeFileSync(genDir + '/' + assetsCName, assetsCFile);
            if(assetBank.module == null) {
                allIncludes.push(assetsHeaderName);
            } else {
                allIncludes.push(`../../../modules/${assetBank.module}/src/gen/assets/${assetBank.bank}.h`);
            }
        }
    });

    const assetIndexFile = generateACertainAssetsIndex(assetsByExtension, allIncludes);
    const assetIndexHeaderFile = generateACertainAssetsIndexHeader(assetsByExtension, allIncludes);
    const assetsIndexName = 'assets_index.c';
    const assetsIndexHeaderName = 'assets_index.h';
    fs.writeFileSync(srcGenDir + '/' + assetsIndexName, assetIndexFile);
    fs.writeFileSync(srcGenDir + '/' + assetsIndexHeaderName, assetIndexHeaderFile);

}

module.exports = {
    generateAssetAssemblyFiles
};
