/* $Id$ */

/*
 * Copyright (c) 2004 Damien Couderc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    - Neither the name of the copyright holder(s) nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


.data

	.globl x86_cpu_vendor

x86_cpu_vendor:
	.space 13


	.globl x86_cpu_name_str

x86_cpu_name_str:
	.space 49


.text

/*
	get cpu vendor string

	returns: pointer to static buffer
*/

	.globl get_x86_cpu_vendor

get_x86_cpu_vendor:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	/* set eax to 0 and call cpuid to get cpu vendor string */
	xorl	%eax,%eax
	cpuid

	/* copy result in x86_cpu_vendor */
	movl	%ebx,x86_cpu_vendor
	movl	%edx,x86_cpu_vendor+4
	movl	%ecx,x86_cpu_vendor+8
	movl	$0,x86_cpu_vendor+12

	popl	%edx
	popl	%ecx
	popl	%ebx

	/* return buffer address */
	movl	$x86_cpu_vendor,%eax

	ret


/*
	get cpu type

	returns: cpu type in eax
*/

	.globl get_x86_cpu_type

get_x86_cpu_type:
	movl	$1,%eax
	cpuid
	ret


/*
	get cpu name string

	returns: pointer to static buffer
*/

	.globl get_x86_cpu_name

get_x86_cpu_name:
	/* get first part */
	movl	$0x80000002,%eax
	cpuid

	/* copy result in cpu_name_str */
	movl	%eax,x86_cpu_name_str
	movl	%ebx,x86_cpu_name_str+4
	movl	%ecx,x86_cpu_name_str+8
	movl	%edx,x86_cpu_name_str+12

	/* get second part */
	movl	$0x80000003,%eax
	cpuid

	/* copy result in cpu_name_str */
	movl	%eax,x86_cpu_name_str+16
	movl	%ebx,x86_cpu_name_str+20
	movl	%ecx,x86_cpu_name_str+24
	movl	%edx,x86_cpu_name_str+28

	/* get first part */
	movl	$0x80000004,%eax
	cpuid

	/* copy result in cpu_name_str */
	movl	%eax,x86_cpu_name_str+32
	movl	%ebx,x86_cpu_name_str+36
	movl	%ecx,x86_cpu_name_str+40
	movl	%edx,x86_cpu_name_str+44

	/* ending 0 */
	movl	$0,x86_cpu_name_str+48

	/* return buffer address */
	movl	$x86_cpu_name_str,%eax

	ret



