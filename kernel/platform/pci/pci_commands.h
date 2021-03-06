/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PCI_COMMANDS_H__
#define __PCI_COMMANDS_H__

#define PCI_CFG_SPACE_SIZE	256
#define PCI_CFG_SPACE_EXP_SIZE	4096

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define PCI_VENDOR_ID		0x00	/* 16 bits */
#define PCI_DEVICE_ID		0x02	/* 16 bits */
#define PCI_COMMAND		0x04	/* 16 bits */
#define  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY	0x2	/* Enable response in Memory space */
#define  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE 0x10	/* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#define  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#define  PCI_COMMAND_WAIT	0x80	/* Enable address/data stepping */
#define  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#define  PCI_COMMAND_FAST_BACK	0x200	/* Enable back-to-back writes */

#define PCI_STATUS		0x06	/* 16 bits */
#define  PCI_STATUS_CAP_LIST	0x10	/* Support Capability List */
#define  PCI_STATUS_66MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF		0x40	/* Support User Definable Features [obsolete] */
#define  PCI_STATUS_FAST_BACK	0x80	/* Accept fast-back to back */
#define  PCI_STATUS_PARITY	0x100	/* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK 0x600	/* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST 0x000
#define  PCI_STATUS_DEVSEL_MEDIUM 0x200
#define  PCI_STATUS_DEVSEL_SLOW 0x400
#define  PCI_STATUS_SIG_TARGET_ABORT 0x800 /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT 0x1000 /* Master ack of " */
#define  PCI_STATUS_REC_MASTER_ABORT 0x2000 /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR 0x4000 /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY 0x8000 /* Set on parity error */

#define PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8
					   revision */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	0x0a	/* Device class */
#define PCI_CLASS_CODE		0x0b	/* Device class code */
#define  PCI_CLASS_CODE_TOO_OLD	0x00
#define  PCI_CLASS_CODE_STORAGE 0x01
#define  PCI_CLASS_CODE_NETWORK 0x02
#define  PCI_CLASS_CODE_DISPLAY	0x03
#define  PCI_CLASS_CODE_MULTIMEDIA 0x04
#define  PCI_CLASS_CODE_MEMORY	0x05
#define  PCI_CLASS_CODE_BRIDGE	0x06
#define  PCI_CLASS_CODE_COMM	0x07
#define  PCI_CLASS_CODE_PERIPHERAL 0x08
#define  PCI_CLASS_CODE_INPUT	0x09
#define  PCI_CLASS_CODE_DOCKING	0x0A
#define  PCI_CLASS_CODE_PROCESSOR 0x0B
#define  PCI_CLASS_CODE_SERIAL	0x0C
#define  PCI_CLASS_CODE_WIRELESS 0x0D
#define  PCI_CLASS_CODE_I2O	0x0E
#define  PCI_CLASS_CODE_SATELLITE 0x0F
#define  PCI_CLASS_CODE_CRYPTO	0x10
#define  PCI_CLASS_CODE_DATA	0x11
/* Base Class 0x12 - 0xFE is reserved */
#define  PCI_CLASS_CODE_OTHER	0xFF

