#include <stdlib.h>
#include <stdint.h>
#include <platform/malta/pci.h>
#include <platform/malta/gt64120.h>
#include <platform/malta/mips.h>
#include <platform/cpu.h>
#include <assert.h>

/*
 * Because of an error/peculiarity in the Galileo chip, we need to swap the
 * bytes when running bigendian.  We also provide non-swapping versions.
 */
#define __GT_READ(ofs)	        (*(volatile uint32_t *)(MIPS_PHYS_TO_KSEG1(MALTA_CORECTRL_BASE + (ofs))))
#define __GT_WRITE(ofs, data)	do { *(volatile uint32_t*)(MIPS_PHYS_TO_KSEG1(MALTA_CORECTRL_BASE + (ofs))) = (data); } while (0)

#define GT_READ(ofs)		    le32_to_cpu(__GT_READ(ofs))
#define GT_WRITE(ofs, data)	    __GT_WRITE(ofs, cpu_to_le32(data))

enum PCIConfigAccess {
    PCI_ACCESS_READ,
    PCI_ACCESS_WRITE
};

/* For reference look at: http://wiki.osdev.org/PCI */

static int gt64xxx_pci0_pcibios_config_access(enum PCIConfigAccess access, int bus, int dev, int func, int where, uint32_t* data)
{
    uint32_t intr = 0;

    if (dev == 0 && func >= 31) {
        /* Because of a bug in the galileo (for slot 31). */
        return -1;
    }

    /* Clear cause register bits */
    GT_WRITE(GT_INTRCAUSE_OFS, ~(GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT));

    /* Setup address */
//    uint32_t addr =
    GT_WRITE(GT_PCI0_CFGADDR_OFS,
    (bus << GT_PCI0_CFGADDR_BUSNUM_SHF) |
    (dev << 11) |
    (func << GT_PCI0_CFGADDR_FUNCTNUM_SHF) |
    ((where / 4) << GT_PCI0_CFGADDR_REGNUM_SHF) |
    GT_PCI0_CFGADDR_CONFIGEN_BIT);

    if (access == PCI_ACCESS_WRITE) {
        if (dev == 0 && func == 0) {
            /*
             * The Galileo system controller is acting
             * differently than other devices.
             */
            GT_WRITE(GT_PCI0_CFGDATA_OFS, *data);
        } else
            __GT_WRITE(GT_PCI0_CFGDATA_OFS, *data);
    } else {
        if (dev == 0 && func == 0) {
            /*
             * The Galileo system controller is acting
             * differently than other devices.
             */
            *data = GT_READ(GT_PCI0_CFGDATA_OFS);
        } else
            *data = __GT_READ(GT_PCI0_CFGDATA_OFS);
    }

    /* Check for master or target abort */
    intr = GT_READ(GT_INTRCAUSE_OFS);

    if (intr & (GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT)) {
        /* Error occurred */

        /* Clear bits */
        GT_WRITE(GT_INTRCAUSE_OFS, ~(GT_INTRCAUSE_MASABORT0_BIT |
                                     GT_INTRCAUSE_TARABORT0_BIT));

        return -1;
    }

    return 0;

}

/*
 * We can't address 8 and 16 bit words directly. Instead we have to
 * read/write a 32bit word and mask/modify the data we actually want.
 */
static int gt64xxx_pci0_pcibios_read(int bus, int dev, int func, int where, int size, uint32_t* data)
{
    uint32_t val = 0;

    if (gt64xxx_pci0_pcibios_config_access(PCI_ACCESS_READ, bus, dev, func, where, &val))
        return -1;

    if (size == 1) {
        *data = (val >> ((where & 3) << 3)) & 0xff;
    } else if (size == 2) {
        *data = (val >> ((where & 3) << 3)) & 0xffff;
    } else {
        *data = val;
    }

    return 0;
}

static int gt64xxx_pci0_pcibios_write(int bus, int dev, int func, int where, int size, uint32_t val)
{
    uint32_t data = 0;

    if (size == 4)
        data = val;
    else {
        if (gt64xxx_pci0_pcibios_config_access(PCI_ACCESS_READ, bus, dev, func, where, &data))
            return -1;

        if (size == 1) {
            data = (data & ~(0xff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
        } else if (size == 2) {
            data = (data & ~(0xffff << ((where & 3) << 3))) | (val << ((where & 3) << 3));
        }
    }

    if (gt64xxx_pci0_pcibios_config_access(PCI_ACCESS_WRITE, bus, dev, func, where, &data))
        return -1;

    return 0;
}


uint32_t platform_pci_bus_read_word(int dev, int devfn, int reg)
{
    uint32_t data = 0;
    int ret = gt64xxx_pci0_pcibios_read(0, dev, devfn, reg, sizeof(uint32_t), &data);
    assert(0 == ret);

    return data;
}

void platform_pci_bus_write_word(int dev, int devfn, int reg, uint32_t value)
{
    __attribute((unused))
    int ret = gt64xxx_pci0_pcibios_write(0, dev, devfn, reg, sizeof(uint32_t), value);
    assert(0 == ret);
}

uint16_t platform_pci_bus_read_half(int dev, int devfn, int reg)
{
    uint32_t data = 0;
    int ret = gt64xxx_pci0_pcibios_read(0, dev, devfn, reg, sizeof(uint16_t), &data);
    assert(0 == ret);

    return (uint16_t)(data & 0xFFFF);
}

void platform_pci_bus_write_half(int dev, int devfn, int reg, uint16_t value)
{
    __attribute((unused))
    int ret = gt64xxx_pci0_pcibios_write(0, dev, devfn, reg, sizeof(uint16_t), value);
    assert(0 == ret);
}

uint8_t platform_pci_bus_read_byte(int dev, int devfn, int reg)
{
    uint32_t data = 0;
    int ret = gt64xxx_pci0_pcibios_read(0, dev, devfn, reg, sizeof(uint8_t), &data);
    assert(0 == ret);

    return (uint8_t)(data & 0xFF);
}

void platform_pci_bus_write_byte(int dev, int devfn, int reg, uint8_t value)
{
    __attribute((unused))
    int ret = gt64xxx_pci0_pcibios_write(0, dev, devfn, reg, sizeof(uint8_t), value);
    assert(0 == ret);
}

intptr_t platform_pci_memory_base(void)
{
    return MALTA_PCI0_MEMORY_BASE;
}

size_t platform_pci_memory_size(void)
{
    return MALTA_PCI0_MEMORY_SIZE;
}

intptr_t platform_pci_io_base(void)
{
    return PCI_IO_SPACE_BASE;
//    return MALTA_PCI0_IO_BASE;
}

size_t platform_pci_io_size(void)
{
    return MALTA_PCI0_IO_SIZE;
}
