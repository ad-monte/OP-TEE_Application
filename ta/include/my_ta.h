/*
 * Copyright (c) 2016-2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MY_TA_H
#define MY_TA_H


/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define MY_TA_UUID \
	{ 0x16f9d3f4, 0x398d, 0x4394, \
		{ 0x93, 0x71, 0x91, 0x3b, 0xf5, 0x56, 0x92, 0xb9} }

// 	{ 0x16f9d3f4, 0x398d, 0x4394, \
	//	{ 0x91, 0x3b, 0xf5, 0x56, 0x92, 0x9b} }
				

/* The function IDs implemented in this TA */
// check this commands.

#define CMD_SECRET_MANAGMENT_STR   10  // Insecure storage
#define CMD_SECRET_MANAGMENT_GET   11  // Insecure storage
#define CMD_SECRET_MANAGMENT_ACC   12  // Insecure storage

#define CMD_LIGHT_CRYPTOGRAPHIC 3
#define CMD_INPUT_VALIDATION 4
// hello world CMD
#define MY_TA_CMD_INC_VALUE       1  // Leaky retrieval
#define MY_TA_CMD_DEC_VALUE   	  2  // State desync
// Project CMD


#endif /*TA_HELLO_WORLD_H*/
