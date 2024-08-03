VERSION = debug
PLATFORM = 16bit

INCLUDE_DIRS = include/
SRC_DIRS = src/;support/;test/;imbibe/
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

.obj.exe:
        echo name $(OBJ_DIR)$^& > $(OBJ_DIR)$^&.lnk
        echo $(LINK_OPT) >> $(OBJ_DIR)$^&.lnk
        for %i in ($($^&_objs)) do echo file $(OBJ_DIR)%i >> $(OBJ_DIR)$^&.lnk
        $(LINK) @$(OBJ_DIR)$^&.lnk

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
# source dependencies
#

bitmap_objs = bitmap.obj
bitmap_graphics_objs = bitmap_graphics.obj $(bitmap_objs)
bin_bitmap_objs = bin_bitmap.obj $(bitmap_objs)
element_objs = element.obj
bitmap_element_objs = bitmap_element.obj $(bitmap_objs) $(element_objs)
cstream_objs = cstream.obj data.obj
timer_objs = timer.obj
idle_handler_objs = idle_handler.obj
idle_dispatcher_objs = idle_dispatcher.obj $(timer_objs) $(idle_handler_objs)
text_window_objs = text_window.obj $(bitmap_objs)
key_dispatcher_objs = key_dispatcher.obj $(idle_handler_objs) &
                           $(idle_dispatcher_objs)
menu_objs = menu.obj $(bin_bitmap_objs)
key_handler_objs =
rectangle_element_objs = rectangle_element.obj $(element_objs)
stop_handler_objs = stop_handler.obj $(key_handler_objs) $(idle_handler_objs)
menu_element_objs = menu_element.obj $(element_objs) $(menu_objs)
menu_handler_objs = menu_handler.obj $(key_handler_objs) $(menu_objs)
hbin_objs = hbin.obj $(bin_bitmap_objs) $(bitmap_objs)
hbin_element_objs = hbin_element.obj $(element_objs) $(bitmap_graphics_objs) &
                    $(hbin_objs) $(window_objs)
hbin_menu_handler_objs = hbin_menu_handler.obj $(key_handler_objs) &
                         $(hbin_objs) $(hbin_element_objs)
hbin_view_handler_objs = hbin_view_handler.obj $(key_handler_objs) &
                         $(hbin_objs) $(hbin_element_objs)

#
# data
#

data.obj: $(src_dir)data.cc $(include_dir)data.hh

$(src_dir)data.cc $(include_dir)data.hh: mkconst.exe pack.exe testdata/test.dsc
        cd testdata
#        ../$(obj_dir)txt2bin ../data/about.txt about.bin
        ../$(obj_dir)pack test.dsc test.pkg
        ../$(obj_dir)mkconst test.pkg data.hh data.cc
        del ../$(include_dir)data.hh
        del ../$(src_dir)data.cc
        move data.hh ../$(include_dir)data.hh
        move data.cc ../$(src_dir)data.cc
        cd ..

#
# imbibe
#

imbibe_objs = imbibe.obj $(bin_bitmap_objs) $(bitmap_objs) &
              $(bitmap_element_objs) $(bitmap_graphics_objs) &
              $(cstream_objs) $(data_objs) $(element_objs) &
              $(menu_element_objs) &
              $(idle_handler_objs) $(idle_dispatcher_objs) $(text_window_objs) &
              $(timer_objs)
imbibe.exe: $(imbibe_objs)

#
# support
#

mkconst_objs = mkconst.obj
mkconst.exe: $(mkconst_objs)

mkmenu_objs = mkmenu.obj
mkmenu.exe: $(mkmenu_objs)

mkms_objs = mkms.obj $(bitmap_objs) $(bitmap_graphics_objs)
mkms.exe: $(mkms_objs)

pack_objs = pack.obj
pack.exe: $(pack_objs)

qview_objs = qview.obj
qview.exe: $(qview_objs)

twm2bin_objs = twm2bin.obj $(bitmap_objs) $(bitmap_graphics_objs)
twm2bin.exe: $(twm2bin_objs)

txt2bin_objs = txt2bin.obj
txt2bin.exe: $(txt2bin_objs)

tmt2bin_objs = tmt2bin.obj
tmt2bin.exe: $(tmt2bin_objs)

mkhbin_objs = mkhbin.obj
mkhbin.exe: $(mkhbin_objs)

#
# test
#

maptest.exe:

test1_objs = test1.obj $(bin_bitmap_objs) $(bitmap_objs) &
             $(bitmap_element_objs) $(cstream_objs) $(element_objs) &
             $(text_window_objs)
test1.exe: $(test1_objs)

test2_objs = test2.obj $(bin_bitmap_objs) $(bitmap_objs) &
             $(bitmap_element_objs) $(cstream_objs) &
             $(idle_dispatcher_objs) $(text_window_objs)
test2.exe: $(test2_objs)

test3_objs = test3.obj $(bin_bitmap_objs) $(cstream_objs) &
             $(key_dispatcher_objs) $(menu_objs) $(menu_element_objs) &
             $(menu_handler_objs) $(rectangle_element_objs) &
             $(stop_handler_objs) $(idle_dispatcher_objs) $(text_window_objs)
test3.exe: $(test3_objs)

test4_objs = test4.obj $(bin_bitmap_objs) $(cstream_objs) $(hbin_objs) &
             $(hbin_element_objs) $(hbin_menu_handler_objs) &
             $(key_dispatcher_objs) $(rectangle_element_objs) &
             $(stop_handler_objs) $(idle_dispatcher_objs) $(text_window_objs)
test4.exe: $(test4_objs)

test4p_objs = test4p.obj no_timer.obj $(bin_bitmap_objs) $(cstream_objs) &
              $(hbin_objs) $(hbin_element_objs) $(hbin_menu_handler_objs) &
              $(key_dispatcher_objs) $(rectangle_element_objs) &
              $(stop_handler_objs) $(idle_dispatcher_objs) $(text_window_objs)
test4p.exe: $(test4p_objs)

test5_objs = test5.obj $(cstream_objs) $(hbin_objs) &
             $(hbin_element_objs) $(hbin_view_handler_objs) &
             $(key_handler_objs) $(key_dispatcher_objs) &
             $(rectangle_element_objs) $(stop_handler_objs) &
             $(idle_dispatcher_objs) $(text_window_objs)
test5.exe: $(test5_objs)

test6_objs = test6.obj $(cstream_objs) $(hbin_objs) &
             $(hbin_element_objs) $(hbin_view_handler_objs) &
             $(key_handler_objs) $(key_dispatcher_objs) &
             $(rectangle_element_objs) $(stop_handler_objs) &
             $(idle_dispatcher_objs) $(text_window_objs)
test6.exe: $(test6_objs)

keytest_objs = keytest.obj $(key_dispatcher_objs) $(key_handler_objs) &
               $(idle_handler_objs) $(idle_dispatcher_objs)
keytest.exe: $(keytest_objs)

timetest_objs = timetest.obj $(timer_objs)
timetest.exe: $(timetest_objs)

vectest_objs = vectest.obj
vectest.exe: $(vectest_objs)

