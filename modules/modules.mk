MOD_C_SRCS := $(shell $(FIND) modules/*/src -name "*.c")
COBJS += $(patsubst modules/%,$(ODIR)/modules/%,$(MOD_C_SRCS:c=o))

MOD_A_SRCS := $(shell $(FIND) modules/*/src -name "*.s")
AOBJS += $(filter-out $(ASSETLISTS),$(patsubst modules/%,$(ODIR)/modules/%,$(MOD_A_SRCS:s=o)))

MOD_BINSRC = $(shell $(FIND) modules/*/assets -name "*.bin")
BINOBJS += $(patsubst %,$(ODIR)/%,$(MOD_BINSRC))

MOD_ACP_SRCS := $(shell $(FIND) modules/*/src -name "*.acp.asm")
ACP_OBJS += $(patsubst modules/%,$(ODIR)/modules/%,$(MOD_ACP_SRCS:asm=bin.deflate))

MOD_ASSETLISTS := $(shell $(FIND) modules/*/src/gen/assets -name "*.s.asset")
ASSETOBJS += $(filter-out $(MOD_ASSETLISTS),$(patsubst %,$(ODIR)/%,$(MOD_ASSETLISTS:s.asset=o.asset)))