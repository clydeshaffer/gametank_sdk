const LinkerConfig = require('./linkerConfig');
const AssetAssembly = require('./assets_asm');

const fs = require('fs');

const assetsDir = './assets';
const linkerConfigFileName = './build/gametank-2M.cfg';
const bankMakeListFileName = './build/bankMakeList.inc';

const projectConfig = JSON.parse(fs.readFileSync('./project.json'));
const requiredProjectKeys = [
    'title',
    'romname',
    'progbanks'
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

const dirInfo = fs.readdirSync(assetsDir).filter((p) => fs.statSync(assetsDir + '/' + p).isDirectory());

const buildCfg = LinkerConfig.generateLinkerConfig(dirInfo, projectConfig.progbanks);

fs.writeFileSync(linkerConfigFileName, buildCfg.linker);
fs.writeFileSync(bankMakeListFileName, buildCfg.bankMakeList);

AssetAssembly.generateAssetAssemblyFiles(dirInfo, buildCfg.folderBankMap);