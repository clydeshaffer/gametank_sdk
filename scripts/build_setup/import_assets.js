const LinkerConfig = require('./linkerConfig');
const AssetAssembly = require('./assets_asm');

const fs = require('fs');

const assetsDir = './assets';
const linkerConfigFileName = './gametank-2M.cfg';
const bankMakeListFileName = './bankMakeList';


const dirInfo = fs.readdirSync(assetsDir);

const buildCfg = LinkerConfig.generateLinkerConfig(dirInfo, 1);

fs.writeFileSync(linkerConfigFileName, buildCfg.linker);
fs.writeFileSync(bankMakeListFileName, buildCfg.bankMakeList);


AssetAssembly.generateAssetAssemblyFiles(dirInfo);