#define PCI_CLASS_SUB_CODE	0x0a	/* Device sub-class code */
#define  PCI_CLASS_SUB_CODE_TOO_OLD_NOTVGA	0x00
#define  PCI_CLASS_SUB_CODE_TOO_OLD_VGA		0x01
#define  PCI_CLASS_SUB_CODE_STORAGE_SCSI	0x00
#define  PCI_CLASS_SUB_CODE_STORAGE_IDE		0x01
#define  PCI_CLASS_SUB_CODE_STORAGE_FLOPPY	0x02
#define  PCI_CLASS_SUB_CODE_STORAGE_IPIBUS	0x03
#define  PCI_CLASS_SUB_CODE_STORAGE_RAID	0x04
#define  PCI_CLASS_SUB_CODE_STORAGE_ATA		0x05
#define  PCI_CLASS_SUB_CODE_STORAGE_SATA	0x06
#define  PCI_CLASS_SUB_CODE_STORAGE_SAS		0x07
#define  PCI_CLASS_SUB_CODE_STORAGE_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_NETWORK_ETHERNET	0x00
#define  PCI_CLASS_SUB_CODE_NETWORK_TOKENRING	0x01
#define  PCI_CLASS_SUB_CODE_NETWORK_FDDI	0x02
#define  PCI_CLASS_SUB_CODE_NETWORK_ATM		0x03
#define  PCI_CLASS_SUB_CODE_NETWORK_ISDN	0x04
#define  PCI_CLASS_SUB_CODE_NETWORK_WORLDFIP	0x05
#define  PCI_CLASS_SUB_CODE_NETWORK_PICMG	0x06
#define  PCI_CLASS_SUB_CODE_NETWORK_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_DISPLAY_VGA		0x00
#define  PCI_CLASS_SUB_CODE_DISPLAY_XGA		0x01
#define  PCI_CLASS_SUB_CODE_DISPLAY_3D		0x02
#define  PCI_CLASS_SUB_CODE_DISPLAY_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_MULTIMEDIA_VIDEO	0x00
#define  PCI_CLASS_SUB_CODE_MULTIMEDIA_AUDIO	0x01
#define  PCI_CLASS_SUB_CODE_MULTIMEDIA_PHONE	0x02
#define  PCI_CLASS_SUB_CODE_MULTIMEDIA_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_MEMORY_RAM		0x00
#define  PCI_CLASS_SUB_CODE_MEMORY_FLASH	0x01
#define  PCI_CLASS_SUB_CODE_MEMORY_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_BRIDGE_HOST		0x00
#define  PCI_CLASS_SUB_CODE_BRIDGE_ISA		0x01
#define  PCI_CLASS_SUB_CODE_BRIDGE_EISA		0x02
#define  PCI_CLASS_SUB_CODE_BRIDGE_MCA		0x03
#define  PCI_CLASS_SUB_CODE_BRIDGE_PCI		0x04
#define  PCI_CLASS_SUB_CODE_BRIDGE_PCMCIA	0x05
#define  PCI_CLASS_SUB_CODE_BRIDGE_NUBUS	0x06
#define  PCI_CLASS_SUB_CODE_BRIDGE_CARDBUS	0x07
#define  PCI_CLASS_SUB_CODE_BRIDGE_RACEWAY	0x08
#define  PCI_CLASS_SUB_CODE_BRIDGE_SEMI_PCI	0x09
#define  PCI_CLASS_SUB_CODE_BRIDGE_INFINIBAND	0x0A
#define  PCI_CLASS_SUB_CODE_BRIDGE_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_COMM_SERIAL		0x00
#define  PCI_CLASS_SUB_CODE_COMM_PARALLEL	0x01
#define  PCI_CLASS_SUB_CODE_COMM_MULTIPORT	0x02
#define  PCI_CLASS_SUB_CODE_COMM_MODEM		0x03
#define  PCI_CLASS_SUB_CODE_COMM_GPIB		0x04
#define  PCI_CLASS_SUB_CODE_COMM_SMARTCARD	0x05
#define  PCI_CLASS_SUB_CODE_COMM_OTHER		0x80
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_PIC	0x00
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_DMA	0x01
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_TIMER	0x02
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_RTC	0x03
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_HOTPLUG	0x04
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_SD	0x05
#define  PCI_CLASS_SUB_CODE_PERIPHERAL_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_INPUT_KEYBOARD	0x00
#define  PCI_CLASS_SUB_CODE_INPUT_DIGITIZER	0x01
#define  PCI_CLASS_SUB_CODE_INPUT_MOUSE		0x02
#define  PCI_CLASS_SUB_CODE_INPUT_SCANNER	0x03
#define  PCI_CLASS_SUB_CODE_INPUT_GAMEPORT	0x04
#define  PCI_CLASS_SUB_CODE_INPUT_OTHER		0x80
#define  PCI_CLASS_SUB_CODE_DOCKING_GENERIC	0x00
#define  PCI_CLASS_SUB_CODE_DOCKING_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_PROCESSOR_386	0x00
#define  PCI_CLASS_SUB_CODE_PROCESSOR_486	0x01
#define  PCI_CLASS_SUB_CODE_PROCESSOR_PENTIUM	0x02
#define  PCI_CLASS_SUB_CODE_PROCESSOR_ALPHA	0x10
#define  PCI_CLASS_SUB_CODE_PROCESSOR_POWERPC	0x20
#define  PCI_CLASS_SUB_CODE_PROCESSOR_MIPS	0x30
#define  PCI_CLASS_SUB_CODE_PROCESSOR_COPROC	0x40
#define  PCI_CLASS_SUB_CODE_SERIAL_1394		0x00
#define  PCI_CLASS_SUB_CODE_SERIAL_ACCESSBUS	0x01
#define  PCI_CLASS_SUB_CODE_SERIAL_SSA		0x02
#define  PCI_CLASS_SUB_CODE_SERIAL_USB		0x03
#define  PCI_CLASS_SUB_CODE_SERIAL_FIBRECHAN	0x04
#define  PCI_CLASS_SUB_CODE_SERIAL_SMBUS	0x05
#define  PCI_CLASS_SUB_CODE_SERIAL_INFINIBAND	0x06
#define  PCI_CLASS_SUB_CODE_SERIAL_IPMI		0x07
#define  PCI_CLASS_SUB_CODE_SERIAL_SERCOS	0x08
#define  PCI_CLASS_SUB_CODE_SERIAL_CANBUS	0x09
#define  PCI_CLASS_SUB_CODE_WIRELESS_IRDA	0x00
#define  PCI_CLASS_SUB_CODE_WIRELESS_IR		0x01
#define  PCI_CLASS_SUB_CODE_WIRELESS_RF		0x10
#define  PCI_CLASS_SUB_CODE_WIRELESS_BLUETOOTH	0x11
#define  PCI_CLASS_SUB_CODE_WIRELESS_BROADBAND	0x12
#define  PCI_CLASS_SUB_CODE_WIRELESS_80211A	0x20
#define  PCI_CLASS_SUB_CODE_WIRELESS_80211B	0x21
#define  PCI_CLASS_SUB_CODE_WIRELESS_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_I2O_V1_0		0x00
#define  PCI_CLASS_SUB_CODE_SATELLITE_TV	0x01
#define  PCI_CLASS_SUB_CODE_SATELLITE_AUDIO	0x02
#define  PCI_CLASS_SUB_CODE_SATELLITE_VOICE	0x03
#define  PCI_CLASS_SUB_CODE_SATELLITE_DATA	0x04
#define  PCI_CLASS_SUB_CODE_CRYPTO_NETWORK	0x00
#define  PCI_CLASS_SUB_CODE_CRYPTO_ENTERTAINMENT 0x10
#define  PCI_CLASS_SUB_CODE_CRYPTO_OTHER	0x80
#define  PCI_CLASS_SUB_CODE_DATA_DPIO		0x00
#define  PCI_CLASS_SUB_CODE_DATA_PERFCNTR	0x01
#define  PCI_CLASS_SUB_CODE_DATA_COMMSYNC	0x10
#define  PCI_CLASS_SUB_CODE_DATA_MGMT		0x20
#define  PCI_CLASS_SUB_CODE_DATA_OTHER		0x80

