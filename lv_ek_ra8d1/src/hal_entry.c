#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

#define REGION_0_ATTR_IDX 0U
#define REGION_1_ATTR_IDX 1U
#define REGION_2_ATTR_IDX 2U
#define READ_WRITE 0U
#define READ_ONLY 1U
#define PRIVILEGED_ONLY 0U
#define ANY_PRVILEGE 1U
#define EXECUTION_PERMITTED 0U
#define NO_EXECUTION 1U

#define NON_TRANSIENT 1U
#define TRANSIENT     0U

#define WRITE_BACK    1U
#define WRITE_THROUGH 0U

#define READ_ALLOCATE  1U
#define WRITE_ALLOCATE 1U

#if 0
#define PARTIAL_MODE_VSIZE         (100)

extern     uint8_t buf_1_1[DISPLAY_HSIZE_INPUT0 * PARTIAL_MODE_VSIZE * 2]BSP_ALIGN_VARIABLE(64) BSP_PLACE_IN_SECTION(".sdram");
#endif

/*******************************************************************************************************************//**
 * main() is generated by the RA Configuration editor and is used to generate threads if an RTOS is used.  This function
 * is called by main() when no RTOS is used.
 **********************************************************************************************************************/
void hal_entry(void)
{
    /* TODO: add your own code here */

#if BSP_TZ_SECURE_BUILD
    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open (&g_ioport_ctrl, &IOPORT_CFG_NAME);

        /* Enable individual fault handlers */
        SCB->SHCSR |= (SCB_SHCSR_USGFAULTENA_Msk | \
                   SCB_SHCSR_BUSFAULTENA_Msk | \
                   SCB_SHCSR_MEMFAULTENA_Msk |
                   SCB_SHCSR_SECUREFAULTENA_Msk);

        bsp_sdram_init(); //SDRAM pins need to be set to HIGH drive strength in pin configuration

        /* Mark the framebuffer as Write-Through Cacheable */
#if defined(RENESAS_CORTEX_M85)
#if (0) //(BSP_CFG_DCACHE_ENABLED)

        ARM_MPU_Disable();

        SCB_DisableDCache();

        ARM_MPU_Region_t mpuTable[]=
        {
#if 0
            // BASE SH RO NP XN LIMIT ATTR
            { .RBAR = ARM_MPU_RBAR((uint32_t)&fb_background[0][0], ARM_MPU_SH_NON, READ_WRITE, PRIVILEGED_ONLY, EXECUTION_PERMITTED),
              .RLAR = ARM_MPU_RLAR((uint32_t)(&fb_background[1][0] + sizeof(fb_background) - 1), REGION_2_ATTR_IDX)
            }
#else
            { .RBAR = ARM_MPU_RBAR((uint32_t)&buf_1_1[0], ARM_MPU_SH_NON, READ_WRITE, PRIVILEGED_ONLY, EXECUTION_PERMITTED),
              .RLAR = ARM_MPU_RLAR((uint32_t)(&buf_1_1[0] + sizeof(buf_1_1) - 1), REGION_2_ATTR_IDX)
            }
#endif
        };


        /* Disable MPU */

        ARM_MPU_Disable();
        ARM_MPU_Load(2, mpuTable, sizeof(mpuTable)/sizeof(ARM_MPU_Region_t));
#if 0
        ARM_MPU_SetMemAttr(REGION_2_ATTR_IDX, ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(NON_TRANSIENT, WRITE_BACK, READ_ALLOCATE, WRITE_ALLOCATE),
                                                           ARM_MPU_ATTR_MEMORY_(NON_TRANSIENT, WRITE_BACK, READ_ALLOCATE, WRITE_ALLOCATE)));
#else
        ARM_MPU_SetMemAttr(REGION_2_ATTR_IDX, ARM_MPU_ATTR(ARM_MPU_ATTR_NON_CACHEABLE, ARM_MPU_ATTR_NON_CACHEABLE));
#endif

        /* Enable MPU, enable default memory map as background, MPU enabled during fault and NMI handlers */

        ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

        SCB_EnableDCache(); //Invalidate and enable data cache
#endif
#endif
    }
}

#if BSP_TZ_SECURE_BUILD

FSP_CPP_HEADER
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ();

/* Trustzone Secure Projects require at least one nonsecure callable function in order to build (Remove this if it is not required to build). */
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ()
{

}
FSP_CPP_FOOTER

#endif