/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <aw-alsa-lib/pcm_plugin.h>
#include <aw-alsa-lib/pcm_config.h>
#include "pcm_local.h"
#include "pcm_direct.h"
#include <sound/aw_list.h>
#include <sound/aw_common.h>


int snd_pcm_slave_conf_hw_params(const snd_pcm_direct_slave_config_t *conf, struct slave_params *params)
{
	if (conf->format > SND_PCM_FORMAT_UNKNOWN)
		params->format = conf->format;
	if (conf->rate != 0)
		params->rate = conf->rate;
	if (conf->channels)
		params->channels = conf->channels;
	if (conf->period_size)
		params->period_size = conf->period_size;
	if (conf->buffer_size)
		params->buffer_size = conf->buffer_size;
	if (conf->periods)
		params->periods = conf->periods;

	if (params->period_size == -1 && params->period_time == -1)
		params->period_time = 125000;
	if (params->format == -2)
		params->format = SND_PCM_FORMAT_UNKNOWN;

	return 0;
}

typedef struct snd_pcm_direct_ipc {
	key_t ipc_key;
	snd_pcm_type_t type;
	snd_pcm_direct_share_t shmptr;
	sem_t semid;
	int use_count;
	pthread_mutex_t mutex;
	struct list_head list;
}snd_pcm_direct_ipc_t;

static LIST_HEAD(g_snd_pcm_direct_ipcs);


static snd_pcm_direct_ipc_t *snd_pcm_direct_ipc_find(snd_pcm_direct_t *dmix)
{
	snd_pcm_direct_ipc_t *ipc;

	list_for_each_entry(ipc, &g_snd_pcm_direct_ipcs, list) {
		if (ipc->ipc_key == dmix->ipc_key &&
			ipc->type == dmix->type) {
			return ipc;
		}
	}
	return NULL;
}

static snd_pcm_direct_ipc_t *snd_pcm_direct_ipc_create(snd_pcm_direct_t *dmix)
{
	int ret;
	snd_pcm_direct_ipc_t *ipc;

	ipc = snd_malloc(sizeof(snd_pcm_direct_ipc_t));
	if (!ipc) {
		awalsa_err("no memory\n");
		return NULL;
	}
	ipc->ipc_key = dmix->ipc_key;
	ipc->type = dmix->type;
	ipc->use_count = 0;
	ret = sem_init(&ipc->semid, 0, 1);
	if (ret != 0) {
		awalsa_err("sem_init failed\n");
		snd_free(ipc);
		return NULL;
	}
	pthread_mutex_init(&ipc->mutex, NULL);

	return ipc;
}

int snd_pcm_direct_semaphore_discard(snd_pcm_direct_t *dmix)
{
	snd_pcm_direct_ipc_t *ipc;
	ipc = container_of(dmix->semid, snd_pcm_direct_ipc_t, semid);

	if (!ipc) {
		awalsa_err("can't find sem...\n");
		return -1;
	}

	pthread_mutex_lock(&ipc->mutex);

	if (--ipc->use_count == 0) {
		list_del(&ipc->list);
		dmix->semid = NULL;
		pthread_mutex_unlock(&ipc->mutex);
		pthread_mutex_destroy(&ipc->mutex);
		sem_destroy(&ipc->semid);
		snd_free(ipc);
	} else {
		/* isn't the last sem, sem up */
		pthread_mutex_unlock(&ipc->mutex);
		snd_pcm_direct_semaphore_up(dmix, DIRECT_IPC_SEM_CLIENT);
		dmix->semid = NULL;
	}

	return 0;
}

int snd_pcm_direct_semaphore_create_or_connect(snd_pcm_direct_t *dmix)
{
	snd_pcm_direct_ipc_t *ipc;

	ipc = snd_pcm_direct_ipc_find(dmix);
	if (!ipc) {
		ipc = snd_pcm_direct_ipc_create(dmix);
		if (!ipc)
			return -1;
		list_add(&ipc->list, &g_snd_pcm_direct_ipcs);
	}
	pthread_mutex_lock(&ipc->mutex);
	ipc->use_count++;
	dmix->semid = &ipc->semid;
	pthread_mutex_unlock(&ipc->mutex);

	return 0;
}

