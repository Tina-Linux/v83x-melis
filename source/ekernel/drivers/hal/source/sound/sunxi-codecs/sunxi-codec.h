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
#ifndef __SUNXI_CODEC_H
#define __SUNXI_CODEC_H
#include <hal_clk.h>
#include <sound/snd_io.h>

#include "sun8iw19-codec.h"

unsigned int sunxi_codec_read(struct snd_codec *codec, unsigned int reg);
int sunxi_codec_write(struct snd_codec *codec, unsigned int reg, unsigned int val);

struct sunxi_codec_param {
	int16_t gpio_spk;
	int16_t pa_msleep_time;
	uint8_t pa_level;
	uint8_t digital_vol;
	uint8_t lineout_vol;
	uint8_t mic1gain;
	uint8_t lineingain;
	uint8_t adcdrc_cfg;
	uint8_t adchpf_cfg;
	uint8_t dacdrc_cfg;
	uint8_t dachpf_cfg;
	uint8_t lineout_diffen;
};

struct sunxi_codec_info {
	hal_clk_id_t pllclk;
	hal_clk_id_t moduleclk;

	struct sunxi_codec_param param;
};

struct sample_rate {
	unsigned int samplerate;
	unsigned int rate_bit;
};

extern struct snd_pcm_ops sunxi_pcm_ops;
extern int sunxi_pcm_new(struct snd_pcm *pcm);
extern void sunxi_pcm_free(struct snd_pcm *pcm);

#endif /* __SUNXI_CODEC_H */
