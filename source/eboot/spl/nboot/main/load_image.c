/*
 * (C) Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_tee.h>
#include <u-boot/zlib.h>
#include <lzma/LzmaTools.h>
#include <u-boot/lz4.h>
#include <linux/zstd.h>

typedef struct _sunxi_image_head
{
	unsigned int  jump_instruction;
	unsigned char magic[MAGIC_SIZE];
	unsigned int  res1;
	unsigned int  res2;
	unsigned int  res3;
	unsigned int  res4;
	unsigned char res5[8];
	unsigned char res6[8];
	int           run_addr;
}sunxi_image_head;


extern const boot0_file_head_t  BT0_head;

int toc1_flash_read(u32 start_sector, u32 blkcnt, void *buff)
{
	memcpy(buff, (void *)(CONFIG_BOOTPKG_BASE + 512 * start_sector), 512 * blkcnt);

	return blkcnt;
}

int toc1_get_image_addr(u32 start_sector)
{
	sunxi_image_head *image_head = (sunxi_image_head *)(CONFIG_BOOTPKG_BASE + 512 * start_sector);

	return image_head->run_addr;
}

#ifdef CFG_SUNXI_ZSTD

#define ZSTD_MAX_WINDOWLOG 17
#define ZSTD_MAX_INPUT (1 << ZSTD_MAX_WINDOWLOG)

static u32 decompress_zstd(const u8 *cbuf, u32 clen, u8 *dbuf, u32 dlen)
{
	ZSTD_DStream *dstream;
	ZSTD_inBuffer in_buf;
	ZSTD_outBuffer out_buf;
	void *workspace;
	size_t wsize;
	u32 res = -1;

	wsize = ZSTD_DStreamWorkspaceBound(ZSTD_MAX_INPUT);
	workspace = malloc(wsize);
	if (!workspace) {
		printf("%s: cannot allocate workspace of size %zu\n", __func__,
		      wsize);
		return -1;
	}

	dstream = ZSTD_initDStream(ZSTD_MAX_INPUT, workspace, wsize);
	if (!dstream) {
		printf("%s: ZSTD_initDStream failed\n", __func__);
		goto err_free;
	}

	in_buf.src = cbuf;
	in_buf.pos = 0;
	in_buf.size = clen;

	out_buf.dst = dbuf;
	out_buf.pos = 0;
	out_buf.size = dlen;

	while (1) {
		size_t ret;

		ret = ZSTD_decompressStream(dstream, &out_buf, &in_buf);
		if (ZSTD_isError(ret)) {
			printf("%s: ZSTD_decompressStream error %d\n", __func__,
			       ZSTD_getErrorCode(ret));
			goto err_free;
		}

		if (in_buf.pos >= clen || !ret)
			break;
	}

	res = out_buf.pos;

err_free:
	free(workspace);
	return res;
}
#endif




int load_image(u32 *uboot_base, u32 *optee_base, u32 *monitor_base , u32 *rtos_base)
{
	int i;
	//int len;
	__maybe_unused void *dram_para_addr = (void *)BT0_head.prvt_head.dram_para;
	u32 image_base;

	struct sbrom_toc1_head_info  *toc1_head = NULL;
	struct sbrom_toc1_item_info  *item_head = NULL;

	struct sbrom_toc1_item_info  *toc1_item = NULL;

	toc1_head = (struct sbrom_toc1_head_info *)CONFIG_BOOTPKG_BASE;
	item_head = (struct sbrom_toc1_item_info *)(CONFIG_BOOTPKG_BASE + sizeof(struct sbrom_toc1_head_info));

#ifdef BOOT_DEBUG
	printf("*******************TOC1 Head Message*************************\n");
	printf("Toc_name          = %s\n",   toc1_head->name);
	printf("Toc_magic         = 0x%x\n", toc1_head->magic);
	printf("Toc_add_sum	      = 0x%x\n", toc1_head->add_sum);

	printf("Toc_serial_num    = 0x%x\n", toc1_head->serial_num);
	printf("Toc_status        = 0x%x\n", toc1_head->status);

	printf("Toc_items_nr      = 0x%x\n", toc1_head->items_nr);
	printf("Toc_valid_len     = 0x%x\n", toc1_head->valid_len);
	printf("TOC_MAIN_END      = 0x%x\n", toc1_head->end);
	printf("***************************************************************\n\n");
#endif
	//init
	toc1_item = item_head;
	for(i=0;i<toc1_head->items_nr;i++,toc1_item++)
	{
#ifdef BOOT_DEBUG
		printf("\n*******************TOC1 Item Message*************************\n");
		printf("Entry_name        = %s\n",   toc1_item->name);
		printf("Entry_data_offset = 0x%x\n", toc1_item->data_offset);
		printf("Entry_data_len    = 0x%x\n", toc1_item->data_len);

		printf("encrypt	          = 0x%x\n", toc1_item->encrypt);
		printf("Entry_type        = 0x%x\n", toc1_item->type);
		printf("run_addr          = 0x%x\n", toc1_item->run_addr);
		printf("index             = 0x%x\n", toc1_item->index);
		printf("Entry_end         = 0x%x\n", toc1_item->end);
		printf("***************************************************************\n\n");
#endif
		printf("Entry_name        = %s\n",   toc1_item->name);

		image_base = toc1_get_image_addr(toc1_item->data_offset/512);
		if(strncmp(toc1_item->name, ITEM_UBOOT_NAME, sizeof(ITEM_UBOOT_NAME)) == 0) {
			*uboot_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		}
		else if (strncmp(toc1_item->name, ITEM_OPTEE_NAME, sizeof(ITEM_OPTEE_NAME)) == 0) {
			*optee_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
			struct spare_optee_head *tee_head = (struct spare_optee_head *)image_base;
			memcpy(tee_head->dram_para, BT0_head.prvt_head.dram_para, 32*sizeof(int));
			memcpy(tee_head->chipinfo, &BT0_head.prvt_head.jtag_gpio[4], 8);
		}
		else if(strncmp(toc1_item->name, ITEM_MONITOR_NAME, sizeof(ITEM_MONITOR_NAME)) == 0) {
			*monitor_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		}
		else if(strncmp(toc1_item->name, ITEM_SCP_NAME, sizeof(ITEM_SCP_NAME)) == 0) {
#ifdef SCP_SRAM_BASE
			toc1_flash_read(toc1_item->data_offset / 512,
					(SCP_SRAM_SIZE + 511) / 512,
					(void *)SCP_SRAM_BASE);
			toc1_flash_read((toc1_item->data_offset +
					 SCP_CODE_DRAM_OFFSET) /
						512,
					(SCP_DRAM_SIZE + 511) / 512,
					(void *)SCP_DRAM_BASE);
			sunxi_deassert_arisc();
#endif
		}
		else if(strncmp(toc1_item->name, ITEM_DTB_NAME, sizeof(ITEM_DTB_NAME)) == 0) {
			/* note , uboot must be less than 2M*/
			image_base = *uboot_base + 2*1024*1024;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		}
		else if(strncmp(toc1_item->name, ITEM_SOCCFG_NAME, sizeof(ITEM_SOCCFG_NAME)) == 0) {
			image_base = *uboot_base + 1*1024*1024;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_LOGO_NAME,
				   sizeof(ITEM_LOGO_NAME)) == 0) {
			image_base	    = *uboot_base + 3 * 1024 * 1024;
			*(uint *)(image_base) = toc1_item->data_len;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)(image_base + 16));
			set_uboot_func_mask(UBOOT_FUNC_MASK_BIT_BOOTLOGO);
		}
		else if(strncmp(toc1_item->name, ITEM_RTOS_NAME, sizeof(ITEM_RTOS_NAME)) == 0) {
			*rtos_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		} else if(strncmp(toc1_item->name, ITEM_MELIS_NAME, sizeof(ITEM_MELIS_NAME)) == 0) {
			image_base = 0x40000000;
			*rtos_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		}
