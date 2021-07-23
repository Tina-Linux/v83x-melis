/*
*************************************************************************************
*                                   eGon
*                           Application Of eGon2.0
*
*               (c) Copyright 2006-2010, All winners Co,Ld.
*                           All Rights Reserved
*
* File Name     : Parse_Picture.h
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
#ifndef  __PARSE_PICTURE_H__
#define  __PARSE_PICTURE_H__
#include <typedef.h>

typedef struct tag_Picture
{
    void *Buffer;           /* ���ͼƬ����     */
    uint32_t BufferSize;    /* buffer����       */

    uint32_t BitCount;      /* һ�����ص�bit��  */
    uint32_t Width;         /* ͼƬ���         */
    uint32_t Height;        /* ͼƬ�߶�         */
    uint32_t RowSize;       /* ͼƬһ�еĴ�С   */
} Picture_t;

int32_t Parse_Pic_BMP_ByBuffer(void *Pic_Buffer, uint32_t Pic_BufferSize, Picture_t *PictureInfo);
int32_t Parse_Pic_BMP_ByPath(char *Path, Picture_t *PictureInfo, void *Addr);
int32_t Parse_Pic_BMP_ByRam(const uint32_t base, const uint32_t size, Picture_t *PictureInfo, void *Addr);

#endif   //__PARSE_PICTURE_H__

