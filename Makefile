VERSION = DEBUG

SIM_OBJ_DIR = build/simdebug/
SIM_DEP_DIR = build/simdeps/
SDL_GL_OBJ_DIR = build/sdlgldebug/
SDL_GL_DEP_DIR = build/sdlgldeps/

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

CC_OPT_RELEASE = -dNDEBUG -otaeilh -ms
CC_OPT_DEBUG = -d2 -osaeih -ml
CC_OPT_PROFILE = $(CC_OPT_RELEASE) -d1
CC_OPT = $(CC_OPT_$(VERSION)) -2 -bt=dos -fpc -w5 -ze

CC = wpp

DOSBOX = dosbox-x -fastlaunch
DOSBOX_BUILD = dosbox-x -fastlaunch -conf dosbox-build.conf 2> /dev/null

IMBIBE_HEADERS = $(wildcard $(SRC_DIR)*.h)
IMBIBE_SOURCES = $(wildcard $(SRC_DIR)*.cpp)

IMBIBE_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.obj,$(IMBIBE_SOURCES))
IMBIBE_SIM_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(SIM_OBJ_DIR)%.o,$(IMBIBE_SOURCES))
IMBIBE_SIM_DEPS = $(patsubst $(SRC_DIR)%.cpp,$(SIM_DEP_DIR)%.d,$(IMBIBE_SOURCES))
IMBIBE_SDL_GL_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(SDL_GL_OBJ_DIR)%.o,$(IMBIBE_SOURCES))
IMBIBE_SDL_GL_DEPS = $(patsubst $(SRC_DIR)%.cpp,$(SDL_GL_DEP_DIR)%.d,$(IMBIBE_SOURCES))

