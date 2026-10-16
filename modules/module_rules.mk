$(ODIR)/modules/%.acp.bin.deflate: $(ODIR)/modules/%.acp.bin
	$(ZOP) --deflate $<

$(ODIR)/modules/%.acp.bin: $(ODIR)/modules/%.acp.o gametank-acp.cfg
	@mkdir -p $(@D)
	$(LN) -C gametank-acp.cfg $< -o $@

$(ODIR)/modules/%.acp.o: modules/%.acp.asm
	@mkdir -p $(@D)
	$(AS) --cpu W65C02 $< -o $@

$(ODIR)/modules/%.si: modules/%.c project.json
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/modules/%.o: modules/%.s project.json
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(ODIR)/modules/%.o: $(ODIR)/modules/%.si project.json
	@mkdir -p $(@D)
	$(AS) $(AFLAGS) -o $@ $<
