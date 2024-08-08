VERSION = DEBUG

SIM_OBJ_DIR = build/simdebug/
DEP_DIR = build/deps/

OBJ_DIR_RELEASE = build/release/
OBJ_DIR_DEBUG = build/debug/
OBJ_DIR_PROFILE = build/profile/
OBJ_DIR = $(OBJ_DIR_$(VERSION))
SRC_DIR = src/

LINK_OPT_RELEASE =
LINK_OPT_DEBUG = DEBUG WATCOM ALL
LINK_OPT_PROFILE = DEBUG WATCOM ALL
LINK_OPT = $(LINK_OPT_$(VERSION))

LINK = wlink

CC_OPT_RELEASE = -dNDEBUG -otexan
CC_OPT_DEBUG = -d3i -od
CC_OPT_PROFILE = -dNDEBUG -d1 -otexan
CC_OPT = -fo=$(OBJ_DIR) $(CC_OPT_$(VERSION)) -2 -ml -bt=dos -fpc -d__STDC_LIMIT_MACROS -wx -ze

CC = wpp

IMBIBE_HEADERS = $(wildcard src/*.h)

IMBIBE_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.obj,$(wildcard $(SRC_DIR)*.cpp))
IMBIBE_SIM_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(SIM_OBJ_DIR)%.o,$(wildcard $(SRC_DIR)*.cpp))
IMBIBE_DEPS = $(patsubst $(SRC_DIR)%.cpp,$(DEP_DIR)%.d,$(wildcard $(SRC_DIR)*.cpp))

.PHONY: all clean deps dirs imbibe
.DEFAULT_GOAL: all

$(SIM_OBJ_DIR):
	mkdir -p $@
$(DEP_DIR):
	mkdir -p $@
$(OBJ_DIR_RELEASE):
	mkdir -p $@
$(OBJ_DIR_DEBUG):
	mkdir -p $@
$(OBJ_DIR_PROFILE):
	mkdir -p $@

dirs: $(SIM_OBJ_DIR) $(DEP_DIR) $(OBJ_DIR)

$(SIM_OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(SIM_OBJ_DIR) $(DEP_DIR)
	g++ -std=gnu++98 -g -W -Wall -Werror -MT $@ -MMD -MP -MF $(DEP_DIR)$*.d -c $< -o $@

$(DEP_DIR)%.d: $(SIM_OBJ_DIR)%.o

include $(wildcard $(DEP_DIR)*.d)

all: imbibe

clean:
	rm -rf build

simbibe: $(IMBIBE_SIM_OBJS)
	g++ -std=gnu++98 -g -o $@ $^

imbibe: $(OBJ_DIR)imbibe.exe
	cd workspace && \
	  dosbox \
	    -c "S:" \
	    -c "`echo $< | tr '/' '\\' ` > RUN.LOG"

$(OBJ_DIR)imbibe.lnk: | $(OBJ_DIR)
	echo NAME $(OBJ_DIR)imbibe.exe > $@
	echo SYSTEM DOS >> $@
	echo OPTION ELIMINATE >> $@
	echo $(LINK_OPT) >> $@
	for i in $(IMBIBE_OBJS); do \
	    echo file $$i >> $@; \
	  done

uc = $(shell echo $(1) | tr a-z A-Z)

$(OBJ_DIR)%.exe: private UPPER_TGT = $(@D)/$(call uc,$(@F))
$(OBJ_DIR)%.exe: private LINK_LOG = $(patsubst %.EXE,%.LOG,$(UPPER_TGT))
$(OBJ_DIR)imbibe.exe: $(OBJ_DIR)imbibe.lnk $(IMBIBE_OBJS)
	rm -f $@
	cd workspace && \
	  dosbox \
	    -c "S:" \
	    -c "$(LINK) @$< > $(LINK_LOG)" \
	    -c "exit"
	mv $(UPPER_TGT) $@

$(OBJ_DIR)%.obj: private UPPER_TGT = $(@D)/$(call uc,$(@F))
$(OBJ_DIR)%.obj: private CC_LOG = $(patsubst %.OBJ,%.LOG,$(UPPER_TGT))
$(OBJ_DIR)%.obj: $(SRC_DIR)%.cpp $(SIM_OBJ_DIR)%.o | $(OBJ_DIR)
	rm -f $@
	cd workspace && \
	  dosbox \
	    -c "S:" \
	    -c "$(CC) $(CC_OPT) $< > $(CC_LOG)" \
	    -c "exit"
	mv $(UPPER_TGT) $@

