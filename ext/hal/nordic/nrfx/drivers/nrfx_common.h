/**
 * Copyright (c) 2017 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef NRFX_COMMON_H__
#define NRFX_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrfx_common nrfx common module
 * @{
 * @ingroup nrfx
 */

#define NRFX_CHECK(module_enabled)  (module_enabled)

#define NRFX_CONCAT_2(p1, p2)       NRFX_CONCAT_2_(p1, p2)
#define NRFX_CONCAT_2_(p1, p2)      p1 ## p2

#define NRFX_CONCAT_3(p1, p2, p3)   NRFX_CONCAT_3_(p1, p2, p3)
#define NRFX_CONCAT_3_(p1, p2, p3)  p1 ## p2 ## p3

#define NRFX_ROUNDED_DIV(a, b)  (((a) + ((b) / 2)) / (b))


/**
 * @brief IRQ handler type.
 */
typedef void (* nrfx_irq_handler_t)(void);

/**
 * @brief Driver state.
 */
typedef enum
{
    NRFX_DRV_STATE_UNINITIALIZED, ///< Uninitialized.
    NRFX_DRV_STATE_INITIALIZED,   ///< Initialized but powered off.
    NRFX_DRV_STATE_POWERED_ON,    ///< Initialized and powered on.
} nrfx_drv_state_t;


__STATIC_INLINE bool nrfx_is_in_ram(void const * p_object);

__STATIC_INLINE IRQn_Type nrfx_get_irq_number(void const * p_reg);

__STATIC_INLINE uint32_t nrfx_bitpos_to_event(uint32_t bit);

__STATIC_INLINE uint32_t nrfx_event_to_bitpos(uint32_t event);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE bool nrfx_is_in_ram(void const * p_object)
{
    return ((((uint32_t)p_object) & 0xE0000000u) == 0x20000000u);
}

__STATIC_INLINE IRQn_Type nrfx_get_irq_number(void const * p_reg)
{
    uint8_t irq_number = (uint8_t)(((uint32_t)p_reg) >> 12u);
    return (IRQn_Type)irq_number;
}

__STATIC_INLINE uint32_t nrfx_bitpos_to_event(uint32_t bit)
{
    static const uint32_t event_reg_offset = 0x100u;
    return event_reg_offset + (bit * sizeof(uint32_t));
}

__STATIC_INLINE uint32_t nrfx_event_to_bitpos(uint32_t event)
{
    static const uint32_t event_reg_offset = 0x100u;
    return (event - event_reg_offset) / sizeof(uint32_t);
}

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRFX_COMMON_H__
