ifndef OS
	OS=$(shell uname)
endif

CC = cc65
AS = ca65
LN = ld65
ifeq ($(OS), Windows_NT)
	FIND = /bin/find
else
	FIND = find
endif

SDIR = src
ODIR = build
ROMDIR = bin

# ROM name is now set in project.json
TARGET := $(shell node -p "require('./project.json').romname")

EMUPATH=../GameTankEmulator

FLASHTOOL = ../GTFO

PORT = COM3

BMPSRC := $(shell $(FIND) assets -name "*.bmp")
$(info bmpsrc is $(BMPSRC))
MIDSRC := $(shell $(FIND) assets -name "*.mid")
JSONSRC := $(shell $(FIND) assets -name "*.json")
ASSETLISTS := $(shell $(FIND) src/gen/assets -name "*.s.asset")
ASSETOBJS = $(filter-out $(ASSETLISTS),$(patsubst src/%,$(ODIR)/%,$(ASSETLISTS:s.asset=o.asset)))

BMPOBJS = $(patsubst %,$(ODIR)/%,$(BMPSRC:bmp=gtg.deflate))
MIDOBJS = $(patsubst %,$(ODIR)/%,$(MIDSRC:mid=gtm2))
JSONOBJS = $(patsubst %,$(ODIR)/%,$(JSONSRC:json=gsi))

BINSRC = $(shell $(FIND) assets -name "*.bin")
BINOBJS = $(patsubst %,$(ODIR)/%,$(BINSRC))

SFXSRC = $(shell $(FIND) assets -name "*.sfx")
SFXOBJS = $(patsubst %,$(ODIR)/%,$(SFXSRC))

CFLAGS = -t none -Osr --cpu 65c02 --codesize 500 --static-locals -I src/gt -g
AFLAGS = --cpu 65C02 --bin-include-dir lib --bin-include-dir $(ODIR)/assets -g
LFLAGS = -C $(ODIR)/gametank-2M.cfg -m $(ODIR)/out.map -vm --dbgfile $(ODIR)/sourcemap.dbg
LLIBS = lib/gametank.lib

C_SRCS := $(shell $(FIND) src -name "*.c")
COBJS = $(patsubst src/%,$(ODIR)/%,$(C_SRCS:c=o))

A_SRCS := $(shell $(FIND) src -name "*.s")
AOBJS = $(filter-out $(ASSETLISTS),$(patsubst src/%,$(ODIR)/%,$(A_SRCS:s=o)))

_AUDIO_FW = audio_fw.bin.deflate
AUDIO_FW = $(patsubst %,$(ODIR)/assets/%,$(_AUDIO_FW))

-include $(ODIR)/bankMakeList.inc #sets _BANKS
_BANKS ?= bankFF
BANKS = $(patsubst %,$(ROMDIR)/$(TARGET).%,$(_BANKS))

$(ROMDIR)/$(TARGET): $(ODIR)/bankMakeList.inc $(BANKS)
	cat $(BANKS) > $@

$(info ASSETOBJS is $(ASSETOBJS))

$(BANKS): $(ODIR)/bankMakeList.inc $(ASSETOBJS) $(AOBJS) $(COBJS) $(LLIBS) $(ODIR)/gametank-2M.cfg
	@mkdir -p $(@D)
	$(LN) $(LFLAGS) $(ASSETOBJS) $(AOBJS) $(COBJS) -o $(ROMDIR)/$(TARGET) $(LLIBS)

.PRECIOUS: $(ODIR)/assets/%.gtg
$(ODIR)/assets/%.gtg: assets/%.bmp | scripts/converters/node_modules
	@mkdir -p $(@D)
	cd scripts/converters ;\
	zopfli --deflate $(shell cd scripts/converters && node sprite_convert.js ../../$< ../../$@)

.PRECIOUS: $(ODIR)/assets/%.gtm2
$(ODIR)/assets/%.gtm2: assets/%.mid | scripts/converters/node_modules
	@mkdir -p $(@D)
	cd scripts/converters ;\
	node midiconvert.js ../../$< ../../$@

.PRECIOUS: $(ODIR)/assets/%.deflate
$(ODIR)/assets/%.deflate: $(ODIR)/assets/%
	@mkdir -p $(@D)
	zopfli --deflate $<

.PRECIOUS: $(ODIR)/assets/%.gsi
$(ODIR)/assets/%.gsi: assets/%.json | scripts/converters/node_modules
	@mkdir -p $(@D)
	cd scripts/converters ;\
	node sprite_metadata.js ../../$< ../../$@

$(ODIR)/assets/%.bin: assets/%.bin
	@mkdir -p $(@D)
	cp $< $@

$(ODIR)/assets/%.sfx: assets/%.sfx
	@mkdir -p $(@D)
	cp $< $@

$(ODIR)/assets/audio_fw.bin.deflate: $(ODIR)/assets/audio_fw.bin
	zopfli --deflate $<

$(ODIR)/assets/audio_fw.bin: src/gt/audio/audio_fw.asm gametank-acp.cfg
	@mkdir -p $(@D)
	$(AS) --cpu 65C02 src/gt/audio/audio_fw.asm -o $(ODIR)/assets/audio_fw.o
	$(LN) -C gametank-acp.cfg $(ODIR)/assets/audio_fw.o -o $(ODIR)/assets/audio_fw.bin

$(ODIR)/gen/assets/%.o.asset: src/gen/assets/%.s.asset $(BINOBJS) $(SFXOBJS)
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

$(ODIR)/%.si: src/%.c src/%.h project.json
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.si: src/%.c project.json
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.o: $(ODIR)/%.si project.json
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

$(ODIR)/%.o: src/%.s project.json
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

$(ODIR)/gt/crt0.o: src/gt/crt0.s $(ODIR)/assets/audio_fw.bin.deflate
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

scripts/%/node_modules:
	cd scripts/$* ;\
	npm install

dummy%:
	@:

.PHONY: clean clean-node flash emulate import

clean:
	rm -rf $(ODIR)/*
	rm -rf $(ROMDIR)/*

clean-node:
	rm -rf scripts/*/node_modules

flash: $(ODIR)/bankMakeList.inc $(BANKS)
	$(FLASHTOOL)/bin/GTFO -p $(PORT) $(ROMDIR)/$(TARGET).bank*

emulate: $(ROMDIR)/$(TARGET)
	$(EMUPATH)/build/GameTankEmulator $(ROMDIR)/$(TARGET)

scripts/build_setup/node_modules: scripts/build_setup/package.json
	cd scripts/build_setup ;\
	npm install

scripts/converters/node_modules: scripts/converters/package.json
	cd scripts/converters ;\
	npm install


$(ODIR)/%.cfg $(ODIR)/%.inc src/gen/assets/%.s.asset: project.json scripts/build_setup/*.js scripts/build_setup/node_modules $(BMPOBJS) $(JSONOBJS) $(AUDIO_FW) $(MIDOBJS) $(BINOBJS) $(SFXOBJS)
	mkdir -p $(ODIR)
	find assets -type f -name '*:Zone.Identifier' -delete
	node ./scripts/build_setup/build_setup.js
	if [ -n "$(wildcard assets/*/*.json)" ]; then \
		for json_file in assets/*/*.json; do \
			inc_name=$$(basename $$json_file); \
			dir_name=$$(dirname $$json_file); \
			mkdir -p src/gen/$$dir_name; \
			node ./scripts/build_setup/frame_import.js $$json_file src/gen/$$dir_name/$$inc_name.h; \
		done \
	fi

import : $(ODIR)/gametank-2M.cfg $(ODIR)/bankMakeList.inc