int snd_pcm_direct_shm_discard(snd_pcm_direct_t *dmix)
{
	snd_pcm_direct_ipc_t *ipc;
	ipc = container_of(dmix->shmptr, snd_pcm_direct_ipc_t, shmptr);

	if (!ipc) {
		awalsa_err("can't find shm...\n");
		return -1;
	}

	pthread_mutex_lock(&ipc->mutex);
	if (--ipc->use_count == 0) {
		list_del(&ipc->list);
		dmix->shmptr = NULL;
		pthread_mutex_unlock(&ipc->mutex);
		pthread_mutex_destroy(&ipc->mutex);
		sem_destroy(&ipc->semid);
		snd_free(ipc);
		awalsa_err("maybe some error?discard seq: shm-->sem\n");
		return 0;
	}
	dmix->shmptr = NULL;
	pthread_mutex_unlock(&ipc->mutex);

	return 0;
}

#define SND_PCM_DIRECT_MAGIC    (0xa15ad300 + sizeof(snd_pcm_direct_share_t))

int snd_pcm_direct_shm_create_or_connect(snd_pcm_direct_t *dmix)
{
	snd_pcm_direct_ipc_t *ipc;
	int first_instance = 0;

	ipc = snd_pcm_direct_ipc_find(dmix);
	if (!ipc) {
		first_instance = 1;
		ipc = snd_pcm_direct_ipc_create(dmix);
	}
	if (!ipc)
		return -1;

	pthread_mutex_lock(&ipc->mutex);
	ipc->use_count++;
	dmix->shmptr = &ipc->shmptr;

	if (first_instance || ipc->use_count <= 2) {
		memset(dmix->shmptr, 0, sizeof(snd_pcm_direct_share_t));
		dmix->shmptr->magic = SND_PCM_DIRECT_MAGIC;
		pthread_mutex_unlock(&ipc->mutex);
		return 1;
	} else {
		if (dmix->shmptr->magic != SND_PCM_DIRECT_MAGIC) {
			pthread_mutex_unlock(&ipc->mutex);
			snd_pcm_direct_shm_discard(dmix);
			return -EINVAL;
		}
	}
	pthread_mutex_unlock(&ipc->mutex);

	return 0;
}

int snd_pcm_direct_last_pcm(snd_pcm_direct_t *dmix)
{
	int ret = -1;
	snd_pcm_direct_ipc_t *ipc;
	ipc = container_of(dmix->semid, snd_pcm_direct_ipc_t, semid);
	if (!ipc) {
		awalsa_err("can't find sem...\n");
		return -1;
	}

	pthread_mutex_lock(&ipc->mutex);
	if (ipc->use_count <= 2)
		ret = 0;
	pthread_mutex_unlock(&ipc->mutex);
	return ret;
}

#define COPY_SLAVE(field) (dmix->shmptr->s.field = spcm->field)

static void save_slave_setting(snd_pcm_direct_t *dmix, snd_pcm_t *spcm)
{
#if 0
	spcm->info &= ~SND_PCM_INFO_PAUSE;
#endif
	COPY_SLAVE(access);
	COPY_SLAVE(format);
	COPY_SLAVE(channels);
	COPY_SLAVE(rate);
	COPY_SLAVE(period_size);
	COPY_SLAVE(period_time);
	COPY_SLAVE(periods);
	COPY_SLAVE(avail_min);
	COPY_SLAVE(start_threshold);
	COPY_SLAVE(stop_threshold);
	COPY_SLAVE(silence_size);
	COPY_SLAVE(boundary);
	COPY_SLAVE(buffer_size);
	COPY_SLAVE(buffer_time);
	COPY_SLAVE(sample_bits);
	COPY_SLAVE(frame_bits);

#if 0
	dmix->shmptr->s.info &= ~SND_PCM_INFO_RESUME;
#endif

}

#undef COPY_SLAVE

