ENTRY(Reset_Handler);

MEMORY 
{
FLASH (rx) :ORIGIN = 0x08000000, LENGTH = 64K
RAM (xrw) :ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS {
    .isr_vector : {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text : {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7) /* Mã hỗ trợ khi gọi từ code ARM sang code Thumb */
        *(.glue_7t) /* Mã hỗ trợ khi gọi từ code Thumb sang code ARM */
        *(.eh_frame)

        KEEP (*(.init)) /* Giữ lại các mã thực thì như biến hàm đã khai báo trước hàm main nhưng không được dùng đến */
        KEEP (*(.fini)) /* Giữ lại các mã thực thì như biến hàm đã khai báo sau hàm main nhưng không được dùng đến */
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .rodata : {
        . = ALIGN(4);
        *(.rodata) /* Lấy tất cả các mục .rodata */
        *(.rodata*) /* Lấy các mục bắt đầu bằng .rodata */
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx : {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        . = ALIGN(4);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data : {
        . = ALIGN(4);
        _sdata = .; /* bắt đầu vùng .data trong RAM */
        *(.data) /* Lấy các biến toàn cục đã khởi tạo giá trị */
        *(.data*)
        . = ALIGN(4);
        _edata = .; /* Kết thúc vùng .data trong RAM */
    } > RAM AT > FLASH /* VMA ở trong RAM nhưng LMA nằm tại FLASH, nên phải load từ FLASH vào RAM */

    .bss : {
        . = ALIGN(4);
        _sbss = .; /* bắt đầu vùng .bss */
        __bss_start__ = _sbss; 
        *(.bss) 
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .; /* kết thúc vùng .bss */
        __bss_end__ = _ebss;
    } > RAM
    
    ._user_heap_stack : {
        . = ALIGN(8);
        PROVIDE ( end = .);
        PROVIDE ( _end = .);
        . = . + 0x200; /* cấp 512 bytes cho Heap */
        . = . + 0x400; /* cấp 1024 bytes cho Heap */
        . = ALIGN(8);
    } > RAM
}