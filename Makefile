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

CC_OPT_RELEASE = -dNDEBUG -otexan -mc
CC_OPT_DEBUG = -d3i -od -ml
CC_OPT_PROFILE = -dNDEBUG -d1 -otexan -mc
CC_OPT = $(CC_OPT_$(VERSION)) -2 -bt=dos -fpc -w5 -ze

CC = wpp

DOSBOX = dosbox-x
DOSBOX_BUILD = dosbox-x -conf dosbox-build.conf

IMBIBE_HEADERS = $(wildcard $(SRC_DIR)*.h)
IMBIBE_SOURCES = $(wildcard $(SRC_DIR)*.cpp)

IMBIBE_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.obj,$(IMBIBE_SOURCES))
IMBIBE_SIM_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(SIM_OBJ_DIR)%.o,$(IMBIBE_SOURCES))
IMBIBE_DEPS = $(patsubst $(SRC_DIR)%.cpp,$(DEP_DIR)%.d,$(IMBIBE_SOURCES))

IMBIBE_RESOURCES = \
	testdata/cover.tbm \
	testdata/logo.tbm \
	testdata/menu-bg.tbm \
	testdata/menu-sel.tbm \
	testdata/quit.tbm

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

simbibe: $(IMBIBE_SIM_OBJS) $(IMBIBE_RESOURCES)
	g++ -std=gnu++98 -g -o $@ $(IMBIBE_SIM_OBJS)

imbibe: $(OBJ_DIR)imbibe.exe $(IMBIBE_RESOURCES)
	cd workspace && \
	  $(DOSBOX) \
	    -c "S:" \
	    -c "`echo $< | tr '/' '\\' ` > RUN.LOG" \
	    -c "exit"

%.tbm: %.bin support/mk_tbm.py Makefile
	support/mk_tbm.py -v -o $@ $<

$(OBJ_DIR)imbibe.lnk: Makefile | $(OBJ_DIR)
	echo NAME $(OBJ_DIR)imbibe.exe > $@ && \
	  echo SYSTEM DOS >> $@ && \
	  echo OPTION ELIMINATE >> $@ && \
	  echo OPTION MAP=$(OBJ_DIR)imbibe.map >> $@ && \
	  echo $(LINK_OPT) >> $@ && \
	  for i in $(IMBIBE_OBJS); do \
	    echo FILE $$i >> $@; \
	  done

# $(patsubst $(SRC_DIR)%,%,$^)
$(OBJ_DIR)u_imbibe.cpp: $(IMBIBE_SOURCES) $(IMBIBE_HEADERS) Makefile | $(OBJ_DIR)
	echo '// Unity build file -- '$@ > $@ && \
	  for i in $(IMBIBE_SOURCES); do \
	    echo '#include "'$$i'"' >> $@; \
	  done

$(OBJ_DIR)u_imbibe.lnk: Makefile | $(OBJ_DIR)
	echo NAME $(OBJ_DIR)imbibe.exe > $@ && \
	  echo SYSTEM DOS >> $@ && \
	  echo OPTION ELIMINATE >> $@ && \
	  echo OPTION MAP=$(OBJ_DIR)u_imbibe.map >> $@ && \
	  echo $(LINK_OPT) >> $@ && \
	  echo FILE $(OBJ_DIR)u_imbibe.obj >> $@


uc = $(shell echo $(1) | tr a-z A-Z)

$(OBJ_DIR)%.exe: private UPPER_TGT = $(@D)/$(call uc,$(@F))
$(OBJ_DIR)%.exe: private LINK_LOG = $(@D)/l_$(patsubst %.exe,%.log,$(@F))
$(OBJ_DIR)imbibe.exe: $(OBJ_DIR)imbibe.lnk $(IMBIBE_OBJS) Makefile
	rm -f $@
	cd workspace && \
	  $(DOSBOX_BUILD) \
	    -c "S:" \
	    -c "$(LINK) @$< > $(LINK_LOG)" \
	    -c "exit"
	mv $(UPPER_TGT) $@


# $(OBJ_DIR)%.exe: private UNITY_SRC = $(@D)/u_$(patsubst %.exe,%.cpp,$(@F))
# $(OBJ_DIR)%.exe: private UNITY_LOG = $(@D)/u_$(patsubst %.exe,%.log,$(@F))
# $(OBJ_DIR)imbibe.exe: $(OBJ_DIR)u_imbibe.lnk $(OBJ_DIR)u_imbibe.cpp | $(OBJ_DIR)
# 	rm -f $@
# 	rm -f $(OBJ_DIR)U_IMBIBE.OBJ
# 	cd workspace && \
# 	  $(DOSBOX_BUILD) \
# 	    -c "S:" \
# 	    -c "$(CC) $(CC_OPT) $(UNITY_SRC) > $(UNITY_LOG)" \
# 	    -c "$(LINK) @$< > $(LINK_LOG)" \
# 	    -c "wdis $(patsubst %.cpp,%.obj,$(UNITY_SRC)) -s -l=$(patsubst %.cpp,%.lst,$(UNITY_SRC)) >> $(UNITY_LOG)" \
# 	    -c "exit"
# 	mv $(UPPER_TGT) $@

$(OBJ_DIR)%.obj: private CC_LOG = $(patsubst %.obj,%.log,$@)
$(OBJ_DIR)%.obj: $(SRC_DIR)%.cpp $(SIM_OBJ_DIR)%.o Makefile | $(OBJ_DIR)
	rm -f $@
	TEMP_OBJ=$(OBJ_DIR)`mktemp -u -p . ~XXXXXX.obj | cut -c 3- | tr a-z A-Z` ; \
	  ( cd workspace && \
	    $(DOSBOX_BUILD) \
	      -c "S:" \
	      -c "$(CC) $(CC_OPT) -fo=$$TEMP_OBJ $< > $(CC_LOG)" \
	      -c "wdis $$TEMP_OBJ -s -l=$(patsubst %.obj,%.lst,$@) >> $(CC_LOG)" \
	      -c "exit" && \
	    cd .. && \
		mv "$$TEMP_OBJ" "$@" )

