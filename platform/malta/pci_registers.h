
/************************************************************************
 *
 *  PCI definitions
 *
 * ######################################################################
 *
 * mips_start_of_header
 * 
 *  $Id: pci.h,v 1.22 2002/06/19 21:07:15 eggerts Exp $
 * 
 * Copyright (c) [Year(s)] MIPS Technologies, Inc. All rights reserved.
 *
 * Unpublished rights reserved under U.S. copyright law.
 *
 * PROPRIETARY/SECRET CONFIDENTIAL INFORMATION OF MIPS TECHNOLOGIES,
 * INC. FOR INTERNAL USE ONLY.
 *
 * Under no circumstances (contract or otherwise) may this information be
 * disclosed to, or copied, modified or used by anyone other than employees
 * or contractors of MIPS Technologies having a need to know.
 *
 * 
 * mips_end_of_header
 *
 ************************************************************************/


#ifndef __PCI_REGS_H__
#define __PCI_REGS_H__

/************************************************************************
 *  PCI definitions
 ************************************************************************/

#define PCI_MAX_FUNC			8	/* Max functions 	*/
#define PCI_MAX_BUS			256	/* Max busses		*/
#define PCI_MAX_DEV			64	/* Max devices    	*/

/* Alignment requirements of memory/IO ranges set by PCI-PCI bridges	*/
#define PCI_ALIGN_IO		       (1 << 12) /* IO range		*/
#define PCI_ALIGN_MEM		       (1 << 20) /* Memory range	*/

/* Default latency timer */
#define PCI_LATTIM_FIXED        0x20

/************************************************************************
 *  Register numbers (derived by offset addresses) and access types
 ************************************************************************/

#define PCI_HEADERTYPE0			0
#define PCI_HEADERTYPE1			1
#define PCI_HEADERTYPE_MAX		1  /* No support for cardbus */

/* Common header (32 bit registers) */
#define PCI_ID				0x00
#define PCI_SC				0x04
#define PCI_CCREV			0x08
#define PCI_BHLC			0x0c

/* Common header (8 bit registers) */
#define PCI_LATTIM			0x0d

/* Header 0 and 1 (32 bit registers) */
#define PCI_BAR_MIN			0x10
#define PCI_BAR(number)			(PCI_BAR_MIN + (number)*4)

/* Header 0 and 1 (8 bit registers) */
#define PCI_CAP_PTR			0x34
#define PCI_INTLINE			0x3c

/* Header 0 (32 bit registers) */
#define PCI_BAR_MAX			0x24
#define PCI_MMII			0x3c

/* Header 1 (32 bit registers) */
#define PCI_BAR_MAX_PPB			0x14
#define PCI_SSSP			0x18
#define PCI_IO				0x1c
#define PCI_MEM				0x20
#define PCI_PREFMEM			0x24
#define PCI_UPPERIO			0x30
#define PCI_BCII			0x3c

/* Header 1 (16 bit registers) */
#define PCI_BC				0x3e

/* First addr. not belonging to standard header */
#define PCI_FIRST_NON_STANDARD		0x40

/************************************************************************
 *  Register encodings
 ************************************************************************/

/* SC */
#define PCI_SC_CMD_IOS_SHF		0
#define PCI_SC_CMD_IOS_MSK		(MSK(1) << PCI_SC_CMD_IOS_SHF)
#define PCI_SC_CMD_IOS_BIT		PCI_SC_CMD_IOS_MSK

#define PCI_SC_CMD_MS_SHF		1
#define PCI_SC_CMD_MS_MSK		(MSK(1) << PCI_SC_CMD_MS_SHF)
#define PCI_SC_CMD_MS_BIT		PCI_SC_CMD_MS_MSK

#define PCI_SC_CMD_BM_SHF		2
#define PCI_SC_CMD_BM_MSK		(MSK(1) << PCI_SC_CMD_BM_SHF)
#define PCI_SC_CMD_BM_BIT		PCI_SC_CMD_BM_MSK

#define PCI_SC_CMD_PERR_SHF		6
#define PCI_SC_CMD_PERR_MSK		(MSK(1) << PCI_SC_CMD_PERR_SHF)
#define PCI_SC_CMD_PERR_BIT		PCI_SC_CMD_PERR_MSK

#define PCI_SC_CMD_SERR_SHF		8
#define PCI_SC_CMD_SERR_MSK		(MSK(1) << PCI_SC_CMD_SERR_SHF)
#define PCI_SC_CMD_SERR_BIT		PCI_SC_CMD_SERR_MSK

#define PCI_SC_CMD_FBB_SHF		9
#define PCI_SC_CMD_FBB_MSK		(MSK(1) << PCI_SC_CMD_FBB_SHF)
#define PCI_SC_CMD_FBB_BIT		PCI_SC_CMD_FBB_MSK

/* STATUS */
#define PCI_STATUS_FBB_SHF		7
#define PCI_STATUS_FBB_MSK		(MSK(1) << PCI_STATUS_FBB_SHF)
#define PCI_STATUS_FBB_BIT		PCI_STATUS_FBB_MSK