#define PCI_CACHE_LINE_SIZE	0x0c	/* 8 bits */
#define PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#define PCI_HEADER_TYPE		0x0e	/* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL 0
#define  PCI_HEADER_TYPE_BRIDGE 1
#define  PCI_HEADER_TYPE_CARDBUS 2

#define PCI_BIST		0x0f	/* 8 bits */
#define PCI_BIST_CODE_MASK	0x0f	/* Return result */
#define PCI_BIST_START		0x40	/* 1 to start BIST, 2 secs or less */
#define PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0	0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1	0x14	/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2	0x18	/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3	0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4	0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5	0x24	/* 32 bits */
#define  PCI_BASE_ADDRESS_SPACE 0x01	/* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO 0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH	0x08	/* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK	(~0x0fULL)
#define  PCI_BASE_ADDRESS_IO_MASK	(~0x03ULL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#define PCI_CARDBUS_CIS		0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_SUBSYSTEM_ID	0x2e
#define PCI_ROM_ADDRESS		0x30	/* Bits 31..11 are address, 10..1 reserved */
#define  PCI_ROM_ADDRESS_ENABLE 0x01
#define PCI_ROM_ADDRESS_MASK	(~0x7ffULL)

#define PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE	0x3c	/* 8 bits */
#define PCI_INTERRUPT_PIN	0x3d	/* 8 bits */
#define PCI_MIN_GNT		0x3e	/* 8 bits */
#define PCI_MAX_LAT		0x3f	/* 8 bits */

