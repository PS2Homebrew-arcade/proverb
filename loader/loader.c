//--------------------------------------------------------------
//File name:    loader.c
//--------------------------------------------------------------
//dlanor: This subprogram has been modified to minimize the code
//dlanor: size of the resident loader portion. Some of the parts
//dlanor: that were moved into the main program include loading
//dlanor: of all IRXs and mounting pfs0: for ELFs on hdd.
//dlanor: Another change was to skip threading in favor of ExecPS2
/*==================================================================
==											==
==	Copyright(c)2004  Adam Metcalf(gamblore_@hotmail.com)		==
==	Copyright(c)2004  Thomas Hawcroft(t0mb0la@yahoo.com)		==
==	This file is subject to terms and conditions shown in the	==
==	file LICENSE which should be kept in the top folder of	==
==	this distribution.							==
==											==
==	Portions of this code taken from PS2Link:				==
==				pkoLoadElf						==
==				wipeUserMemory					==
==				(C) 2003 Tord Lindstrom (pukko@home.se)	==
==				(C) 2003 adresd (adresd_ps2dev@yahoo.com)	==
==	Portions of this code taken from Independence MC exploit	==
==				tLoadElf						==
==				LoadAndRunHDDElf					==
==				(C) 2003 Marcus Brown <mrbrown@0xd6.org>	==
==											==
==================================================================*/
#include "tamtypes.h"
#include "debug.h"
#include "kernel.h"
#include "iopcontrol.h"
#include "sifrpc.h"
#include "loadfile.h"
#include "string.h"
#include "iopheap.h"
#include "errno.h"
//--------------------------------------------------------------

//--------------------------------------------------------------
//End of data declarations
//--------------------------------------------------------------
//Start of function code:
//--------------------------------------------------------------
// Clear user memory
// PS2Link (C) 2003 Tord Lindstrom (pukko@home.se)
//         (C) 2003 adresd (adresd_ps2dev@yahoo.com)
//--------------------------------------------------------------
static void wipeUserMem(void)
{
	int i;
	for (i = 0x100000; i < GetMemorySize(); i += 64) {
		asm volatile(
		    "\tsq $0, 0(%0) \n"
		    "\tsq $0, 16(%0) \n"
		    "\tsq $0, 32(%0) \n"
		    "\tsq $0, 48(%0) \n" ::"r"(i));
	}
}

//--------------------------------------------------------------
//End of func:  void wipeUserMem(void)
//--------------------------------------------------------------
// *** MAIN ***
//--------------------------------------------------------------
int main(int argc, char *argv[])
{
    if (argc < 1)
    {
        return -EINVAL;
    }

    // Wipe the user memory
    wipeUserMem();

    // Apply the patch required to load executables from memory cards
    // (memory card modules are already loaded by OSDSYS)
    int ret;
    SifInitRpc(0);
    //if ((ret = sbv_patch_disable_prefix_check()))
    //    return ret;

    // Writeback data cache before loading the ELF.
    FlushCache(0);

    // Load the ELF into memory
    static t_ExecData elfdata;
    elfdata.epc = 0;
    SifLoadFileInit();
    ret = SifLoadElf(argv[0], &elfdata);
    SifLoadFileExit();

    // Reboot the IOP
    while (!SifIopReset("", 0))
    {
    };
    while (!SifIopSync())
    {
    };

    if (ret == 0 && elfdata.epc != 0)
    {
        // Execute the ELF
        FlushCache(0);
        FlushCache(2);
        return ExecPS2((void *)elfdata.epc, (void *)elfdata.gp, argc, argv);
    }
    else
    {
        return -ENOENT;
    }
}
//--------------------------------------------------------------
//End of func:  int main(int argc, char *argv[])
//--------------------------------------------------------------
//End of file:  loader.c
//--------------------------------------------------------------
