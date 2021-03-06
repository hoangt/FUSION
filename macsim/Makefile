SIM_HOME = .
GEMS_HOME = ../gems-lib
BINDIR=$(SIM_HOME)/bin
OBJDIR=$(SIM_HOME)/obj
SRCDIR=$(SIM_HOME)/src
VPATH=$(SRCDIR): $(SRCDIR)/energy_introspector
INCDIR=-I$(SRCDIR) -I$(GEMS_HOME)/ruby_clean/O3sim_ruby 

CC = g++-4.6
#CWARN = -W -Wall -Wshadow -Wimplicit -Wreturn-type -Wcomment -Wtrigraphs -Wformat -Wparentheses -Wpointer-arith -Wuninitialized -O
CFLAGS = $(INCDIR) -std=c++0x
CWARD = -O3
CDBG = -g $(CWARN) -fno-inline -DCHECK_REGISTER_FILE
DFLAGS = -g $(CWARN) -fno-inline -DDEBUG=0

# Uncomment for full Debug mode
CFLAGS += $(CWARD)
#CFLAGS += $(DFLAGS) 


CTAG = etags
CTAGFILE = filelist
# src, object and bin files
TGT=ooo-sim
#Debug Target
DBGTGT=ooo-sim-dbg

POWEROBJS = \
  $(OBJDIR)/parameters.o \
  $(OBJDIR)/parser.o \
  $(OBJDIR)/string_ops.o \
  $(OBJDIR)/ENERGYLIB_McPAT.o \
  $(OBJDIR)/ENERGYLIB_IntSim.o \
  $(OBJDIR)/THERMALLIB_HotSpot.o \
  $(OBJDIR)/RELIABILITYLIB_RAMP.o \
  $(OBJDIR)/SENSORLIB_RNG.o

COMMONOBJS = \
  $(OBJDIR)/all_knobs.o \
  $(OBJDIR)/all_stats.o \
  $(OBJDIR)/allocate.o \
  $(OBJDIR)/allocate_smc.o \
  $(OBJDIR)/bp.o \
  $(OBJDIR)/bp_gshare.o \
  $(OBJDIR)/bp_targ.o \
  $(OBJDIR)/bug_detector.o \
  $(OBJDIR)/cache.o \
  $(OBJDIR)/core.o \
  $(OBJDIR)/exec.o \
  $(OBJDIR)/factory_class.o \
  $(OBJDIR)/fetch_factory.o \
  $(OBJDIR)/frontend.o \
  $(OBJDIR)/knob.o \
  $(OBJDIR)/macsim.o \
  $(OBJDIR)/main.o \
  $(OBJDIR)/map.o \
  $(OBJDIR)/memory.o \
  $(OBJDIR)/memreq_info.o \
  $(OBJDIR)/port.o \
  $(OBJDIR)/pref.o \
  $(OBJDIR)/pref_common.o \
  $(OBJDIR)/pref_factory.o \
  $(OBJDIR)/pref_stride.o \
  $(OBJDIR)/process_manager.o \
  $(OBJDIR)/retire.o \
  $(OBJDIR)/rob.o \
  $(OBJDIR)/rob_smc.o \
  $(OBJDIR)/router.o \
  $(OBJDIR)/schedule.o \
  $(OBJDIR)/schedule_io.o \
  $(OBJDIR)/schedule_ooo.o \
  $(OBJDIR)/schedule_smc.o \
  $(OBJDIR)/statistics.o \
  $(OBJDIR)/trace_read.o \
  $(OBJDIR)/uop.o \
  $(OBJDIR)/utils.o \


OBJS = $(COMMONOBJS) 


DBGOBJS = $(COMMONOBJS) 

RUBY_PROTOCOLS = Fusion 

%.rubyprotocol_build:
	$(MAKE) -C $(GEMS_HOME)/ruby_clean PROTOCOL=$* PROTOCOL_ROOT=../protocols/FUSION NOTESTER=1 -j 8  DEBUG=1
	if [ ! -d $(OBJDIR)/ruby/$* ]; then mkdir -p $(OBJDIR)/ruby/$*; fi;
	cp $(GEMS_HOME)/ruby_clean/amd64-linux/generated/$*/lib/*.so $(OBJDIR)/ruby/$*/
	if [ -L "$(BINDIR)/libruby.so" ]; then rm -f $(BINDIR)/libruby.so; fi; 
	ln -s ../$(OBJDIR)/ruby/$*/libruby.so $(BINDIR)/libruby.so

.knobs:
	$(MAKE) -C knobs/


#-- Rules
all: $(TGT)
dbg: $(DBGTGT)

$(TGT): $(BINDIR)/$(TGT) 
	@echo "$@ uptodate"

$(DBGTGT): $(BINDIR)/$(DBGTGT)
	@echo "$@ uptodate"

$(BINDIR)/$(DBGTGT): $(DBGOBJS)
	$(CC) $(DFLAGS) -o $@ $(DBGOBJS)

$(BINDIR)/$(TGT): .knobs $(OBJS) $(addsuffix .rubyprotocol_build,$(RUBY_PROTOCOLS))
	$(CC) $(CFLAGS) -L$(GEMS_HOME)/ruby_clean/amd64-linux/generated/$(word 1,$(RUBY_PROTOCOLS))/lib/ -o $@ $(OBJS) -lz -lruby

# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
# $< detects only dependencies.
# Otherwise it will try to include the header in the
# compilation leading to a linker error.

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CC) $(CFLAGS) -c -o $@ $<
	$(CC) -MM $(CFLAGS) $? > $(OBJDIR)/$*.d
	@mv -f $(OBJDIR)/$*.d $(OBJDIR)/$*.d.tmp
	@sed -e 's|.*:|obj/$*.o:|' < obj/$*.d.tmp > obj/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < obj/$*.d.tmp | fmt -1 | \
	 sed -e 's/^ *//' -e 's/$$/:/' >> obj/$*.d
	@rm -f $(OBJDIR)/$*.d.tmp



.PHONY : clean depend fresh

tags :
	find . -name "*.cc" -print -or -name "*.h" -print | xargs ctags -e -a

clean :
	-rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d $(PARSE_C) $(PARSE_H)
	-rm -f $(SRCDIR)/*.output $(LEX_C)
	-rm -f */*~ *~ core
	-rm -f $(BINDIR)/$(TGT) $(BINDIR)/$(DBGTGT)
	$(MAKE) -C $(GEMS_HOME)/ruby_clean/ clean
	@echo "Cleaning Knobs"
	$(MAKE) -C knobs/ clean
fresh : clean all
