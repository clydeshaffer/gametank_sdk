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

#set this for your output ROM file name
TARGET=game.gtr

EMUPATH=../GameTankEmulator

FLASHTOOL = ../GTFO

SDIR = src
ODIR = build

PORT = COM3

BMPSRC := $(shell $(FIND) assets -name "*.bmp")
$(info bmpsrc is $(BMPSRC))
MIDSRC := $(shell $(FIND) assets -name "*.mid")
JSONSRC := $(shell $(FIND) assets -name "*.json")
ASSETLISTS := $(shell $(FIND) src/gen/assets -name "*.s.asset")
ASSETOBJS = $(filter-out $(ASSETLISTS),$(patsubst src/%,$(ODIR)/%,$(ASSETLISTS:s.asset=o)))

BMPOBJS = $(patsubst %,$(ODIR)/%,$(BMPSRC:bmp=gtg.deflate))
MIDOBJS = $(patsubst %,$(ODIR)/%,$(MIDSRC:mid=gtm2))
JSONOBJS = $(patsubst %,$(ODIR)/%,$(JSONSRC:json=gsi))

BINSRC = $(shell $(FIND) assets -name "*.bin")
BINOBJS = $(patsubst %,$(ODIR)/%,$(BINSRC))

CFLAGS = -t none -Osr --cpu 65c02 --codesize 500 --static-locals -I src/gt
AFLAGS = --cpu 65C02 --bin-include-dir lib --bin-include-dir $(ODIR)/assets
LFLAGS = -C gametank-2M.cfg -m $(ODIR)/out.map -vm
LLIBS = lib/gametank.lib

C_SRCS := $(shell $(FIND) src -name "*.c")
COBJS = $(patsubst src/%,$(ODIR)/%,$(C_SRCS:c=o))

A_SRCS := $(shell $(FIND) src -name "*.s")
AOBJS = $(filter-out $(ASSETLISTS),$(patsubst src/%,$(ODIR)/%,$(A_SRCS:s=o)))

_AUDIO_FW = audio_fw.bin.deflate
AUDIO_FW = $(patsubst %,$(ODIR)/assets/%,$(_AUDIO_FW))

-include bankMakeList #sets _BANKS
_BANKS ?= bankFF
BANKS = $(patsubst %,bin/$(TARGET).%,$(_BANKS))

bin/$(TARGET): $(BANKS)
	cat $(BANKS) > $@

$(info ASSETOBJS is $(ASSETOBJS))

$(BANKS): $(ASSETOBJS) $(AOBJS) $(COBJS) $(LLIBS) gametank-2M.cfg
	@mkdir -p $(@D)
	$(LN) $(LFLAGS) $(ASSETOBJS) $(AOBJS) $(COBJS) -o bin/$(TARGET) $(LLIBS)

.PRECIOUS: $(ODIR)/assets/%.gtg
$(ODIR)/assets/%.gtg: assets/%.bmp | node_modules
	@mkdir -p $(@D)
	cd scripts/converters ;\
	zopfli --deflate $(shell cd scripts/converters && node sprite_convert.js ../../$< ../../$@)

.PRECIOUS: $(ODIR)/assets/%.gtm2
$(ODIR)/assets/%.gtm2: assets/%.mid | node_modules
	@mkdir -p $(@D)
	cd scripts/converters ;\
	node midiconvert.js ../../$< ../../$@

.PRECIOUS: $(ODIR)/assets/%.deflate
$(ODIR)/assets/%.deflate: $(ODIR)/assets/%
	@mkdir -p $(@D)
	zopfli --deflate $<

.PRECIOUS: $(ODIR)/assets/%.gsi
$(ODIR)/assets/%.gsi: assets/%.json | node_modules
	@mkdir -p $(@D)
	cd scripts/converters ;\
	node sprite_metadata.js ../../$< ../../$@

$(ODIR)/assets/%.bin: assets/%.bin
	@mkdir -p $(@D)
	cp $< $@

$(ODIR)/assets/audio_fw.bin.deflate: $(ODIR)/assets/audio_fw.bin
	zopfli --deflate $<

$(ODIR)/assets/audio_fw.bin: src/gt/audio_fw.asm gametank-acp.cfg
	@mkdir -p $(@D)
	$(AS) --cpu 65C02 src/gt/audio_fw.asm -o $(ODIR)/assets/audio_fw.o
	$(LN) -C gametank-acp.cfg $(ODIR)/assets/audio_fw.o -o $(ODIR)/assets/audio_fw.bin

$(ODIR)/gen/assets/%.o: src/gen/assets/%.s.asset $(BMPOBJS) $(JSONOBJS) $(AUDIO_FW) $(MIDOBJS) $(BINOBJS)
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

$(ODIR)/%.si: src/%.c src/%.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.si: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/%.o: $(ODIR)/%.si
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

$(ODIR)/%.o: src/%.s
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

$(ODIR)/gt/crt0.o: src/gt/crt0.s build/assets/audio_fw.bin.deflate
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<

gametank-2M.cfg: import

src/gen/assets/%: import

scripts/%/node_modules:
	cd scripts/$* ;\
	npm install

dummy%:
	@:

.PHONY: clean clean-node flash emulate import node_modules

clean:
	rm -rf $(ODIR)/*
	rm -rf bin/*

clean-node:
	rm -rf scripts/*/node_modules

flash: $(BANKS)
	$(FLASHTOOL)/bin/GTFO -p $(PORT) bin/$(TARGET).bank*

emulate: bin/$(TARGET)
	$(EMUPATH)/build/GameTankEmulator bin/$(TARGET)

node_modules: scripts/build_setup/node_modules scripts/converters/node_modules

import: node_modules
	node ./scripts/build_setup/import_assets.js