#define PCI_INTERRUPT_LINE_DISABLE	0xff

/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#define PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#define PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#define PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#define PCI_IO_BASE		0x1c	/* I/O range behind the bridge */
#define PCI_IO_LIMIT		0x1d
#define  PCI_IO_RANGE_TYPE_MASK 0x0f	/* I/O bridging type */
#define  PCI_IO_RANGE_TYPE_16	0x00
#define  PCI_IO_RANGE_TYPE_32	0x01
#define  PCI_IO_RANGE_MASK	~0x0f
#define PCI_SEC_STATUS		0x1e	/* Secondary status register, only bit 14 used */
#define PCI_MEMORY_BASE		0x20	/* Memory range behind */
#define PCI_MEMORY_LIMIT	0x22
#define  PCI_MEMORY_RANGE_TYPE_MASK 0x0f
#define  PCI_MEMORY_RANGE_MASK	~0x0f
#define PCI_PREF_MEMORY_BASE	0x24	/* Prefetchable memory range behind */
#define PCI_PREF_MEMORY_LIMIT	0x26
#define  PCI_PREF_RANGE_TYPE_MASK 0x0f
#define  PCI_PREF_RANGE_TYPE_32 0x00
#define  PCI_PREF_RANGE_TYPE_64 0x01
#define  PCI_PREF_RANGE_MASK	~0x0f
#define PCI_PREF_BASE_UPPER32	0x28	/* Upper half of prefetchable memory range */
#define PCI_PREF_LIMIT_UPPER32	0x2c
#define PCI_IO_BASE_UPPER16	0x30	/* Upper half of I/O addresses */
#define PCI_IO_LIMIT_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#define PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_BRIDGE_CONTROL	0x3e
#define  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#define  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#define  PCI_BRIDGE_CTL_NO_ISA	0x04	/* Disable bridging of ISA ports */
#define  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#define  PCI_BRIDGE_CTL_MASTER_ABORT 0x20  /* Report master aborts */
#define  PCI_BRIDGE_CTL_BUS_RESET 0x40	/* Secondary bus reset */
#define  PCI_BRIDGE_CTL_FAST_BACK 0x80	/* Fast Back2Back enabled on secondary interface */

/* From 440ep */
#define PCI_ERREN       0x48     /* Error Enable */
#define PCI_ERRSTS      0x49     /* Error Status */
#define PCI_BRDGOPT1    0x4A     /* PCI Bridge Options 1 */
#define PCI_PLBSESR0    0x4C     /* PCI PLB Slave Error Syndrome 0 */
#define PCI_PLBSESR1    0x50     /* PCI PLB Slave Error Syndrome 1 */
#define PCI_PLBSEAR     0x54     /* PCI PLB Slave Error Address */
#define PCI_CAPID       0x58     /* Capability Identifier */
#define PCI_NEXTITEMPTR 0x59     /* Next Item Pointer */
#define PCI_PMC         0x5A     /* Power Management Capabilities */
#define PCI_PMCSR       0x5C     /* Power Management Control Status */
#define PCI_PMCSRBSE    0x5E     /* PMCSR PCI to PCI Bridge Support Extensions */
#define PCI_BRDGOPT2    0x60     /* PCI Bridge Options 2 */
#define PCI_PMSCRR      0x64     /* Power Management State Change Request Re. */

