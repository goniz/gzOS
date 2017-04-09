/* Linker script for OVPsim SmartLoaderLinux firmware. */
OUTPUT_ARCH(mips)
ENTRY(_start)
PHDRS
{
  text PT_LOAD FLAGS(5); /* read-only, executable */
  data PT_LOAD FLAGS(6); /* read-write */
}
PROVIDE(__tlb_stride_length = 1);
SECTIONS
{
  /* RAM is mapped in kseg0 (cacheable) and kseg1 (non-cacheable) */
  _ram = 0x80000000;

  .text 0x80100000:
  {
    _stext = ABSOLUTE(.);
    /* Exception handlers. */
    _ebase = ABSOLUTE(.);
    KEEP (*(.ebase))
    . = ALIGN(4096);
    /* Text and read-only data follows. */
    __text = ABSOLUTE(.);
    *(.text .text.*)
    __etext = ABSOLUTE(.);
    . = ALIGN(4096);
  } : text

  .rodata ADDR(.text) + SIZEOF(.text):
  {
    /* Read only data */
    __rodata = ABSOLUTE(.);
    . = ALIGN (4);
    *(.rodata .rodata.*)
    KEEP(*(.strings))
	*(.gcc_except_table .gcc_except_table.*)
	. = ALIGN(4);
	PROVIDE(__syscalls_start = .);
	PROVIDE(syscall_table = .);
	KEEP(*(.syscalls))
	PROVIDE(__syscalls_end = .);

	. = ALIGN(4);
    PROVIDE(__pci_drv_start = .);
    PROVIDE(pci_drivers_table = .);
    KEEP(*(.pci_drivers))
    PROVIDE(__pci_drv_end = .);

    . = ALIGN(4);
    PROVIDE(__drivers_start = .);
    PROVIDE(drivers_table = .);
    KEEP (*(SORT(.drivers.*)))
    KEEP (*(SORT(.drivers)))
    PROVIDE(__drivers_end = .);

    PROVIDE(__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    PROVIDE(__init_array_end = .);

    __erodata = ABSOLUTE(.);
    . = ALIGN (4);
  } : text

  .data ADDR(.rodata) + SIZEOF (.rodata):
  {
    __data = ABSOLUTE(.);
    _gp = .;
    *(.data .data.*)
    *(.sdata .sdata.*)
    *(.eh_frame .eh_frame.*)
    . = ALIGN(4);
    __edata = ABSOLUTE(.);
  } : data

  .bss ADDR (.data) + SIZEOF (.data) (NOLOAD) :
  {
    __bss = ABSOLUTE(.);
    *(.sbss .sbss.* .scommon)
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN (64k);
    __ebss = ABSOLUTE(.);
  } : data

  _end = .;

  /* Sections to be discarded */
  /DISCARD/ :
  {
    *(.comment)
    *(.note)
    *(.options)
    *(.pdr)
    *(.reginfo)
    *(.gnu.attributes)
    *(.MIPS.abiflags)
    *(.MIPS.options)
	*(.rel.dyn)
  }
}