#define PCI_STATUS_CAP_SHF		4
#define PCI_STATUS_CAP_MSK		(MSK(1) << PCI_STATUS_CAP_SHF)
#define PCI_STATUS_CAP_BIT		PCI_STATUS_CAP_MSK

/* BAR */
#define PCI_BAR_IO_SHF			0
#define PCI_BAR_IO_MSK			(MSK(1) << PCI_BAR_IO_SHF)
#define PCI_BAR_IO_BIT			PCI_BAR_IO_MSK

#define PCI_BAR_TYPE_SHF		1
#define PCI_BAR_TYPE_MSK		(MSK(2) << PCI_BAR_TYPE_SHF)
#define PCI_BAR_TYPE_32			0
#define PCI_BAR_TYPE_32_1M		1
#define PCI_BAR_TYPE_64			2
#define PCI_BAR_TYPE_RSVD		3

#define PCI_BAR_PREFETCH_SHF		3
#define PCI_BAR_PREFETCH_MSK		(MSK(1) << PCI_BAR_PREFETCH_SHF)

#define PCI_BAR_IOSIZE_SHF		2
#define PCI_BAR_IOSIZE_MSK		(MSK(30) << PCI_BAR_IOSIZE_SHF)

#define PCI_BAR_MEMSIZE_SHF		4
#define PCI_BAR_MEMSIZE_MSK		(MSK(28) << PCI_BAR_MEMSIZE_SHF)

/* ID */
#define PCI_ID_DEVID_SHF		16
#define PCI_ID_DEVID_MSK		(MSK(16) << PCI_ID_DEVID_SHF)

#define PCI_ID_VENDORID_SHF		0
#define PCI_ID_VENDORID_MSK		(MSK(16) << PCI_ID_VENDORID_SHF)

/* SC */
#define PCI_SC_STATUS_SHF		16
#define PCI_SC_STATUS_MSK		(MSK(16) << PCI_SC_STATUS_SHF)

#define PCI_SC_COMMAND_SHF		0
#define PCI_SC_COMMAND_MSK		(MSK(16) << PCI_SC_COMMAND_SHF)

/* Class Code and Revision ID (CCREV) */
#define PCI_CCREV_CC_SHF	        8	
#define PCI_CCREV_CC_MSK		(MSK(24) << PCI_CCREV_CC_SHF)

#define PCI_CCREV_REVID_SHF		0
#define PCI_CCREV_REVID_MSK		(MSK(8) << PCI_CCREV_REVID_SHF)

/* BIST, Header Type, Lat timer, Cache line size (BHLC) */
#define PCI_BHLC_BIST_SHF	       24
#define PCI_BHLC_BIST_MSK	       (MSK(8) << PCI_BHLC_BIST_SHF)
#define PCI_BHLC_HT_SHF		       16
#define PCI_BHLC_HT_MSK		       (MSK(7) << PCI_BHLC_HT_SHF)
#define PCI_BHLC_MULTI_SHF	       23
#define PCI_BHLC_MULTI_MSK	       (MSK(1) << PCI_BHLC_MULTI_SHF)
#define PCI_BHLC_MULTI_BIT	       PCI_BHLC_MULTI_MSK
#define PCI_BHLC_LT_SHF		       8
#define PCI_BHLC_LT_MSK		       (MSK(8) << PCI_BHLC_LT_SHF)
#define PCI_BHLC_CLS_SHF	       0
#define PCI_BHLC_CLS_MSK	       (MSK(8) << PCI_BHLC_CLS_SHF)

/* Max Lat, Min Gnt, Int pin, Int line (MMII) */
#define PCI_MMII_MAXLAT_SHF	       24
#define PCI_MMII_MAXLAT_MSK	       (MSK(8) << PCI_MMII_MAXLAT_SHF)
#define PCI_MMII_MINGNT_SHF	       16
#define PCI_MMII_MINGNT_MSK	       (MSK(8) << PCI_MMII_MINGNT_SHF)
#define PCI_MMII_INTPIN_SHF	       8
#define PCI_MMII_INTPIN_MSK	       (MSK(8) << PCI_MMII_INTPIN_SHF)

#define PCI_MMII_INTPIN_NU	       0
#define PCI_MMII_INTPIN_A	       1
#define PCI_MMII_INTPIN_B	       2
#define PCI_MMII_INTPIN_C	       3
#define PCI_MMII_INTPIN_D	       4

#define PCI_MMII_INTLINE_SHF           0
#define PCI_MMII_INTLINE_MSK	       (MSK(8) << PCI_MMII_INTLINE_SHF)
#define PCI_MMII_INTLINE_NONE	       0xFF

/* Extended capabilities */
#define PCI_EXT_CAP_ID_OFS			0
#define PCI_EXT_CAP_PWR_MGMT_STATUS_OFS		4
#define PCI_EXT_CAP_NEXT_PTR_OFS		1

#define PCI_EXT_CAP_ID_PWR_MGMT			0x01
#define PCI_EXT_CAP_PWR_MGMT_STATUS_D0		0x8000

