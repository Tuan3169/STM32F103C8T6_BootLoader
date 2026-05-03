.syntax unified
.cpu cortex-m3
.thumb

/* Stack and Heap Configuration */
.equ Stack_Size, 0x400
.equ Heap_Size, 0x200

.global _estack
.global __initial_sp
.global Reset_Handler
.global __Vectors
.global __Vectors_End
.global __Vectors_Size

/* External symbols */
.extern ApplicationOTAEntryPoint

/* ===========================================================================
   Stack Section
   =========================================================================== */
.section .stack
    .align 3
    .globl __stack_base
__stack_base:
    .space Stack_Size
_estack:
__initial_sp:

/* ===========================================================================
   Heap Section
   =========================================================================== */
.section .heap
    .align 3
    .globl __heap_base
    .globl __heap_limit
__heap_base:
    .space Heap_Size
__heap_limit:

/* ===========================================================================
   Vector Table
   =========================================================================== */
.section .isr_vector, "a", %progbits
    .align 2
    .globl __Vectors
__Vectors:
    .word _estack                 /* 0x00: Initial Stack Pointer */
    .word Reset_Handler           /* 0x04: Reset Handler */

    /* Cortex-M3 System Exceptions */
    .word NMI_Handler            /* 0x08: NMI Handler */
    .word HardFault_Handler      /* 0x0C: Hard Fault Handler */
    .word MemManage_Handler      /* 0x10: MPU Fault Handler */
    .word BusFault_Handler       /* 0x14: Bus Fault Handler */
    .word UsageFault_Handler     /* 0x18: Usage Fault Handler */
    .word 0                      /* 0x1C: Reserved */
    .word 0                      /* 0x20: Reserved */
    .word 0                      /* 0x24: Reserved */
    .word 0                      /* 0x28: Reserved */
    .word SVC_Handler            /* 0x2C: SVCall Handler */
    .word DebugMon_Handler       /* 0x30: Debug Monitor Handler */
    .word 0                      /* 0x34: Reserved */
    .word PendSV_Handler         /* 0x38: PendSV Handler */
    .word SysTick_Handler        /* 0x3C: SysTick Handler */

    /* External Interrupts */
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler
    .word DMA1_Channel5_IRQHandler
    .word DMA1_Channel6_IRQHandler
    .word DMA1_Channel7_IRQHandler
    .word ADC1_2_IRQHandler
    .word USB_HP_CAN1_TX_IRQHandler
    .word USB_LP_CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_IRQHandler
    .word TIM1_UP_IRQHandler
    .word TIM1_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM4_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler
    .word EXTI15_10_IRQHandler
    .word RTC_Alarm_IRQHandler
    .word USBWakeUp_IRQHandler

__Vectors_End:
    .equ __Vectors_Size, __Vectors_End - __Vectors

/* ===========================================================================
   Reset Handler
   =========================================================================== */
.section .text.Reset_Handler, "ax", %progbits
    .align 2
    .thumb_func
    .globl Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* Copy .data section from Flash to RAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata

    cmp r0, r1
    beq init_bss

copy_data_loop:
    ldr r3, [r2], #4
    str r3, [r0], #4
    cmp r0, r1
    blt copy_data_loop

    /* Initialize .bss section to zero */
init_bss:
    ldr r0, =_sbss
    ldr r1, =_ebss
    mov r2, #0

    cmp r0, r1
    beq call_entry

zero_bss_loop:
    str r2, [r0], #4
    cmp r0, r1
    blt zero_bss_loop

    /* Call application entry point */
call_entry:
    bl ApplicationOTAEntryPoint

    /* Infinite loop (should not reach here) */
infinite_loop:
    b infinite_loop

.size Reset_Handler, . - Reset_Handler

/* ===========================================================================
   Exception Handlers (Weak definitions)
   =========================================================================== */
.section .text.Default_Handler, "ax", %progbits
Default_Handler:
    b Default_Handler

.thumb_func
NMI_Handler:
    b Default_Handler

.thumb_func
HardFault_Handler:
    b Default_Handler

.thumb_func
MemManage_Handler:
    b Default_Handler

.thumb_func
BusFault_Handler:
    b Default_Handler

.thumb_func
UsageFault_Handler:
    b Default_Handler

.thumb_func
SVC_Handler:
    b Default_Handler

.thumb_func
DebugMon_Handler:
    b Default_Handler

.thumb_func
PendSV_Handler:
    b Default_Handler

.thumb_func
SysTick_Handler:
    b Default_Handler

.thumb_func
WWDG_IRQHandler:
    b Default_Handler

.thumb_func
PVD_IRQHandler:
    b Default_Handler

.thumb_func
TAMPER_IRQHandler:
    b Default_Handler

.thumb_func
RTC_IRQHandler:
    b Default_Handler

.thumb_func
FLASH_IRQHandler:
    b Default_Handler

.thumb_func
RCC_IRQHandler:
    b Default_Handler

.thumb_func
EXTI0_IRQHandler:
    b Default_Handler

.thumb_func
EXTI1_IRQHandler:
    b Default_Handler

.thumb_func
EXTI2_IRQHandler:
    b Default_Handler

.thumb_func
EXTI3_IRQHandler:
    b Default_Handler

.thumb_func
EXTI4_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel1_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel2_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel3_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel4_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel5_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel6_IRQHandler:
    b Default_Handler

.thumb_func
DMA1_Channel7_IRQHandler:
    b Default_Handler

.thumb_func
ADC1_2_IRQHandler:
    b Default_Handler

.thumb_func
USB_HP_CAN1_TX_IRQHandler:
    b Default_Handler

.thumb_func
USB_LP_CAN1_RX0_IRQHandler:
    b Default_Handler

.thumb_func
CAN1_RX1_IRQHandler:
    b Default_Handler

.thumb_func
CAN1_SCE_IRQHandler:
    b Default_Handler

.thumb_func
EXTI9_5_IRQHandler:
    b Default_Handler

.thumb_func
TIM1_BRK_IRQHandler:
    b Default_Handler

.thumb_func
TIM1_UP_IRQHandler:
    b Default_Handler

.thumb_func
TIM1_TRG_COM_IRQHandler:
    b Default_Handler

.thumb_func
TIM1_CC_IRQHandler:
    b Default_Handler

.thumb_func
TIM2_IRQHandler:
    b Default_Handler

.thumb_func
TIM3_IRQHandler:
    b Default_Handler

.thumb_func
TIM4_IRQHandler:
    b Default_Handler

.thumb_func
I2C1_EV_IRQHandler:
    b Default_Handler

.thumb_func
I2C1_ER_IRQHandler:
    b Default_Handler

.thumb_func
I2C2_EV_IRQHandler:
    b Default_Handler

.thumb_func
I2C2_ER_IRQHandler:
    b Default_Handler

.thumb_func
SPI1_IRQHandler:
    b Default_Handler

.thumb_func
SPI2_IRQHandler:
    b Default_Handler

.thumb_func
USART1_IRQHandler:
    b Default_Handler

.thumb_func
USART2_IRQHandler:
    b Default_Handler

.thumb_func
USART3_IRQHandler:
    b Default_Handler

.thumb_func
EXTI15_10_IRQHandler:
    b Default_Handler

.thumb_func
RTC_Alarm_IRQHandler:
    b Default_Handler

.thumb_func
USBWakeUp_IRQHandler:
    b Default_Handler

.end
