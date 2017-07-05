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

#ifndef NRFX_GLUE_H__
#define NRFX_GLUE_H__

#ifdef __cplusplus
extern "C" {
#endif


// NRFX_ASSERT(expression)
#define NRFX_ASSERT(expression)

// NRFX_STATIC_ASSERT(expression)
#define NRFX_STATIC_ASSERT(expression)


// NRFX_IRQ_PRIORITY_SET(IRQn, priority)
// NRFX_IRQ_ENABLE(IRQn)
// NRFX_IRQ_IS_ENABLED(IRQn)
// NRFX_IRQ_DISABLE(IRQn)
#include <irq.h>
static inline void NRFX_IRQ_PRIORITY_SET(IRQn_Type irq_number, uint8_t priority)
{
    // Intentionally empty. Priorities of IRQs are set through IRQ_CONNECT.
}
static inline void NRFX_IRQ_ENABLE(IRQn_Type irq_number)
{
    irq_enable(irq_number);
}
static inline bool NRFX_IRQ_IS_ENABLED(IRQn_Type irq_number)
{
    return irq_is_enabled(irq_number);
}
static inline void NRFX_IRQ_DISABLE(IRQn_Type irq_number)
{
    irq_disable(irq_number);
}


// NRFX_CRITICAL_SECTION_ENTER()
// NRFX_CRITICAL_SECTION_EXIT()
#include <irq.h>
#define NRFX_CRITICAL_SECTION_ENTER()  { unsigned int irq_lock_key = irq_lock();
#define NRFX_CRITICAL_SECTION_EXIT()     irq_unlock(irq_lock_key); }


#ifdef __cplusplus
}
#endif

#endif // NRFX_GLUE_H__