#ifdef CFG_SUNXI_GUNZIP
		else if((strncmp(toc1_item->name, ITEM_RTOS_GZ_NAME, sizeof(ITEM_RTOS_GZ_NAME)) == 0) ||
				(strncmp(toc1_item->name, ITEM_MELIS_GZ_NAME, sizeof(ITEM_MELIS_GZ_NAME)) == 0)) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from gz header */
			*rtos_base = image_base;
			void *dst = (void *)image_base;
			int dstlen = *(unsigned long *)(CONFIG_BOOTPKG_BASE + toc1_item->data_offset + toc1_item->data_len - 4);
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE + toc1_item->data_offset);
			unsigned long srclen = toc1_item->data_len;
			unsigned long *lenp = &srclen;
			int ret = gunzip(dst, dstlen, src, lenp);
			if (ret)
				return -1;
		}
#endif
#ifdef CFG_SUNXI_LZ4
		else if((strncmp(toc1_item->name, ITEM_RTOS_LZ4_NAME, sizeof(ITEM_RTOS_LZ4_NAME)) == 0) ||
				(strncmp(toc1_item->name, ITEM_MELIS_LZ4_NAME, sizeof(ITEM_MELIS_LZ4_NAME)) == 0)) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from lz4 header */
			*rtos_base = image_base;
			void *dst = (void *)image_base;
			unsigned int dstlen = 0x800000;
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE + toc1_item->data_offset);
			unsigned long srclen = toc1_item->data_len;
			int ret = ulz4fn(src, srclen, dst, &dstlen);
			if (ret)
				return -1;
		}
#endif
#ifdef CFG_SUNXI_ZSTD
		else if(strncmp(toc1_item->name, ITEM_RTOS_ZSTD_NAME, sizeof(ITEM_RTOS_ZSTD_NAME)) == 0) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from zstd header */
			*rtos_base = image_base;
			void *dst = (void *)image_base;
			unsigned int dstlen = 0x800000;
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE + toc1_item->data_offset);
			unsigned long srclen = toc1_item->data_len;
			int ret = decompress_zstd(src, srclen, dst, dstlen);
			if (ret)
				return -1;
		}
#endif
#ifdef CFG_SUNXI_LZMA
		else if(strncmp(toc1_item->name, ITEM_RTOS_LZMA_NAME, sizeof(ITEM_RTOS_LZMA_NAME)) == 0) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from lzma header */
			*rtos_base = image_base;
			SizeT src_len = ~0UL, dst_len = ~0UL;
			void *dst = (void *)image_base;
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE + toc1_item->data_offset);
			int ret = lzmaBuffToBuffDecompress(dst, &src_len, src, dst_len);
			if (ret)
				return -1;
		}
#endif
	}

	return 0;
}
