VERSION = debug

OBJ_DIR = build/$(VERSION)/
SRC_DIR = src/

LINK_OPT_RELEASE = 
LINK_OPT_DEBUG = d all
LINK_OPT_PROFILE = d all
LINK_OPT = op elim sys dos $(LINK_OPT_$(PLATFORM))

LINK = wlink

CC_OPT_RELEASE = -dNDEBUG -otexan
CC_OPT_DEBUG = -d3
CC_OPT_PROFILE = -dNDEBUG -d1 -otexan
CC_OPT = -fo=$(OBJ_DIR) $(CC_OPT_$(VERSION)) -2 -ml -bt=dos -fpc

CC = wpp

imbibe_hhs = $(wildcard src/*.)

imbibe_objs = $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.obj,$(wildcard src/*.cpp))

.PHONY: all clean imbibe
.DEFAULT_GOAL: all

all: imbibe

clean:
	rm -rf build

imbibe: $(OBJ_DIR)imbibe.exe
	cd workspace && \
	  dosbox \
	    -c "S:" \
	    -c `echo $< | tr / \\` \

$(OBJ_DIR)imbibe.lnk: | $(OBJ_DIR)
	echo name $(OBJ_DIR)imbibe > $@
	echo $(LINK_OPT) >> $@
	for i in $(imbibe_objs); do \
	    echo file $$i >> $@; \
	  done

$(OBJ_DIR)imbibe.exe: $(OBJ_DIR)imbibe.lnk $(imbibe_objs)
	cd workspace && \
	  dosbox \
	    -c "S:" \
	    -c "$(LINK) @$<" \
	    -c "exit"
	UPPER_TGT=$(@D)/`echo $(@F) | tr a-z A-Z`; \
	  if test -f $$UPPER_TGT; \
	    then mv $$UPPER_TGT $@; fi

$(OBJ_DIR)%.obj: $(SRC_DIR)%.cpp $(imbibe_hhs) | $(OBJ_DIR)
	cd workspace && \
	  dosbox \
	    -c "S:" \
	    -c "$(CC) $(CC_OPT) $<" \
	    -c "exit"
	UPPER_TGT=$(@D)/`echo $(@F) | tr a-z A-Z`; \
	  if test -f $$UPPER_TGT; \
	    then mv $$UPPER_TGT $@; fi

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

