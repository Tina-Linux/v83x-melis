/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "os_debug.h"
#include "sys/param.h"

#if XR_OS_RESOURCE_TRACE

extern int g_xr_msgqueue_cnt;
extern int g_xr_mutex_cnt;
extern int g_xr_semaphore_cnt;
extern int g_xr_thread_cnt;
extern int g_xr_timer_cnt;

void xr_os_resource_info(void)
{
	XR_OS_LOG(1, "<<< xr os resource info >>>\n");
	XR_OS_LOG(1, "g_xr_msgqueue_cnt  %d\n", g_xr_msgqueue_cnt);
	XR_OS_LOG(1, "g_xr_mutex_cnt     %d\n", g_xr_mutex_cnt);
	XR_OS_LOG(1, "g_xr_semaphore_cnt %d\n", g_xr_semaphore_cnt);
	XR_OS_LOG(1, "g_xr_thread_cnt    %d\n", g_xr_thread_cnt);
	XR_OS_LOG(1, "g_xr_timer_cnt     %d\n", g_xr_timer_cnt);
}

#endif /* XR_OS_RESOURCE_TRACE */

