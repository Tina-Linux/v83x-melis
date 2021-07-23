/*
*************************************************************************************
*                                   eGon
*                           Application Of eGon2.0
*
*               (c) Copyright 2006-2010, All winners Co,Ld.
*                           All Rights Reserved
*
* File Name     : Parse_Picture.c
*
* Author        : javen
*
* Description   : ͼƬ����
*
* History       :
*      <author>         <time>          <version >          <desc>
*       javen          2010-09-10          1.0            create this file
*
*************************************************************************************
*/

#include "parse_bmp.h"
#include "bmp.h"
#include <string.h>
#include <log.h>

#define spcl_size_align( x, y )         ( ( (x) + (y) - 1 ) & ~( (y) - 1 ) )
#define abs(x) (x) >= 0 ? (x):-(x)

/*
*******************************************************************************
*                     Parse_Pic_BMP_ByBuffer
*
* Description:
*    ����������ڴ��е�ͼƬ
*
* Parameters:
*    Pic_Buffer     :  input.  ���ͼƬ�����
*    Pic_BufferSize :  input.  ��������С
*    PictureInfo    :  output. ͼƬ���������Ϣ
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int32_t Parse_Pic_BMP_ByBuffer(void *Pic_Buffer, uint32_t Pic_BufferSize, Picture_t *PictureInfo)
{
    bmp_info_head_t *info_head_p = NULL;

    info_head_p = (bmp_info_head_t *)((uint32_t)Pic_Buffer + sizeof(bmp_file_head_t));

    PictureInfo->BitCount = info_head_p->biBitCount;
    PictureInfo->Width    = info_head_p->biWidth;
    PictureInfo->Height   = abs(info_head_p->biHeight);
    PictureInfo->RowSize  = spcl_size_align(info_head_p->biWidth * (info_head_p->biBitCount >> 3), 4);

    PictureInfo->Buffer     = (void *)((uint32_t)Pic_Buffer + sizeof(bmp_info_head_t) + sizeof(bmp_file_head_t));
    PictureInfo->BufferSize = Pic_BufferSize - (sizeof(bmp_info_head_t) + sizeof(bmp_file_head_t));

    return 0;
}

/*
*******************************************************************************
*                     Parse_Pic_BMP
*
* Description:
*    ��·����������ͼƬ, ���Ұѽ��������ͼƬ������ָ���ĵ�ַ��
* ���ָ���ĵ�ַΪNULL, ����Դ�����κε�ַ��
*
* Parameters:
*    Path           :  input.  ͼƬ·��
*    PictureInfo    :  output. ͼƬ���������Ϣ
*    Addr           :  input.  ��Ž������ͼƬ,
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
int32_t Parse_Pic_BMP_ByPath(char *Path, Picture_t *PictureInfo, void *Addr)
{
    HBMP_i_t hbmp_buf;
    HBMP  hbmp = NULL;

    memset(&hbmp_buf, 0, sizeof(HBMP_i_t));
    hbmp = open_bmp(Path, &hbmp_buf);
    if (hbmp == NULL)
    {
        __err("ERR: open_bmp failed");
        return -1;
    }

    PictureInfo->BitCount = get_bitcount(hbmp);
    PictureInfo->Width    = get_width(hbmp);
    PictureInfo->Height   = get_height(hbmp);
    PictureInfo->RowSize  = get_rowsize(hbmp);

    PictureInfo->BufferSize = PictureInfo->RowSize * PictureInfo->Height;
    if (Addr)
    {
        PictureInfo->Buffer = (void *)Addr;
    }
    else
    {
        PictureInfo->Buffer = (void *)rt_malloc(PictureInfo->BufferSize);
    }
    if (PictureInfo->Buffer == NULL)
    {
        __log("ERR: wboot_malloc failed");
        goto error;
    }

    memset(PictureInfo->Buffer, 0, PictureInfo->BufferSize);

    get_matrix(hbmp, PictureInfo->Buffer);

    close_bmp(hbmp);
    hbmp = NULL;

    return 0;

error:
    close_bmp(hbmp);

    return -1;
}

#if defined CONFIG_BOOT_LOGO_BMP
int32_t Parse_Pic_BMP_ByRam(uint32_t base, uint32_t size, Picture_t *PictureInfo, void *Addr)
{
    HBMP_i_t hbmp_buf;
    HBMP  hbmp = NULL;

    memset(&hbmp_buf, 0, sizeof(HBMP_i_t));
    hbmp = open_ram_bmp(base, size, &hbmp_buf);
    if (hbmp == NULL)
    {
        __err("ERR: open_bmp failed");
        return -1;
    }

    PictureInfo->BitCount = get_bitcount(hbmp);
    PictureInfo->Width    = get_width(hbmp);
    PictureInfo->Height   = get_height(hbmp);
    PictureInfo->RowSize  = get_rowsize(hbmp);

    PictureInfo->BufferSize = PictureInfo->RowSize * PictureInfo->Height;
    if (Addr)
    {
        PictureInfo->Buffer = (void *)Addr;
    }
    else
    {
        PictureInfo->Buffer = (void *)rt_malloc(PictureInfo->BufferSize);
    }
    if (PictureInfo->Buffer == NULL)
    {
        __log("ERR: wboot_malloc failed");
        goto error;
    }

    memset(PictureInfo->Buffer, 0, PictureInfo->BufferSize);

    get_matrix(hbmp, PictureInfo->Buffer);

    close_bmp(hbmp);
    hbmp = NULL;

    return 0;

error:
    close_bmp(hbmp);

    return -1;
}
#endif