int snd_pcm_direct_initialize_slave(snd_pcm_direct_t *dmix, snd_pcm_t *spcm, struct slave_params *params)
{
	int ret, buffer_is_not_initialized;
	snd_pcm_hw_params_t hw_params = {0};
	snd_pcm_sw_params_t sw_params = {0};
	snd_pcm_uframes_t boundary;

	ret = snd_pcm_hw_params_any(spcm, &hw_params);
	if (ret < 0) {
		awalsa_err("snd_pcm_hw_params_any failed\n");
		return ret;
	}
	ret = snd_pcm_hw_params_set_access(spcm, &hw_params,
					SND_PCM_ACCESS_MMAP_INTERLEAVED);
	if (ret < 0) {
		awalsa_err("snd_pcm_hw_params_set_access MMAP_INTERLEAVED failed\n");
		return ret;
	}

	if (params->format == SND_PCM_FORMAT_UNKNOWN)
		ret = -EINVAL;
	else
		ret = snd_pcm_hw_params_set_format(spcm, &hw_params,
						params->format);
	if (ret < 0) {
		awalsa_err("snd_pcm_hw_params_set_format %u failed\n", params->format);
		return ret;
	}
	ret = snd_pcm_hw_params_set_channels(spcm, &hw_params, params->channels);
	if (ret < 0) {
		awalsa_err("snd_pcm_hw_params_set_channels %u failed\n", params->channels);
		return ret;
	}
	ret = snd_pcm_hw_params_set_rate(spcm, &hw_params, params->rate, NULL);
	if (ret < 0) {
		awalsa_err("snd_pcm_hw_params_set_rate %u failed\n", params->rate);
		return ret;
	}
	buffer_is_not_initialized = 0;
	if (params->buffer_time > 0) {
		ret = snd_pcm_hw_params_set_buffer_time(spcm, &hw_params, params->buffer_time);
		if (ret < 0) {
			awalsa_err("snd_pcm_hw_params_set_buffer_time %u failed\n", params->buffer_time);
			return ret;
		}
	} else if (params->buffer_size > 0) {
		ret = snd_pcm_hw_params_set_buffer_size(spcm, &hw_params, params->buffer_size);
		if (ret < 0) {
			awalsa_err("snd_pcm_hw_params_set_buffer_size %lu failed\n", params->buffer_size);
			return ret;
		}
	} else {
		buffer_is_not_initialized = 1;
	}

	if (params->period_time > 0) {
		ret = snd_pcm_hw_params_set_period_time(spcm, &hw_params, params->period_time, NULL);
		if (ret < 0) {
			awalsa_err("snd_pcm_hw_params_set_period_time %u failed\n", params->period_time);
			return ret;
		}
	} else if (params->period_size > 0) {
		ret = snd_pcm_hw_params_set_period_size(spcm, &hw_params, params->period_size, NULL);
		if (ret < 0) {
			awalsa_err("snd_pcm_hw_params_set_period_size %lu failed\n", params->period_size);
			return ret;
		}
	}
	if (buffer_is_not_initialized && params->periods > 0) {
		ret = snd_pcm_hw_params_set_periods(spcm, &hw_params, params->periods, 0);
		if (ret < 0) {
			awalsa_err("snd_pcm_hw_params_set_periods %u failed\n", params->periods);
			return ret;
		}
	}

	ret = snd_pcm_hw_params(spcm, &hw_params);
	if (ret < 0) {
		awalsa_err("unable to install hw params\n");
		return ret;
	}

	dmix->shmptr->hw.format = params_format(&hw_params);
	dmix->shmptr->hw.rate = params_rate(&hw_params);
	dmix->shmptr->hw.buffer_size = params_buffer_size(&hw_params);
	dmix->shmptr->hw.buffer_time = params_buffer_time(&hw_params);
	dmix->shmptr->hw.period_size = params_period_size(&hw_params);
	dmix->shmptr->hw.period_time = params_period_time(&hw_params);
	dmix->shmptr->hw.periods = params_periods(&hw_params);

	ret = snd_pcm_sw_params_current(spcm, &sw_params);
	if (ret < 0) {
		awalsa_err("unable to get current sw_params\n");
		return ret;
	}
	ret = snd_pcm_sw_params_get_boundary(&sw_params, &boundary);
	if (ret < 0) {
		awalsa_err("unable to get boundary\n");
		return ret;
	}
	ret = snd_pcm_sw_params_set_stop_threshold(spcm, &sw_params, boundary);
	if (ret < 0) {
		awalsa_err("unable to set stop threshold\n");
		return ret;
	}

	if (dmix->type != SND_PCM_TYPE_DMIX)
		goto __skip_silencing;
#if 0
	ret = snd_pcm_sw_params_set_silence_threshold(spcm, &sw_params, 0);
	if (ret < 0) {
		awalsa_err("unable to set silence threshold\n");
		return ret;
	}
#endif
	ret = snd_pcm_sw_params_set_silence_size(spcm, &sw_params, boundary);
	if (ret < 0) {
		awalsa_err("unable to set silence size\n");
		return ret;
	}

__skip_silencing:
	ret = snd_pcm_sw_params(spcm, &sw_params);
	if (ret < 0) {
		awalsa_err("unable to install sw params\n");
		return ret;
	}

	/* ensure ringbuffer empty, avoid noise */
	if (dmix->type == SND_PCM_TYPE_DMIX) {
		const snd_pcm_channel_area_t *dst_areas;
		dst_areas = snd_pcm_mmap_areas(spcm);
		snd_pcm_areas_silence(dst_areas, 0, spcm->channels,
					spcm->buffer_size, spcm->format);
	}

	/* start PCM stream (dma start) */
	ret = snd_pcm_start(spcm);
	if (ret < 0) {
		awalsa_err("unable to start PCM stream\n");
		return ret;
	}

	save_slave_setting(dmix, spcm);

	dmix->slave_buffer_size = spcm->buffer_size;
	dmix->slave_period_size = spcm->period_size;
	dmix->slave_boundary = spcm->boundary;

	spcm->donot_close = 1;

	return 0;
}

