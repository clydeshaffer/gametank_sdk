function hex(num) {
    if(num < 16) {
        return '0' + num.toString(16).toUpperCase();
    }
    return num.toString(16).toUpperCase();
}

function generateLinkerConfig(assetFolderNames, extra_code_banks) {

    let folderBankMap  = {};

    function genSection(name, entries) {
        var outText = name + ' {\n';
        for(var entName in entries) {
            outText += `\t${entName}:\t`;
            const attributes = [];
            for(var attName in entries[entName]) {
                attributes.push(`${attName} = ${entries[entName][attName]}`);
            }
            outText += attributes.join(',\ ') + ';\n';
        }
        outText += '}';
        return outText;
    }

    var section_MEMORY = {
        ZP : {
            start : '$0',
            size : '$100',
            type : 'rw',
            define : 'yes'
        },
        RAM : {
            start : '$200',
            size : '$1D00',
            define : 'yes'
        },
    };

    var section_SEGMENTS = {
        ZEROPAGE: {
            load : 'ZP',
            type : 'zp',
            define : 'yes'
        },
        DATA : {
            load : 'ROM',
            type : 'rw',
            define : 'yes',
            run : 'RAM'
        },
        BSS : {
            load : 'RAM',
            type : 'bss',
            define : 'yes'
        },
        HEAP : {
            load : 'RAM',
            type : 'bss',
            optional : 'yes'
        },
        ONCE : {
            load : 'ROM',
            type : 'ro',
            optional : 'yes'
        },
        CODE : {
            load : 'ROM',
            type : 'ro'
        },
        RODATA : {
            load : 'ROM',
            type : 'ro'
        },
        STARTUP : {
            load : 'ROM',
            type : 'ro'
        },
        VECTORS : {
            load : 'ROM',
            type : 'ro',
            start : '$FFFA'
        },
        SAVE : {
            load : 'PERSIST',
            type : 'ro'
        },
    };

    const footer = `
FEATURES {
    CONDES: segment = STARTUP,
            type    = constructor,
            label   = __CONSTRUCTOR_TABLE__,
            count   = __CONSTRUCTOR_COUNT__;
    CONDES: segment = STARTUP,
            type    = destructor,
            label   = __DESTRUCTOR_TABLE__,
            count   = __DESTRUCTOR_COUNT__;
}

SYMBOLS {
    # Define the stack size for the application
    __STACKSIZE__:  value = $0800, type = weak;
}
`;

    const names2 = ['COMMON', ...assetFolderNames];
    const asset_banks = names2.length;
    const bankNames = [];

    for(var i = 0; i < asset_banks; i++) {
        const bankNum = i + 128;
        const bankName = 'BANK' + hex(bankNum);
        bankNames.push(`bank${hex(bankNum)}`);
        const bankFile = `"%O.bank${hex(bankNum)}"`;
        const segmentName = names2[i];
        folderBankMap[segmentName] = bankNum;
        section_MEMORY[bankName] = {
            start : '$8000',
            size : '$4000',
            file : bankFile,
            bank : bankNum.toString(),
            fill : 'yes'
        };
        section_SEGMENTS[segmentName] = {
            load : bankName,
            type : 'ro'
        };
    }

    bankNames.push('filler');

    for(var i = 1; i <= extra_code_banks; i++) {
        const bankNum = 253 - extra_code_banks + i;
        const bankName = 'BANK' + hex(bankNum);
        bankNames.push(`bank${hex(bankNum)}`);
        const bankFile = `"%O.bank${hex(bankNum)}"`;
        const segmentName = 'PROG' + (extra_code_banks - i).toString(16).toUpperCase()
        section_MEMORY[bankName] = {
            start : '$8000',
            size : '$4000',
            file : bankFile,
            bank : bankNum.toString(),
            fill : 'yes'
        };
        section_SEGMENTS[segmentName] = {
            load : bankName,
            type : 'ro'
        };
    }

    section_MEMORY['PERSIST'] = {
        start : '$8000',
        size : '$4000',
        file : '"%O.bankFE"',
        bank : '254',
        fill : 'yes'
    }

    section_MEMORY['ROM'] = {
        start : '$C000',
        size : '$4000',
        file : '"%O.bankFF"',
        bank : '255',
        fill : 'yes'
    };

    section_MEMORY['FILLER'] = {
        start : '$8000',
        size : '$' + (Math.pow(2,21) - (extra_code_banks + asset_banks + 2) * 16384).toString(16).toUpperCase(),
        file : '"%O.filler"',
        fill : 'yes'
    }

    bankNames.push('bankFE');
    bankNames.push('bankFF');

    var output = '';
    function printout(str) {
        output += str + '\n';
    }
    printout('#GENERATED Linker config to create a GameTank 2 Megabyte image');
    printout('#Editing this manually is not recommended, run "make import" instead!');
    printout(genSection('MEMORY', section_MEMORY));
    printout('');
    printout(genSection('SEGMENTS', section_SEGMENTS));
    printout(footer);

    return {
        linker: output,
        bankMakeList : `_BANKS = ${bankNames.join(' ')}`,
        folderBankMap
    }
}

module.exports = {
    generateLinkerConfig
};