OUTPUT_ARCH(mips)
ENTRY(_start)
PHDRS
{
  text PT_LOAD FLAGS(5); /* read-only, executable */
  data PT_LOAD FLAGS(6); /* read-write */
}
SECTIONS
{
  .text 0x401000:
  {
    . = ALIGN(4096);
    /* Text and read-only data follows. */
    __text = ABSOLUTE(.);
    *(.text .text.*)
    . = ALIGN(4096);
    __etext = ABSOLUTE(.);
  } : text

  .rodata ADDR(.text) + SIZEOF(.text):
  {
    . = ALIGN(4096);
    /* Read only data */
    __rodata = ABSOLUTE(.);
    . = ALIGN (4);
    *(.rodata .rodata.*)
    KEEP(*(.strings))
	*(.gcc_except_table .gcc_except_table.*)
	. = ALIGN(4);

    PROVIDE(__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    PROVIDE(__init_array_end = .);

    . = ALIGN(4096);
    __erodata = ABSOLUTE(.);
  } : text

  .data ADDR(.rodata) + SIZEOF (.rodata):
  {
    . = ALIGN(4096);
    __data = ABSOLUTE(.);
    _gp = .;
    *(.data .data.*)
    *(.sdata .sdata.*)
    *(.eh_frame .eh_frame.*)
    . = ALIGN(4096);
    __edata = ABSOLUTE(.);
  } : data

  .bss ADDR (.data) + SIZEOF (.data) (NOLOAD) :
  {
    . = ALIGN(4096);
    __bss = ABSOLUTE(.);
    *(.sbss .sbss.* .scommon)
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4096);
    __ebss = ABSOLUTE(.);
  }

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
