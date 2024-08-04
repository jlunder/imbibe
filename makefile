VERSION = debug
PLATFORM = 16bit

INCLUDE_DIRS = include/
SRC_DIRS = src/;support/;test/
OBJ_DIR = $(PLATFORM)/$(VERSION)/
INCLUDE_DIR = include/
SRC_DIR = src/

LINK_OPT_RELEASE = 
LINK_OPT_DEBUG = d all
LINK_OPT_PROFILE = d all
LINK_OPT_16BIT = sys dos
LINK_OPT_32BIT = op st=64K sys dos4gw
LINK_OPT = op elim $(LINK_OPT_$(VERSION)) $(LINK_OPT_$(PLATFORM))

LINK_16BIT = wlink
LINK_32BIT = wlink
LINK = $(LINK_$(PLATFORM))

CC_OPT_RELEASE = -dNDEBUG -otexan
CC_OPT_DEBUG = -d3
CC_OPT_PROFILE = -dNDEBUG -d1 -otexan
CC_OPT = -fo=$(OBJ_DIR) -i=$(INCLUDE_DIRS) $(CC_OPT_$(VERSION)) -2 -ml -bt=dos -fpc

CC_16BIT = wpp
CC_32BIT = wpp386
CC = $(CC_$(PLATFORM))

.cc: $(SRC_DIRS)
.obj: $(OBJ_DIR)
.exe: $(OBJ_DIR)

.cc.obj: .AUTODEPEND
        $(CC) $(CC_OPT) $<

clean: .SYMBOLIC
        echo y | del $(OBJ_DIR)*.*

clean_all: .SYMBOLIC
        echo y | del 16bit/release/*.*
        echo y | del 16bit/debug/*.*
        echo y | del 16bit/profile/*.*
        echo y | del 32bit/release/*.*
        echo y | del 32bit/debug/*.*
        echo y | del 32bit/profile/*.*

#
# data
#

# data.obj: $(SRC_DIR)data.cc $(INCLUDE_DIR)data.hh

#$(SRC_DIR)data.cc $(INCLUDE_DIR)data.hh: mkconst.exe pack.exe testdata/test.dsc
#        cd testdata
##        ../$(OBJ_DIR)txt2bin ../data/about.txt about.bin
#        ../$(OBJ_DIR)pack test.dsc test.pkg
#        ../$(OBJ_DIR)mkconst test.pkg data.hh data.cc
#        del ../$(INCLUDE_DIR)data.hh
#        del ../$(SRC_DIR)data.cc
#        move data.hh ../$(INCLUDE_DIR)data.hh
#        move data.cc ../$(SRC_DIR)data.cc
#        cd ..

#
# imbibe
#

imbibe_objs = &
  $(OBJ_DIR)bin_bitm.obj &
  $(OBJ_DIR)bitmap.obj   &
  $(OBJ_DIR)bitmap_e.obj &
  $(OBJ_DIR)bitmap_g.obj &
  $(OBJ_DIR)cstream.obj  &
  $(OBJ_DIR)element.obj  &
  $(OBJ_DIR)hbin.obj     &
  $(OBJ_DIR)hbin_ele.obj &
  $(OBJ_DIR)hbin_men.obj &
  $(OBJ_DIR)hbin_vie.obj &
  $(OBJ_DIR)imbibe.obj   &
  $(OBJ_DIR)key_disp.obj &
  $(OBJ_DIR)menu.obj     &
  $(OBJ_DIR)menu_ele.obj &
  $(OBJ_DIR)menu_han.obj &
  $(OBJ_DIR)rectangl.obj &
  $(OBJ_DIR)task.obj     &
  $(OBJ_DIR)task_man.obj &
  $(OBJ_DIR)text_win.obj &
  $(OBJ_DIR)timed_ta.obj &
  $(OBJ_DIR)timer.obj    &
  &
  $(OBJ_DIR)data.obj

$(OBJ_DIR)imbibe.exe: $(imbibe_objs)
        echo name $(OBJ_DIR)$^& > $(OBJ_DIR)$^&.lnk
        echo $(LINK_OPT) >> $(OBJ_DIR)$^&.lnk
        for %i in ($(imbibe_objs)) do echo file %i >> $(OBJ_DIR)$^&.lnk
        $(LINK) @$(OBJ_DIR)$^&.lnk

imbibe: $(OBJ_DIR)imbibe.exe .SYMBOLIC

