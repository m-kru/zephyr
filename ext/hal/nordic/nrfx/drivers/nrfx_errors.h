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

#ifndef NRFX_ERRORS_H__
#define NRFX_ERRORS_H__

#if !defined(NRFX_ERRORS_DEFINED_EXTERNALLY)

/**
@defgroup nrfx_error nrfx Global Error Codes
@{

@brief Global Error definitions
*/

#define NRFX_ERROR_BASE_NUM                 0xBAD00000
#define NRFX_ERROR_SDK_COMMON_ERROR_BASE    (NRFX_ERROR_BASE_NUM +  0x0080)
#define NRFX_ERROR_PERIPH_DRIVERS_ERR_BASE  (NRFX_ERROR_BASE_NUM + 0x10000)

typedef enum {
    NRFX_SUCCESS                = (NRFX_ERROR_BASE_NUM + 0),  ///< Successful command
    NRFX_ERROR_INTERNAL         = (NRFX_ERROR_BASE_NUM + 3),  ///< Internal Error
    NRFX_ERROR_NO_MEM           = (NRFX_ERROR_BASE_NUM + 4),  ///< No Memory for operation
    NRFX_ERROR_NOT_SUPPORTED    = (NRFX_ERROR_BASE_NUM + 6),  ///< Not supported
    NRFX_ERROR_INVALID_PARAM    = (NRFX_ERROR_BASE_NUM + 7),  ///< Invalid Parameter
    NRFX_ERROR_INVALID_STATE    = (NRFX_ERROR_BASE_NUM + 8),  ///< Invalid state, operation disallowed in this state
    NRFX_ERROR_INVALID_LENGTH   = (NRFX_ERROR_BASE_NUM + 9),  ///< Invalid Length
    NRFX_ERROR_TIMEOUT          = (NRFX_ERROR_BASE_NUM + 13), ///< Operation timed out
    NRFX_ERROR_FORBIDDEN        = (NRFX_ERROR_BASE_NUM + 15), ///< Forbidden Operation
    NRFX_ERROR_NULL             = (NRFX_ERROR_BASE_NUM + 14), ///< Null Pointer
    NRFX_ERROR_INVALID_ADDR     = (NRFX_ERROR_BASE_NUM + 16), ///< Bad Memory Address
    NRFX_ERROR_BUSY             = (NRFX_ERROR_BASE_NUM + 17), ///< Busy

    NRFX_ERROR_MODULE_ALREADY_INITIALIZED = (NRFX_ERROR_SDK_COMMON_ERROR_BASE + 0x0005),

    NRFX_ERROR_DRV_TWI_ERR_OVERRUN  = (NRFX_ERROR_PERIPH_DRIVERS_ERR_BASE + 0x0000),
    NRFX_ERROR_DRV_TWI_ERR_ANACK    = (NRFX_ERROR_PERIPH_DRIVERS_ERR_BASE + 0x0001),
    NRFX_ERROR_DRV_TWI_ERR_DNACK    = (NRFX_ERROR_PERIPH_DRIVERS_ERR_BASE + 0x0002)
} ret_code_t;

/**
@}
*/

#endif // NRFX_ERRORS_DEFINED_EXTERNALLY

#endif // NRFX_ERRORS_H__
