const fs = require('fs');
const argv = require('minimist')(process.argv.slice(2));
const path = require('path');

const inFileName = argv._[0];
const outFileName = argv._[1];

const spriteInfo = JSON.parse(fs.readFileSync(inFileName));

const meta = spriteInfo.meta;

const prefix = path.parse(inFileName).name.toUpperCase();

let code = `//@generated
//Editing this manually is not recommended, run "make import" instead!
`;

function pdefine(name, val) {
    code += `#define ${prefix}_${name} ${val}`;
    code += '\n';
}

pdefine('IMG_NAME', `"${meta.image}"`);
pdefine('SHEET_WIDTH', meta.size.w);
pdefine('SHEET_HEIGHT', meta.size.h);
pdefine('SHEET_FRAMES', spriteInfo.frames.length);

meta.frameTags.forEach((tag) => {
    const anim_name = tag.name.split(' ').join('_').toUpperCase();
    pdefine(`TAG_${anim_name}_START`, tag.from);
    pdefine(`TAG_${anim_name}_END`, tag.to+1);
    pdefine(`TAG_${anim_name}_LENGTH`, tag.to - tag.from + 1);
});

fs.writeFileSync(outFileName, code);