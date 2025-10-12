#include <debug.h>
#include <kernel.h>
#include <iopcontrol.h>
#include <loadfile.h>
#include <iopcontrol_special.h>
#include <sifrpc.h>
#include <sbv_patches.h>
#include <string.h>
#include <fileio.h>
#include <tamtypes.h>


#ifdef DEBUG
#include <sio.h>
extern int sio_printf(char* fmt, ...);
#define UART(X...) sio_printf(X)
#else
#define UART(X...)
#endif

extern u8 loader_elf[];
extern int size_loader_elf;
extern u8 ioprp_img[];
extern int size_ioprp_img;

// Runs the target ELF using the embedded ELF loader
int LoadELFFromFile(int argc, char *argv[]);

int main()
{
    FlushCache(0);
    FlushCache(2);
    UART("Flushed Cache\n");
    SifInitRpc(0);
    UART("SifInitRpc(0)\n");
    // here we don't care of _ps2sdk_memory_init as I do on all the other programs bc libcglue is not here
    UART("IOPRP Reboot: Start\n");
    while (!SifIopRebootBuffer(ioprp_img, size_ioprp_img)) {}; //embed the ioprp image with bin2c
    UART("IOPRP Reboot: Waiting\n");
    while (!SifIopSync()) {};
    UART("IOPRP Reboot: Ready to continue\n");
    SifInitRpc(0);
    UART("Disable Prefix Check\n");
    sbv_patch_disable_prefix_check();
    UART("LOAD: SIO2MAN\n");
    SifLoadStartModule("rom0:SIO2MAN", 0, NULL, NULL);
    UART("LOAD: DONGLEMAN\n");
    SifLoadStartModule("rom0:MCMAN", 0, NULL, NULL); //MCMAN: only mc0, MCMANO: Only mc1
    char *argv[1] = {
        "mc0:" BOOT_PATH,
    };

    // Make sure the file exists on either mc0 or mc1
    int fd = fioOpen(argv[0], 0x0001 /*FIO_O_RDONLY*/);
    if (fd < 0) {
        UART("FATAL: Doesnot exist '%s'\n", argv[0]);
        return -1;
    }

    UART("Exists: '%s'\n", argv[0]);
    fioClose(fd);

    LoadELFFromFile(1, argv);
    return -1;
    // here we just do nothing, if something went wrong, return from main() and let arcade OSDSYS do the error screen for us
}

typedef struct
{
    u8 ident[16]; // struct definition for ELF object header
    u16 type;
    u16 machine;
    u32 version;
    u32 entry;
    u32 phoff;
    u32 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} elf_header_t;

typedef struct
{
    u32 type; // struct definition for ELF program section header
    u32 offset;
    void *vaddr;
    u32 paddr;
    u32 filesz;
    u32 memsz;
    u32 flags;
    u32 align;
} elf_pheader_t;

#define ELF_MAGIC 0x464c457f
#define ELF_PT_LOAD 1

// Runs the target ELF using the embedded ELF loader
// Based on PS2SDK elf-loader
int LoadELFFromFile(int argc, char *argv[])
{
    u8 *boot_elf;
    elf_header_t *eh;
    elf_pheader_t *eph;
    void *pdata;
    int i;

    // Wipe memory region where the ELF loader is going to be loaded (see loader/linkfile)
    UART("Wipe usermem for ELF Loader\n");
    memset((void *)0x00084000, 0, 0x00100000 - 0x00084000);

    boot_elf = (u8 *)loader_elf;
    eh = (elf_header_t *)boot_elf;
    if (_lw((u32)&eh->ident) != ELF_MAGIC) {
        UART("FATAL: INVALID ELF MAGIC\n");
        return -1;
    }

    eph = (elf_pheader_t *)(boot_elf + eh->phoff);

    // Scan through the ELF program headers and copy them into RAM
    for (i = 0; i < eh->phnum; i++)
    {
        if (eph[i].type != ELF_PT_LOAD)
            continue;

        pdata = (void *)(boot_elf + eph[i].offset);
        memcpy(eph[i].vaddr, pdata, eph[i].filesz);
    }

    UART("Exit Services, flushCache 0 & 2\n");
    SifExitCmd();
    SifExitRpc();
    FlushCache(0);
    FlushCache(2);

    UART("ExecPS2(%p, 0, %d, %s)\n", (void *)eh->entry, argc, argv[0]);
    return ExecPS2((void *)eh->entry, NULL, argc, argv);
}