#define COPY_SLAVE(field) (spcm->field = dmix->shmptr->s.field)

static void copy_slave_setting(snd_pcm_direct_t *dmix, snd_pcm_t *spcm)
{
	COPY_SLAVE(access);
	COPY_SLAVE(format);
	COPY_SLAVE(channels);
	COPY_SLAVE(rate);
	COPY_SLAVE(period_size);
	COPY_SLAVE(period_time);
	COPY_SLAVE(periods);
	COPY_SLAVE(avail_min);
	COPY_SLAVE(start_threshold);
	COPY_SLAVE(stop_threshold);
	COPY_SLAVE(silence_size);
	COPY_SLAVE(boundary);
	COPY_SLAVE(buffer_size);
	COPY_SLAVE(buffer_time);
	COPY_SLAVE(sample_bits);
	COPY_SLAVE(frame_bits);
}

#undef COPY_SLAVE

int snd_pcm_direct_initialize_secondary_slave(snd_pcm_direct_t *dmix,
						snd_pcm_t *spcm,
						struct slave_params *params)
{
	int ret;

	spcm->donot_close = 1;
	spcm->setup = 1;

	copy_slave_setting(dmix, spcm);

	dmix->slave_buffer_size = spcm->buffer_size;
	dmix->slave_period_size = spcm->period_size;
	dmix->slave_boundary = spcm->boundary;

	ret = snd_pcm_mmap(spcm);
	if (ret < 0) {
		awalsa_err("unable to mmap channels\n");
		return ret;
	}
	return 0;
}

int snd_pcm_direct_slave_recover(snd_pcm_direct_t *direct)
{
	int ret;
	int semerr;

	semerr = snd_pcm_direct_semaphore_down(direct,
						   DIRECT_IPC_SEM_CLIENT);
	if (semerr < 0) {
		awalsa_err("SEMDOWN FAILED with err %d\n", semerr);
		return semerr;
	}

	if (snd_pcm_state(direct->spcm) != SND_PCM_STATE_XRUN) {
		/* ignore... someone else already did recovery */
		semerr = snd_pcm_direct_semaphore_up(direct,
						     DIRECT_IPC_SEM_CLIENT);
		if (semerr < 0) {
			awalsa_err("SEMUP FAILED with err %d\n", semerr);
			return semerr;
		}
		return 0;
	}

	ret = snd_pcm_prepare(direct->spcm);
	if (ret < 0) {
		awalsa_err("recover: unable to prepare slave\n");
		semerr = snd_pcm_direct_semaphore_up(direct,
						     DIRECT_IPC_SEM_CLIENT);
		if (semerr < 0) {
			awalsa_err("SEMUP FAILED with err %d\n", semerr);
			return semerr;
		}
		return ret;
	}

#if 0
	if (direct->type == SND_PCM_TYPE_DSHARE) {
		const snd_pcm_channel_area_t *dst_areas;
		dst_areas = snd_pcm_mmap_areas(direct->spcm);
		snd_pcm_areas_silence(dst_areas, 0, direct->spcm->channels,
				      direct->spcm->buffer_size,
				      direct->spcm->format);
	}
#endif

	ret = snd_pcm_start(direct->spcm);
	if (ret < 0) {
		awalsa_err("recover: unable to start slave\n");
		semerr = snd_pcm_direct_semaphore_up(direct,
						     DIRECT_IPC_SEM_CLIENT);
		if (semerr < 0) {
			awalsa_err("SEMUP FAILED with err %d\n", semerr);
			return semerr;
		}
		return ret;
	}
	direct->shmptr->s.recoveries++;
	semerr = snd_pcm_direct_semaphore_up(direct,
						 DIRECT_IPC_SEM_CLIENT);
	if (semerr < 0) {
		awalsa_err("SEMUP FAILED with err %d\n", semerr);
		return semerr;
	}
	return 0;
}

/*
 * enter xrun state, if slave xrun occurred
 * @return: 0 - no xrun >0: xrun happened
 */