/* Header type 2 (CardBus bridges) */
#define PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#define PCI_CB_SEC_STATUS	0x16	/* Secondary status */
#define PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#define PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#define PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#define PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#define PCI_CB_MEMORY_BASE_0	0x1c
#define PCI_CB_MEMORY_LIMIT_0	0x20
#define PCI_CB_MEMORY_BASE_1	0x24
#define PCI_CB_MEMORY_LIMIT_1	0x28
#define PCI_CB_IO_BASE_0	0x2c
#define PCI_CB_IO_BASE_0_HI	0x2e
#define PCI_CB_IO_LIMIT_0	0x30
#define PCI_CB_IO_LIMIT_0_HI	0x32
#define PCI_CB_IO_BASE_1	0x34
#define PCI_CB_IO_BASE_1_HI	0x36
#define PCI_CB_IO_LIMIT_1	0x38
#define PCI_CB_IO_LIMIT_1_HI	0x3a
#define  PCI_CB_IO_RANGE_MASK	~0x03
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_CB_BRIDGE_CONTROL	0x3e
#define  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#define  PCI_CB_BRIDGE_CTL_SERR		0x02
#define  PCI_CB_BRIDGE_CTL_ISA		0x04
#define  PCI_CB_BRIDGE_CTL_VGA		0x08
#define  PCI_CB_BRIDGE_CTL_MASTER_ABORT 0x20
#define  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#define  PCI_CB_BRIDGE_CTL_16BIT_INT	0x80	/* Enable interrupt for 16-bit cards */
#define  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#define  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#define  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#define PCI_CB_SUBSYSTEM_VENDOR_ID 0x40
#define PCI_CB_SUBSYSTEM_ID	0x42
#define PCI_CB_LEGACY_MODE_BASE 0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#define PCI_CAP_LIST_ID		0	/* Capability ID */
#define  PCI_CAP_ID_PM		0x01	/* Power Management */
#define  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#define  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#define  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#define  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#define PCI_CAP_LIST_NEXT	1	/* Next capability in the list */
#define PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF		4

/* Power Management Registers */

#define  PCI_PM_CAP_VER_MASK	0x0007	/* Version */
#define  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#define  PCI_PM_CAP_AUX_POWER	0x0010	/* Auxilliary power support */
#define  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#define  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#define  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#define  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#define PCI_PM_CTRL		4	/* PM control and status register */
#define  PCI_PM_CTRL_STATE_MASK 0x0003	/* Current power state (D0 to D3) */
#define  PCI_PM_CTRL_PME_ENABLE 0x0100	/* PME pin enable */
#define  PCI_PM_CTRL_DATA_SEL_MASK	0x1e00	/* Data select (??) */
#define  PCI_PM_CTRL_DATA_SCALE_MASK	0x6000	/* Data scale (??) */
#define  PCI_PM_CTRL_PME_STATUS 0x8000	/* PME pin status */
#define PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#define  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#define  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#define PCI_PM_DATA_REGISTER	7	/* (??) */
#define PCI_PM_SIZEOF		8

/* AGP registers */

#define PCI_AGP_VERSION		2	/* BCD version number */
#define PCI_AGP_RFU		3	/* Rest of capability flags */
#define PCI_AGP_STATUS		4	/* Status register */
#define  PCI_AGP_STATUS_RQ_MASK 0xff000000	/* Maximum number of requests - 1 */
#define  PCI_AGP_STATUS_SBA	0x0200	/* Sideband addressing supported */
#define  PCI_AGP_STATUS_64BIT	0x0020	/* 64-bit addressing supported */
#define  PCI_AGP_STATUS_FW	0x0010	/* FW transfers supported */
#define  PCI_AGP_STATUS_RATE4	0x0004	/* 4x transfer rate supported */
#define  PCI_AGP_STATUS_RATE2	0x0002	/* 2x transfer rate supported */
#define  PCI_AGP_STATUS_RATE1	0x0001	/* 1x transfer rate supported */
#define PCI_AGP_COMMAND		8	/* Control register */
#define  PCI_AGP_COMMAND_RQ_MASK 0xff000000  /* Master: Maximum number of requests */
#define  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#define  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#define  PCI_AGP_COMMAND_64BIT	0x0020	/* Allow processing of 64-bit addresses */
#define  PCI_AGP_COMMAND_FW	0x0010	/* Force FW transfers */
#define  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#define  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 4x rate */
#define  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 4x rate */
#define PCI_AGP_SIZEOF		12

/* PCI-X registers */

#define  PCI_X_CMD_DPERR_E      0x0001  /* Data Parity Error Recovery Enable */
#define  PCI_X_CMD_ERO          0x0002  /* Enable Relaxed Ordering */
#define  PCI_X_CMD_MAX_READ     0x0000  /* Max Memory Read Byte Count */
#define  PCI_X_CMD_MAX_SPLIT    0x0030  /* Max Outstanding Split Transactions */
#define  PCI_X_CMD_VERSION(x)   (((x) >> 12) & 3) /* Version */