/* Sec lat, sub bus num, sec bus number, prim bus number (SSSP) */
#define PCI_SSSP_SLT_SHF	       24
#define PCI_SSSP_SLT_MSK	       (MSK(8) << PCI_SSSP_SLT_SHF)
#define PCI_SSSP_SUBBN_SHF	       16
#define PCI_SSSP_SUBBN_MSK	       (MSK(8) << PCI_SSSP_SUBBN_SHF)
#define PCI_SSSP_SECBN_SHF             8
#define PCI_SSSP_SECBN_MSK	       (MSK(8) << PCI_SSSP_SECBN_SHF)
#define PCI_SSSP_PBN_SHF	       0
#define PCI_SSSP_PBN_MSK	       (MSK(8) << PCI_SSSP_PBN_SHF)

/* Bridge control, int pin, int line (BCII) */
#define PCI_BCII_BC_SHF		       16
#define PCI_BCII_BC_MSK		       (MSK(16) << PCI_BCII_BC_SHF)

#define PCI_BCII_BC_PERR_SHF	       (16 + 0)
#define PCI_BCII_BC_PERR_MSK	       (MSK(1) << PCI_BCII_BC_PERR_SHF)
#define PCI_BCII_BC_PERR_BIT	       PCI_BCII_BC_PERR_MSK

#define PCI_BCII_BC_SERR_SHF	       (16 + 1)
#define PCI_BCII_BC_SERR_MSK	       (MSK(1) << PCI_BCII_BC_SERR_SHF)
#define PCI_BCII_BC_SERR_BIT	       PCI_BCII_BC_SERR_MSK

#define PCI_BCII_BC_MA_SHF	       (16 + 5)
#define PCI_BCII_BC_MA_MSK	       (MSK(1) << PCI_BCII_BC_MA_SHF)
#define PCI_BCII_BC_MA_BIT	       PCI_BCII_BC_MA_MSK

#define PCI_BCII_BC_FBB_SHF	       (16 + 7)
#define PCI_BCII_BC_FBB_MSK	       (MSK(1) << PCI_BCII_BC_FBB_SHF)
#define PCI_BCII_BC_FBB_BIT	       PCI_BCII_BC_FBB_MSK

/* IO */
#define PCI_IO_BASE_SHF			0
#define PCI_IO_BASE_MSK			(MSK(8) << PCI_IO_BASE_SHF)
#define PCI_IO_LIMIT_SHF		8
#define PCI_IO_LIMIT_MSK		(MSK(8) << PCI_IO_LIMIT_SHF)

/* UpperIO */
#define PCI_UPPERIO_BASE_SHF		0
#define PCI_UPPERIO_BASE_MSK		(MSK(16) << PCI_UPPERIO_BASE_SHF)
#define PCI_UPPERIO_LIMIT_SHF		16
#define PCI_UPPERIO_LIMIT_MSK		(MSK(16) << PCI_UPPERIO_LIMIT_SHF)

/* Mem */
#define PCI_MEM_BASE_SHF		0
#define PCI_MEM_BASE_MSK		(MSK(16) << PCI_MEM_BASE_SHF)
#define PCI_MEM_LIMIT_SHF		16
#define PCI_MEM_LIMIT_MSK		(MSK(16) << PCI_MEM_LIMIT_SHF)

/* PrefMem */
#define PCI_PREFMEM_BASE_SHF		0
#define PCI_PREFMEM_BASE_MSK		(MSK(16) << PCI_PREFMEM_BASE_SHF)
#define PCI_PREFMEM_LIMIT_SHF		16
#define PCI_PREFMEM_LIMIT_MSK		(MSK(16) << PCI_PREFMEM_LIMIT_SHF)



/************************************************************************
 *  PCI configuration cycle AD bus definition
 ************************************************************************/

/* Type 0 */
#define PCI_CFG_TYPE0_REG_SHF		0
#define PCI_CFG_TYPE0_REG_MSK		(MSK(6) << 2)

#define PCI_CFG_TYPE0_FUNC_SHF		8
#define PCI_CFG_TYPE0_FUNC_MSK		(MSK(3) << PCI_CFG_TYPE0_FUNC_SHF)

/* Type 1 */
#define PCI_CFG_TYPE1_REG_SHF		0
#define PCI_CFG_TYPE1_REG_MSK		(MSK(6) << 2)

#define PCI_CFG_TYPE1_FUNC_SHF		8
#define PCI_CFG_TYPE1_FUNC_MSK		(MSK(3) << PCI_CFG_TYPE0_FUNC_SHF)

#define PCI_CFG_TYPE1_DEV_SHF		11
#define PCI_CFG_TYPE1_DEV_MSK		(MSK(5) << PCI_CFG_TYPE1_DEV_SHF)

#define PCI_CFG_TYPE1_BUS_SHF		16
#define PCI_CFG_TYPE1_BUS_MSK		(MSK(8) << PCI_CFG_TYPE1_BUS_SHF)

#endif /* #ifndef __PCI_REGS_H__ */



