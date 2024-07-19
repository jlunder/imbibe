version = debug
extender = pmodew
platform = 16bit

obj_dir = $(version)\

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
cc_opt_16bit = -2 -ms -bt=dos -fpc
cc_opt_32bit = -3r -mf -bt=dos -fpc
cc_opt = -fo=$(obj_dir) -i=$(include_dirs) $(cc_opt_$(version)) $(cc_opt_$(platform))

cc_16bit = wpp
cc_32bit = wpp386
cc = $(cc_$(platform))

asm_opt_prod = -dNDEBUG
asm_opt_debug = -d1
asm_opt_profile = -d1
asm_opt = -fo=$(obj_dir) -ms -bt=dos -fpc -2r $(asm_opt_$(version))

asm = wasm

#.cc: $(src_dirs)
.obj: $(obj_dir)
.exe: $(obj_dir)

.obj.exe:
        echo name $(obj_dir)$^& > $(obj_dir)$^&.lnk
        echo $(link_opt) >> $(obj_dir)$^&.lnk
        for %i in ($($^&_objs)) do echo file $(obj_dir)%i >> $(obj_dir)$^&.lnk
        $(link) @$(obj_dir)$^&.lnk

.cc.obj: .AUTODEPEND
        $(cc) $(cc_opt) $<

.asm.obj: .AUTODEPEND
        $(asm) $(asm_opt) $<

clean: .SYMBOLIC
        echo y | del $(obj_dir)*.*

clean_all: .SYMBOLIC
        echo y | del prod\*.*
        echo y | del debug\*.*
        echo y | del profile\*.*

test_objs = test.obj timer.obj
test.exe: $(test_objs)