IMBIBE_RESOURCES = \
	$(patsubst data/%.bin,testdata/%.tbm,$(wildcard data/*/*.bin)) \
	$(patsubst data/%.bin,testdata/%.tbm,$(wildcard data/*/viewer/*.bin)) \
	$(patsubst data/%.json,testdata/%.cfg,$(wildcard data/*/*.json)) \
	$(patsubst data/%.json,testdata/%.cfg,$(wildcard data/*.json))

.PHONY: all clean deps dirs imbibe
.DEFAULT_GOAL: all

$(SIM_OBJ_DIR):
	mkdir -p $@
$(SIM_DEP_DIR):
	mkdir -p $@
$(SDL_GL_OBJ_DIR):
	mkdir -p $@
$(SDL_GL_DEP_DIR):
	mkdir -p $@
$(OBJ_DIR_RELEASE):
	mkdir -p $@
$(OBJ_DIR_DEBUG):
	mkdir -p $@
$(OBJ_DIR_PROFILE):
	mkdir -p $@

dirs: $(SIM_OBJ_DIR) $(SIM_DEP_DIR) $(SIM_OBJ_DIR) $(SDL_GL_DEP_DIR) $(OBJ_DIR)

$(SIM_OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(SIM_OBJ_DIR) $(SIM_DEP_DIR)
	g++ -std=gnu++17 -g -W -Wall -Werror -MT $@ -MMD -MP -MF $(SIM_DEP_DIR)$*.d -c $< -o $@

$(SDL_GL_OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(SDL_GL_OBJ_DIR) $(SDL_GL_DEP_DIR)
	g++ -std=gnu++17 -g -W -Wall -Werror -MT $@ -MMD -MP -MF $(SDL_GL_DEP_DIR)$*.d -DGLIMBIBE $(shell pkg-config --cflags sdl2) -c $< -o $@

$(SIM_DEP_DIR)%.d: $(SIM_OBJ_DIR)%.o

$(SDL_GL_DEP_DIR)%.d: $(SDL_GL_OBJ_DIR)%.o

include $(wildcard $(SIM_DEP_DIR)*.d)
include $(wildcard $(SDL_GL_DEP_DIR)*.d)

all: imbibe simbibe glimbibe

clean:
	rm -rf build

simbibe: $(IMBIBE_SIM_OBJS) $(IMBIBE_RESOURCES)
	g++  $(IMBIBE_SIM_OBJS) -g -o $@

glimbibe: $(IMBIBE_SDL_GL_OBJS) $(IMBIBE_RESOURCES)
	g++  $(IMBIBE_SDL_GL_OBJS) -g $(shell pkg-config --libs sdl2) -lGL -o $@

imbibe: $(OBJ_DIR)imbibe.exe $(IMBIBE_SDL_GL_OBJS)
	cd workspace && \
	  $(DOSBOX) \
	    -c "S:" \
	    -c "`echo $< | tr '/' '\\' ` > RUN.LOG"
#	    -c "exit"

testdata/%.tbm: data/%.bin support/mk_tbm.py Makefile
	mkdir -p $(dir $@)
	support/mk_tbm.py -v -o $@ $<

testdata/%.cfg: data/%.json support/mk_cfg.py Makefile
	mkdir -p $(dir $@)
	support/mk_cfg.py -v -r data -o $@ $<

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

# $(OBJ_DIR)%.exe: private UPPER_TGT = $(@D)/$(call uc,$(@F))
# $(OBJ_DIR)%.exe: private UNITY_SRC = $(@D)/u_$(patsubst %.exe,%.cpp,$(@F))
# $(OBJ_DIR)%.exe: private UNITY_LOG = $(@D)/u_$(patsubst %.exe,%.log,$(@F))
# $(OBJ_DIR)imbibe.exe: $(OBJ_DIR)u_imbibe.lnk $(OBJ_DIR)u_imbibe.cpp | $(OBJ_DIR)
# 	rm -f $@
# 	rm -f $(OBJ_DIR)*.OBJ
# 	TEMP_OBJ=$(OBJ_DIR)`mktemp -u -p . ~XXXXXX.obj | cut -c 3- | tr a-z A-Z` ; \
# 	  ( cd workspace && \
# 	    $(DOSBOX_BUILD) \
# 	      -c "S:" \
# 	      -c "$(CC) $(CC_OPT) -fo=$$TEMP_OBJ $(UNITY_SRC) > $(UNITY_LOG)" \
# 	      -c "move $$TEMP_OBJ $(OBJ_DIR)u_imbibe.obj" \
# 	      -c "$(LINK) @$< > $(LINK_LOG)" \
# 	      -c "wdis $(patsubst %.cpp,%.obj,$(UNITY_SRC)) -s -l=$(patsubst %.cpp,%.lst,$(UNITY_SRC))" \
# 	      -c "exit" )
# 	mv $(UPPER_TGT) $@

# $(OBJ_DIR)imbibe.exe: $(OBJ_DIR)u_imbibe.cpp | $(OBJ_DIR)
# 	rm -f $@
# 	rm -f $(OBJ_DIR)*.OBJ
# 	ia16-elf-g++ -std=gnu++17 -g -W -Wall -Werror -li86 -I. -o $@ $<


$(OBJ_DIR)%.obj: private CC_LOG = $(patsubst %.obj,%.log,$@)
$(OBJ_DIR)%.obj: $(SRC_DIR)%.cpp $(SIM_OBJ_DIR)%.o Makefile | $(OBJ_DIR)
	rm -f $@
	TEMP_OBJ=$(OBJ_DIR)`mktemp -u -p . ~XXXXXX.obj | cut -c 3- | tr a-z A-Z` ; \
	  ( cd workspace && \
	    $(DOSBOX_BUILD) \
	      -c "S:" \
	      -c "$(CC) $(CC_OPT) -fo=$$TEMP_OBJ $< > $(CC_LOG)" \
	      -c "wdis $$TEMP_OBJ -s -l=$(patsubst %.obj,%.lst,$@)" \
	      -c "exit" && \
	    cd .. && \
		mv "$$TEMP_OBJ" "$@" )

