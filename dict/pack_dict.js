const fs = require('fs');
const readline = require('readline');

async function processDictLines() {

    const dictStream = fs.createReadStream('./valid-words.txt');
    const rl = readline.createInterface({
        input : dictStream,
        crlfDelay: Infinity
    });

    let firstLetterCounts = {};
    let firstLetterGroups = {};
    const alphabet = "abcdefghijklmnopqrstuvwxyz";
    for(const letter of alphabet) {
        firstLetterCounts[letter] = 0;
        firstLetterGroups[letter] = [];
    }

    for await (const line of rl) {
        firstLetterCounts[line[0]]++;
        firstLetterGroups[line[0]].push(line.substring(1));
    }

    console.log(firstLetterCounts);

    const indexBuf = Buffer.alloc(alphabet.length * 2);
    let letterIndex = 0;

    for(const letter of alphabet) {
        const group = firstLetterGroups[letter].sort();
        const sectionBuf = Buffer.alloc(group.length * group[0].length);
        let wordIndex = 0;
        group.forEach((suffix) => {
            sectionBuf.write(suffix.split('').reverse().join(''), wordIndex, 'latin1');
            wordIndex += suffix.length;
        });
        fs.mkdirSync(`../assets/dict_${letter}`, { recursive: true });
        console.log(`writing assets/dict_${letter}/words.bin`);
        fs.writeFileSync(`../assets/dict_${letter}/words.bin`, sectionBuf);

        indexBuf.writeUInt16LE(firstLetterCounts[letter], letterIndex * 2);
        ++letterIndex;
    }

    fs.mkdirSync('../assets/dict__index', { recursive: true });
    fs.writeFileSync('../assets/dict__index/counts.bin', indexBuf);
}

processDictLines();