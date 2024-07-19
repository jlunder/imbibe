version = debug
extender = pmodew
platform = 16bit

include_dirs = include\
src_dirs = src\;support\;test\;imbibe\
obj_dir = $(platform)\$(version)\
include_dir = include\
src_dir = src\

link_opt_prod = 
link_opt_debug = d all
link_opt_profile = d all
link_opt_16bit = sys dos
link_opt_32bit = op st=64K sys $(extender)
link_opt = op elim $(link_opt_$(version)) $(link_opt_$(platform))

link_16bit = wlink
link_32bit = wlink
link = $(link_$(platform))

cc_opt_prod = -dNDEBUG -otexan
cc_opt_debug = -d3
cc_opt_profile = -dNDEBUG -d1 -otexan
cc_opt_16bit = -2 -ml -bt=dos -fpc
cc_opt_32bit = -3r -mf -bt=dos -fpc
cc_opt = -fo=$(obj_dir) -i=$(include_dirs) $(cc_opt_$(version)) $(cc_opt_$(platform))

cc_16bit = wpp
cc_32bit = wpp386
cc = $(cc_$(platform))

.cc: $(src_dirs)
.obj: $(obj_dir)
.exe: $(obj_dir)

.obj.exe:
        echo name $(obj_dir)$^& > $(obj_dir)$^&.lnk
        echo $(link_opt) >> $(obj_dir)$^&.lnk
        for %i in ($($^&_objs)) do echo file $(obj_dir)%i >> $(obj_dir)$^&.lnk
        $(link) @$(obj_dir)$^&.lnk

.cc.obj: .AUTODEPEND
        $(cc) $(cc_opt) $<

clean: .SYMBOLIC
        echo y | del $(obj_dir)*.*

clean_all: .SYMBOLIC
        echo y | del 16bit\prod\*.*
        echo y | del 16bit\debug\*.*
        echo y | del 16bit\profile\*.*
        echo y | del 32bit\prod\*.*
        echo y | del 32bit\debug\*.*
        echo y | del 32bit\profile\*.*

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

$(src_dir)data.cc $(include_dir)data.hh: mkconst.exe pack.exe testdata\test.dsc
        cd testdata
#        ..\$(obj_dir)txt2bin ..\data\about.txt about.bin
        ..\$(obj_dir)pack test.dsc test.pkg
        ..\$(obj_dir)mkconst test.pkg data.hh data.cc
        del ..\$(include_dir)data.hh
        del ..\$(src_dir)data.cc
        move data.hh ..\$(include_dir)data.hh
        move data.cc ..\$(src_dir)data.cc
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

