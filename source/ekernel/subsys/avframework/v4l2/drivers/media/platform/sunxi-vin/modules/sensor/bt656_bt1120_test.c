/*
 * A V4L2 driver for csi bt1120 test cameras.
 *
 * Copyright (c) 2021 by Lindenis Co., Ltd.  http://www.lindenis.com
 * Authors:  Michael <michael@lindeni.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <melis/init.h>
#include <linux/videodev2.h>
#include <hal_timer.h>
#include <v4l2-device.h>
#include <v4l2-mediabus.h>

#include "camera.h"
#include "sensor_helper.h"


struct v4l2_subdev *gl_sd;

//MODULE_AUTHOR("michael");
//MODULE_DESCRIPTION("A low-level driver for bt1120 sensors");
//MODULE_LICENSE("GPL");

#define MCLK (24 * 1000 * 1000)
#define CLK_POL V4L2_MBUS_PCLK_SAMPLE_RISING
//#define CLK_POL V4L2_MBUS_PCLK_SAMPLE_FALLING
#define V4L2_IDENT_SENSOR 0x00c8

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 30


#define I2C_ADDR 0x60
#define SENSOR_NAME "bt656_bt1120_in"

static struct regval_list sensor_default_regs[] = {
};

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	return 0;
}

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	switch (on) {
	case STBY_ON:
		sensor_print("CSI_SUBDEV_STBY_ON!\n");
		sensor_s_sw_stby(sd, ON);
		break;
	case STBY_OFF:
		sensor_print("CSI_SUBDEV_STBY_OFF!\n");
		sensor_s_sw_stby(sd, OFF);
		break;
	case PWR_ON:
		sensor_print("CSI_SUBDEV_PWR_ON!\n");
		cci_lock(sd);
		usleep(1200);
		usleep(10000);
		cci_unlock(sd);
		break;
	case PWR_OFF:
		sensor_print("CSI_SUBDEV_PWR_OFF!\n");
		cci_lock(sd);
		usleep(1200);
		usleep(1200);
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	usleep(6000);
	usleep(6000);
	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	/*Make sure it is a target sensor */
	ret = sensor_detect(sd);
	if (ret) {
		sensor_err("chip found is not an target chip.\n");
		return ret;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = 1920; /* VGA_WIDTH; */
	info->height = 1080; /* VGA_HEIGHT; */
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = 30; /* 30fps */

	info->preview_first_flag = 1;
	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);

	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL) {
			memcpy(arg, info->current_wins,
			       sizeof(struct sensor_win_size));
			ret = 0;
		} else {
			sensor_err("empty wins!\n");
			ret = -1;
		}
		break;
	case SET_FPS:
		break;
	case VIDIOC_VIN_SENSOR_CFG_REQ:
		sensor_cfg_req(sd, (struct sensor_config *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

/*
 * Store information about the video data format.
 */
static struct sensor_format_struct sensor_formats[] = {
	{
	.desc = "BT1120",
	//.mbus_code = MEDIA_BUS_FMT_ARGB8888_1X32,//MEDIA_BUS_FMT_RBG888_1X24,//MEDIA_BUS_FMT_VYUY8_1X16, /* MEDIA_BUS_FMT_YVYU8_1X16, */
	.mbus_code = MEDIA_BUS_FMT_VYUY8_1X16,//2 OK MEDIA_BUS_FMT_RBG888_1X24,//MEDIA_BUS_FMT_VYUY8_1X16, /* MEDIA_BUS_FMT_YVYU8_1X16, */
	//.mbus_code = MEDIA_BUS_FMT_VYUY8_2X8,//MEDIA_BUS_FMT_RBG888_1X24,//MEDIA_BUS_FMT_VYUY8_1X16, /* MEDIA_BUS_FMT_YVYU8_1X16, */
	.regs = NULL,
	.regs_size = 0,
	.bpp = 2,
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
	{
	.width = 1920,
	.height = 1080,
	.hoffset = 0,
	.voffset = 0,
	.regs = sensor_default_regs,
	.regs_size = ARRAY_SIZE(sensor_default_regs),
	.set_size = NULL,
	},
	{
	.width = 1280,
	.height = 720,
	.hoffset = 0,
	.voffset = 0,
	.regs = sensor_default_regs,
	.regs_size = ARRAY_SIZE(sensor_default_regs),
	.set_size = NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_BT656;
	cfg->flags = CLK_POL | CSI_CH_0;
	return 0;
}

static int sensor_reg_init(struct sensor_info *info)
{
	struct v4l2_subdev *sd = &info->sd;
	struct sensor_format_struct *sensor_fmt = info->fmt;
	struct sensor_win_size *wsize = info->current_wins;

	sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size);

	if (wsize->regs)
		sensor_write_array(sd, wsize->regs, wsize->regs_size);

	if (wsize->set_size)
		wsize->set_size(sd);

	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;
	return 0;
}

static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sensor_info *info = to_state(sd);

	sensor_print("%s on = %d, %d*%d %x\n", __func__, enable,
		     info->current_wins->width, info->current_wins->height,
		     info->fmt->mbus_code);

	if (!enable)
		return 0;
	return 0;//sensor_reg_init(info);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.s_parm = sensor_s_parm,
	.g_parm = sensor_g_parm,
	.s_stream = sensor_s_stream,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_pad_ops sensor_pad_ops = {
	.enum_mbus_code = sensor_enum_mbus_code,
	.enum_frame_size = sensor_enum_frame_size,
	.get_fmt = sensor_get_fmt,
	.set_fmt = sensor_set_fmt,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.pad = &sensor_pad_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv[] = {
	{
	.name = SENSOR_NAME,
	.addr_width = CCI_BITS_8,
	.data_width = CCI_BITS_8,
	},
	{}
};

static int sensor_probe(int id)
{

	struct sensor_info *info;

	info = rt_calloc(1, sizeof(struct sensor_info));
	if (info == NULL)
		return -ENOMEM;
	gl_sd = &info->sd;
	cci_dev_probe_helper(gl_sd, &sensor_ops, &cci_drv[id]);
	rt_mutex_init(&info->lock, "sinfomtx", RT_IPC_FLAG_FIFO);

	info->fmt = &sensor_formats[0];
	info->fmt_pt = &sensor_formats[0];
	info->win_pt = &sensor_win_sizes[0];
	info->fmt_num = N_FMTS;
	info->win_size_num = N_WIN_SIZES;
	info->sensor_field = V4L2_FIELD_NONE;

	return 0;
}

static int sensor_remove(int id)
{
	struct v4l2_subdev *sd;

	sd = cci_dev_remove_helper(&cci_drv[id]);

	rt_free(to_state(sd));
	return 0;
}

int bt656_bt1120_test_init(void)
{
	sensor_probe(0);
	//sensor_probe(1);

	return 0;
}

int bt656_bt1120_test_remove(void)
{
	sensor_remove(0);
//	sensor_remove(1);

	return 0;
}

device_initcall(bt656_bt1120_test_init);
