#ifndef __MALTA_H__
#define __MALTA_H__

/*
 * Malta Memory Map:
 *
 *  0000.0000   128MB   Basic SDRAM
 *  0800.0000   128MB   Extra SDRAM
 *  1000.0000   128MB   PCI memory space
 *  1800.0000    62MB   PCI I/O space
 *  1be0.0000     2MB   System Controller's internal registers
 *  1e00.0000     4MB   Monitor Flash
 *  1f00.0000    12MB   Switches
 *                      LEDs
 *                      ASCII display
 *                      Soft reset
 *                      FPGA revision number
 *                      CBUS UART (tty2)
 *                      General Purpose I/O
 *                      I2C controller
 *  1f10.0000    11MB   System Controller specific
 *  1fc0.0000     4MB   Monitor Flash alias
 *  1fd0.0000     3MB   System Controller specific
 *
 * CPU interrupts:
 *
 *  NMI     PIIX4 NMI or NMI button
 *    0     PIIX4 Interrupt Request
 *    1     PIIX4 System Management Interrupt
 *    2     CBUS UART2
 *    3     COREHI (Core Card)
 *    4     CORELO (Core Card)
 *    5     Internal CPU timer interrupt
 *
 * http://wiki.osdev.org/8259_PIC
 *
 * PIIX4 master PIC 8259A:
 *
 *    0      System Timer      PIIX4
 *    1      Keyboard          Super I/O
 *    2      –                 Reserved by PIIX4 (for cascading)
 *    3      UART1             Super I/O
 *    4      UART0             Super I/O
 *    5      –                 Not used
 *    6      Floppy Disk       Super I/O
 *    7      Parallel Port     Super I/O
 *
 * PIIX4 slave PIC 8259A:
 *
 *    8      Real Time Clock   PIIX4
 *    9      I²C bus           PIIX4
 *   10      PCI INTA#,INTB#   Ethernet controller
 *   11      PCI INTC#,INTD#   USB controller (PIIX4)
 *   12      Mouse             Super I/O
 *   13      –                 Reserved by PIIX4
 *   14      Primary IDE       PIIX4
 *   15      Secondary IDE     PIIX4
 */

#define MALTA_PHYS_SDRAM_BASE   0x00000000

#define MALTA_PCI0_MEMORY_BASE  0x10000000
#define MALTA_PCI0_MEMORY_SIZE  (128 * 1024 * 1024)
#define MALTA_PCI0_IO_BASE      0x18000000
#define MALTA_PCI0_IO_SIZE      (64 * 1024 * 1024)
#define MALTA_CORECTRL_BASE     0x1be00000
#define MALTA_FPGA_BASE         0x1f000000

#define MALTA_CBUS_UART         (MALTA_FPGA_BASE + 0x900)
#define MALTA_CBUS_UART_INTR    2

#define MALTA_PCI0_IO_ADDR(x)   (MALTA_PCI0_IO_BASE + (x))
#define MALTA_PCI0_MEM_ADDR(x)  (MALTA_PCI0_MEMORY_BASE + (x))

/* Intel 82371EB: RTC (MC146818) */
#define MALTA_RTC_ADDR          MALTA_PCI0_ADDR(0x70)
#define MALTA_RTC_DATA          MALTA_PCI0_ADDR(0x71)
/* FDC37M817: UART (NS16550) */
#define MALTA_SMSC_UART0        MALTA_PCI0_ADDR(0x3f8)
#define MALTA_SMSC_UART1        MALTA_PCI0_ADDR(0x2f8)
/* FDC37M817: Keyboard and Mouse (i8042) */
#define MALTA_SMSC_KYBD_DATA    MALTA_PCI0_ADDR(0x60)
#define MALTA_SMSC_KYBD_CTRL    MALTA_PCI0_ADDR(0x64)

#define MALTA_REVISION                      0x1fc00010
#define MALTA_REVISION_CORID_SHF            10
#define MALTA_REVISION_CORID_MSK            (0x3f << MALTA_REVISION_CORID_SHF)
#define MALTA_REVISION_CORID_CORE_LV        1
#define MALTA_REVISION_CORID_CORE_FPGA6     14

#define PCI_CFG_PIIX4_PIRQRCA		0x60
#define PCI_CFG_PIIX4_PIRQRCB		0x61
#define PCI_CFG_PIIX4_PIRQRCC		0x62
#define PCI_CFG_PIIX4_PIRQRCD		0x63
#define PCI_CFG_PIIX4_SERIRQC		0x64
#define PCI_CFG_PIIX4_GENCFG		0xb0

#define PCI_CFG_PIIX4_SERIRQC_EN	(1 << 7)
#define PCI_CFG_PIIX4_SERIRQC_CONT	(1 << 6)

#define PCI_CFG_PIIX4_GENCFG_SERIRQ	(1 << 16)

#define PCI_CFG_PIIX4_IDETIM_PRI	0x40
#define PCI_CFG_PIIX4_IDETIM_SEC	0x42

#define PCI_CFG_PIIX4_IDETIM_IDE	(1 << 15)

#endif
