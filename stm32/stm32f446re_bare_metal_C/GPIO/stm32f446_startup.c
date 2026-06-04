#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
void Reset_Handler(void);
int main(void);
void NMI_Handler(void)__attribute__((weak,alias("Default_Handler")));
void HardFault_Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void MemManage_Handler (void) __attribute__ ((weak, alias("Default_Handler")));
uint32_t vector_tbl[] __attribute__((section(".isr_vector_tbl"))) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    (uint32_t)&NMI_Handler,
    (uint32_t)&HardFault_Handler,
    (uint32_t)&MemManage_Handler,
};
void Default_Handler(void) {
    while(1) {
    }
}
void Reset_Handler(void)
{
    // Calculate the sizes of the .data and .bss sections
    uint32_t data_mem_size =  (uint32_t)&_edata - (uint32_t)&_sdata;
    uint32_t bss_mem_size  =   (uint32_t)&_ebss - (uint32_t)&_sbss;
    // Initialize pointers to the source and destination of the .data 
    // section
 uint32_t *p_src_mem =  (uint32_t *)&_etext;
    uint32_t *p_dest_mem = (uint32_t *)&_sdata;
    /*Copy .data section from FLASH to SRAM*/
    for(uint32_t i = 0; i < data_mem_size; i++  )
    {
         *p_dest_mem++ = *p_src_mem++;
    }
    // Initialize the .bss section to zero in SRAM
    p_dest_mem =  (uint32_t *)&_sbss;
    for(uint32_t i = 0; i < bss_mem_size; i++)
    {
         /*Set bss section to zero*/
        *p_dest_mem++ = 0;
    }
        // Call the application's main function.
    main();
}