int snd_pcm_direct_client_chk_xrun(snd_pcm_direct_t *direct, snd_pcm_t *pcm)
{
	if (direct->shmptr->s.recoveries != direct->recoveries) {
		/* no matter how many xruns we missed -
		 * so don't increment but just update to actual counter
		 */
		direct->recoveries = direct->shmptr->s.recoveries;
		pcm->fast_ops->drop(pcm);
#if 0
		/* trigger_tstamp update is missing in drop callbacks */
		gettimestamp(&direct->trigger_tstamp, pcm->tstamp_type);
#endif
		/* no timer clear:
		 * if slave already entered xrun again the event is lost.
		 * snd_pcm_direct_clear_timer_queue(direct);
		 */
		direct->state = SND_PCM_STATE_XRUN;
		return 1;
	}
	return 0;
}

/*
 *  ring buffer operation
 */
int snd_pcm_direct_check_interleave(snd_pcm_direct_t *dmix, snd_pcm_t *pcm)
{
	unsigned int chn, channels;
	int bits, interleaved = 1;
	const snd_pcm_channel_area_t *dst_areas;
	const snd_pcm_channel_area_t *src_areas;

	bits = snd_pcm_format_physical_width(pcm->format);
	if ((bits % 8) != 0)
		interleaved = 0;
	channels = dmix->channels;
	dst_areas = snd_pcm_mmap_areas(dmix->spcm);
	src_areas = snd_pcm_mmap_areas(pcm);
	for (chn = 1; chn < channels; chn++) {
		if (dst_areas[chn-1].addr != dst_areas[chn].addr) {
			interleaved = 0;
			break;
		}
		if (src_areas[chn-1].addr != src_areas[chn].addr) {
			interleaved = 0;
			break;
		}
	}
	for (chn = 0; chn < channels; chn++) {
#if 0
		if (dmix->bindings && dmix->bindings[chn] != chn) {
			interleaved = 0;
			break;
		}
#endif
		if (dst_areas[chn].first != chn * bits ||
		    dst_areas[chn].step != channels * bits) {
			interleaved = 0;
			break;
		}
		if (src_areas[chn].first != chn * bits ||
		    src_areas[chn].step != channels * bits) {
			interleaved = 0;
			break;
		}
	}
	return dmix->interleaved = interleaved;
}

int snd_pcm_direct_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_direct_t *direct = pcm->private_data;
	snd_pcm_t *spcm = direct->spcm;
	return spcm->ops->hw_refine(spcm->op_arg, params);
}

int snd_pcm_direct_prepare(snd_pcm_t *pcm)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	int err;

	switch (snd_pcm_state(dmix->spcm)) {
	case SND_PCM_STATE_SETUP:
	case SND_PCM_STATE_XRUN:
	case SND_PCM_STATE_SUSPENDED:
		err = snd_pcm_prepare(dmix->spcm);
		if (err < 0)
			return err;
		snd_pcm_start(dmix->spcm);
		break;
	case SND_PCM_STATE_OPEN:
	case SND_PCM_STATE_DISCONNECTED:
		return -EBADFD;
	default:
		break;
	}
	snd_pcm_direct_check_interleave(dmix, pcm);
	dmix->state = SND_PCM_STATE_PREPARED;
	dmix->appl_ptr = dmix->last_appl_ptr = 0;
	dmix->hw_ptr = 0;
#if 0
	return snd_pcm_direct_set_timer_params(dmix);
#endif
	return 0;
}

int snd_pcm_direct_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_direct_t *dmix = pcm->private_data;

	params->can_paused = 0;

	/*TODO*/
	/*params->intervals[SND_PCM_HW_PARAM_BUFFER_SIZE].range.min = dmix->spcm->buffer_size;*/
	return 0;
}

int snd_pcm_direct_hw_free(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_pcm_direct_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t * params)
{
	return 0;
}

int snd_pcm_direct_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t * info)
{
        return snd_pcm_channel_info_shm(pcm, info, -1);
}

int snd_pcm_direct_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_pcm_direct_munmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

int snd_pcm_direct_wait(snd_pcm_t *pcm, int timeout)
{
	snd_pcm_direct_t *dmix = pcm->private_data;
	snd_pcm_t *spcm = dmix->spcm;

	return snd_pcm_hw_wait_with_index(spcm, dmix->poll_index, timeout);
}

int snd_pcm_direct_initialize_poll_index(snd_pcm_direct_t *dmix)
{
	snd_pcm_t *spcm = dmix->spcm;

	dmix->poll_index = snd_pcm_hw_poll_index_init(spcm);
	if (dmix->poll_index < 0) {
		awalsa_err("init poll index failed\n");
		return -1;
	}
	return 0;
}


int snd_pcm_direct_destroy_poll_index(snd_pcm_direct_t *dmix)
{
	snd_pcm_t *spcm = dmix->spcm;

	if (dmix->poll_index <= 0)
		return 0;
	return snd_pcm_hw_poll_index_release(spcm, dmix->poll_index);
}
