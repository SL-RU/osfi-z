
MEMORY
{
   PLUGIN_RAM : ORIGIN = 0x0000000020019f30, LENGTH = 100K
}

SECTIONS
{
    .header : {
        _plugin_start_addr = .;
        plugin_start_addr = .;
        KEEP(*(.header))
    } > PLUGIN_RAM

    .text :
    {
        *(.text)           
        *(.text*)
        *(.glue_7)
        *(.glue_7t)

    } > PLUGIN_RAM

    .rodata :
    {
        *(.rodata*)
    } > PLUGIN_RAM

    .data :
    {
        *(.data*)
    } > PLUGIN_RAM
    
    bssaddr = .;

    /DISCARD/ :
    {
        *(.eh_frame)
    }

    .bss bssaddr (NOLOAD) :
    {
    	plugin_bss_start = .;
    	_plugin_bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(0x4);
    } > PLUGIN_RAM
    
    pluginendaddr = .;

    /* Final end of plugin after IRAM setup. The plugin or codec buffer
       is considered unused by the in-RAM image at this point once IRAM
       is copied. */
    .pluginend pluginendaddr :
    {
        _plugin_end_addr = .;
        plugin_end_addr = .;
    }

    loadendaddr = plugin_end_addr;

    /* This is for ovl_offset.pl and is the highest address that must
       be loaded into the plugin buffer (past the end of last data in
       stored image). */
    .pluginloadend loadendaddr :
    {
    	_plugin_load_end_addr = .;
    	plugin_load_end_addr = .;
    }

    /* Special trick to avoid a linker error when no other sections are
       left after garbage collection (plugin not for this platform) */
    .comment 0 :
    {
        KEEP(*(.comment))
    }
}
