[   82.862288] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[   82.863877] Mem abort info:
[   82.864169]   ESR = 0x0000000096000045
[   82.864381]   EC = 0x25: DABT (current EL), IL = 32 bits
[   82.864783]   SET = 0, FnV = 0
[   82.864949]   EA = 0, S1PTW = 0
[   82.865097]   FSC = 0x05: level 1 translation fault
[   82.867045] Data abort info:
[   82.867452]   ISV = 0, ISS = 0x00000045
[   82.867866]   CM = 0, WnR = 1
[   82.868312] user pgtable: 4k pages, 39-bit VAs, pgdp=0000000043cbb000
[   82.868810] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
[   82.870089] Internal error: Oops: 96000045 [#1] PREEMPT SMP
[   82.871337] Modules linked in: scull(O) hello(O) faulty(O)
[   82.872949] CPU: 0 PID: 340 Comm: sh Tainted: G           O      5.15.124-yocto-standard #1
[   82.873881] Hardware name: linux,dummy-virt (DT)
[   82.882721] pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[   82.889400] pc : faulty_write+0x18/0x20 [faulty]
[   82.896614] lr : vfs_write+0xf8/0x29c
[   82.896997] sp : ffffffc0096f3d80
[   82.897113] x29: ffffffc0096f3d80 x28: ffffff80020cee00 x27: 0000000000000000
[   82.897473] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
[   82.898753] x23: 0000000000000000 x22: ffffffc0096f3dc0 x21: 0000005585e07a60
[   82.899065] x20: ffffff8003696c00 x19: 0000000000000012 x18: 0000000000000000
[   82.909482] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[   82.909652] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
[   82.909797] x11: 0000000000000000 x10: 0000000000000000 x9 : ffffffc008268e3c
[   82.918475] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
[   82.918743] x5 : 0000000000000001 x4 : ffffffc000b70000 x3 : ffffffc0096f3dc0
[   82.919229] x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000
[   82.920101] Call trace:
[   82.920535]  faulty_write+0x18/0x20 [faulty]
[   82.920880]  ksys_write+0x74/0x10c
[   82.932192]  __arm64_sys_write+0x24/0x30
[   82.933639]  invoke_syscall+0x5c/0x130
[   82.933862]  el0_svc_common.constprop.0+0x4c/0x100
[   82.935310]  do_el0_svc+0x4c/0xb4
[   82.935705]  el0_svc+0x28/0x80
[   82.936045]  el0t_64_sync_handler+0xa4/0x130
[   82.936344]  el0t_64_sync+0x1a0/0x1a4
[   82.936688] Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
[   82.939517] ---[ end trace 40fb5b385810e3ed ]---
Segmentation fault
