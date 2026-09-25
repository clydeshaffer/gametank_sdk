const LinkerConfig = require('./linkerConfig');
const AssetAssembly = require('./assets_asm');

const fs = require('fs');

const assetsDir = './assets';
const modulesDir = './modules';
const linkerConfigFileName = './build/gametank-2M.cfg';
const bankMakeListFileName = './build/bankMakeList.inc';
const bankNumHeaderFileName = './src/gen/bank_nums.h';
const modulesHeaderFileName = './src/gen/modules_enabled.h';
const modulesAsmFileName = './src/gen/modules_enabled.inc';

const projectConfig = JSON.parse(fs.readFileSync('./project.json'));
const requiredProjectKeys = [
    'title',
    'romname',
    'progbanks',
    'modules',
];

let hadError = false;
requiredProjectKeys.forEach((reqKey) => {
    if(!Object.hasOwn(projectConfig, reqKey)) {
        console.error(`Project config missing ${reqKey}`);
        hadError = true;
    }
});
if(hadError) {
    throw new Error('Project config was missing one or more required properties');
}

const modulesInfo = fs.readdirSync(modulesDir).filter((p) => fs.statSync(modulesDir + '/' + p).isDirectory());
const modulesWithAssets = modulesInfo.filter((p) => (fs.existsSync(modulesDir + '/' + p + '/assets')) && fs.statSync(modulesDir + '/' + p + '/assets').isDirectory());

const moduleAssetBanks = modulesWithAssets.flatMap((modName) => fs.readdirSync(modulesDir + '/' + modName + '/assets').filter((p) => fs.statSync(modulesDir + '/' + modName + '/assets/' + p).isDirectory()).map((bankName) => ({module : modName, bank : bankName})));
const moduleAssetBankNames = moduleAssetBanks.map((mab) => `${mab.module}_${mab.bank}`);


const dirInfo = fs.readdirSync(assetsDir).filter((p) => fs.statSync(assetsDir + '/' + p).isDirectory());


const allAssetBanks = [...dirInfo, ...moduleAssetBankNames];

const buildCfg = LinkerConfig.generateLinkerConfig(allAssetBanks, projectConfig.progbanks);

fs.writeFileSync(linkerConfigFileName, buildCfg.linker);
fs.writeFileSync(bankMakeListFileName, buildCfg.bankMakeList);

AssetAssembly.generateAssetAssemblyFiles(dirInfo, moduleAssetBanks, buildCfg.folderBankMap);

fs.writeFileSync(bankNumHeaderFileName, 
    `//@generated
//Editing this manually is not recommended
//This file contains bank numbers for segments not associated with asset folders
${buildCfg.bankNumbers.map((bank) => `#define BANK_${bank.name} ${bank.num}`).join('\n')}
`
);

fs.writeFileSync(modulesHeaderFileName, 
    `//@generated
//Editing this manually is not recommended
//This file tells the SDK optional features which ones are enabled
${projectConfig.modules.map((mod) => `#define ENABLE_MODULE_${mod}`).join('\n')}
`
);

fs.writeFileSync(modulesAsmFileName, 
    `;@generated
;Editing this manually is not recommended
;This file tells the SDK optional features which ones are enabled
${projectConfig.modules.map((mod) => `ENABLE_MODULE_${mod}:`).join('\n')}
`
);