/* Slot Identification */

#define PCI_SID_ESR		2	/* Expansion Slot Register */
#define  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#define  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#define PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#define PCI_MSI_FLAGS		2	/* Various flags */
#define  PCI_MSI_FLAGS_64BIT	0x80	/* 64-bit addresses allowed */
#define  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#define  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#define  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#define PCI_MSI_RFU		3	/* Rest of capability flags */
#define PCI_MSI_ADDRESS_LO	4	/* Lower 32 bits */
#define PCI_MSI_ADDRESS_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#define PCI_MSI_DATA_32		8	/* 16 bits of data for 32-bit devices */
#define PCI_MSI_DATA_64		12	/* 16 bits of data for 64-bit devices */

#define PCI_MAX_PCI_DEVICES	32
#define PCI_MAX_PCI_FUNCTIONS	8

#define PCI_FIND_CAP_TTL 0x48
#define CAP_START_POS 0x40

/* Extended Capabilities (PCI-X 2.0 and Express) */
#define PCI_EXT_CAP_ID(header)		(header & 0x0000ffff)
#define PCI_EXT_CAP_VER(header)		((header >> 16) & 0xf)
#define PCI_EXT_CAP_NEXT(header)	((header >> 20) & 0xffc)

#define PCI_EXT_CAP_ID_ERR	0x01	/* Advanced Error Reporting */
#define PCI_EXT_CAP_ID_VC	0x02	/* Virtual Channel Capability */
#define PCI_EXT_CAP_ID_DSN	0x03	/* Device Serial Number */
#define PCI_EXT_CAP_ID_PWR	0x04	/* Power Budgeting */
#define PCI_EXT_CAP_ID_RCLD	0x05	/* Root Complex Link Declaration */
#define PCI_EXT_CAP_ID_RCILC	0x06	/* Root Complex Internal Link Control */
#define PCI_EXT_CAP_ID_RCEC	0x07	/* Root Complex Event Collector */
#define PCI_EXT_CAP_ID_MFVC	0x08	/* Multi-Function VC Capability */
#define PCI_EXT_CAP_ID_VC9	0x09	/* same as _VC */
#define PCI_EXT_CAP_ID_RCRB	0x0A	/* Root Complex RB? */
#define PCI_EXT_CAP_ID_VNDR	0x0B	/* Vendor-Specific */
#define PCI_EXT_CAP_ID_CAC	0x0C	/* Config Access - obsolete */
#define PCI_EXT_CAP_ID_ACS	0x0D	/* Access Control Services */
#define PCI_EXT_CAP_ID_ARI	0x0E	/* Alternate Routing ID */
#define PCI_EXT_CAP_ID_ATS	0x0F	/* Address Translation Services */
#define PCI_EXT_CAP_ID_SRIOV	0x10	/* Single Root I/O Virtualization */
#define PCI_EXT_CAP_ID_MRIOV	0x11	/* Multi Root I/O Virtualization */
#define PCI_EXT_CAP_ID_MCAST	0x12	/* Multicast */
#define PCI_EXT_CAP_ID_PRI	0x13	/* Page Request Interface */
#define PCI_EXT_CAP_ID_AMD_XXX	0x14	/* Reserved for AMD */
#define PCI_EXT_CAP_ID_REBAR	0x15	/* Resizable BAR */
#define PCI_EXT_CAP_ID_DPA	0x16	/* Dynamic Power Allocation */
#define PCI_EXT_CAP_ID_TPH	0x17	/* TPH Requester */
#define PCI_EXT_CAP_ID_LTR	0x18	/* Latency Tolerance Reporting */
#define PCI_EXT_CAP_ID_SECPCI	0x19	/* Secondary PCIe Capability */
#define PCI_EXT_CAP_ID_PMUX	0x1A	/* Protocol Multiplexing */
#define PCI_EXT_CAP_ID_PASID	0x1B	/* Process Address Space ID */

#endif /* __PCI_COMMANDS_H__ */
