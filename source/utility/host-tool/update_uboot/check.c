/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*                                                  All Rights Reserved
*
* File Name : check.c
* Author : Gary.Wang
* Version : 1.1.0
*
* Date : 2007.10.12
* Description : This file provides a function to check Boot0 and Boot1.
*
* Others : None at present.
*
* History :
*  <Author>        <time>       <version>      <description>
* Gary.Wang       2007.10.12      1.1.0        build the file
*
************************************************************************************************************************
*/
#include "check.h"
#include "spare_head.h"
#include <string.h>

/********************************************************************************
*��������: check_magic
*����ԭ��: __s32 check_magic( __u32 *mem_base, const char *magic )
*��������: ʹ�á������͡���У���ڴ��е�һ������
*��ڲ���: mem_base       Boot�ļ����ڴ��е���ʼ��ַ
*          magic          Boot��magic
*�� �� ֵ: CHECK_IS_CORRECT      У����ȷ
*          CHECK_IS_WRONG        У�����
*��    ע:
********************************************************************************/
__s32 check_magic(__u32 *mem_base, const char *magic)
{
    struct spare_boot_ctrl_head  *bfh;
    unsigned char *p;

    bfh = (struct spare_boot_ctrl_head *)mem_base;
    p = bfh->magic;
    if (strcmp((const char *)p, magic))
    {
        return CHECK_IS_WRONG;
    }
    return CHECK_IS_CORRECT;
}

/********************************************************************************
*��������: check_sum
*����ԭ��: __s32 check_sum( __u32 *mem_base, __u32 size, const char *magic )
*��������: ʹ�á������͡���У���ڴ��е�һ������
*��ڲ���: mem_base           ��У����������ڴ��е���ʼ��ַ��������4�ֽڶ���ģ�
*          size               ��У������ݵĸ��������ֽ�Ϊ��λ��������4�ֽڶ���ģ�
*�� �� ֵ: CHECK_IS_CORRECT   У����ȷ
*          CHECK_IS_WRONG     У�����
*��    ע:
********************************************************************************/
__s32 check_sum(__u32 *mem_base, __u32 size)
{
    __u32 *buf;
    __u32 count;
    __u32 src_sum;
    __u32 sum;
    struct spare_boot_ctrl_head  *bfh;

    bfh = (struct spare_boot_ctrl_head *)mem_base;
    /* ����У��� */
    // ��Boot_file_head�еġ�check_sum���ֶ�ȡ��У���
    src_sum = bfh->check_sum;

    // ��STAMP_VALUEд��Boot_file_head�еġ�check_sum���ֶ�
    bfh->check_sum = STAMP_VALUE;

    // �� �֣�4bytes��Ϊ��λ����
    count = size >> 2;
    sum = 0;
    buf = (__u32 *)mem_base;

    // �����ۼӣ����У���
    do
    {
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
        sum += *buf++;
    }
    while ((count -= 4) > (4 - 1));

    while (count-- > 0)
        sum += *buf++;

    // �ָ�Boot_file_head�еġ�check_sum���ֶε�ֵ
    bfh->check_sum = src_sum;
    if (sum == src_sum)
        return CHECK_IS_CORRECT;
    else
        return CHECK_IS_WRONG;
}

/********************************************************************************
*��������: check_file
*����ԭ��: __s32 check_file( __u32 *mem_base, __u32 size, const char *magic )
*��������: ʹ�á������͡���У���ڴ��е�һ������
*��ڲ���: mem_base       ��У����������ڴ��е���ʼ��ַ��������4�ֽڶ���ģ�
*          size           ��У������ݵĸ��������ֽ�Ϊ��λ��������4�ֽڶ���ģ�
*          magic          magic number, ��У���ļ��ı�ʶ��
*�� �� ֵ: CHECK_IS_CORRECT       У����ȷ
*          CHECK_IS_WRONG         У�����
*��    ע:
********************************************************************************/
__s32 check_file(__u32 *mem_base, __u32 size, const char *magic)
{
    if (check_magic(mem_base, magic) == CHECK_IS_CORRECT
            && check_sum(mem_base, size) == CHECK_IS_CORRECT)
        return CHECK_IS_CORRECT;
    else
        return CHECK_IS_WRONG;
}

__s32 gen_check_sum(void *boot_buf)
{
    struct spare_boot_head_t  *head_p;
    __u32 length;
    __u32 *buf;
    __u32 loop;
    __u32 i;
    __u32 sum;

    head_p = (struct spare_boot_head_t *)boot_buf;
    length = head_p->boot_head.length;

    // must 4-byte-aligned
    if ((length & 0x3) != 0)
        return -1;
    buf = (__u32 *)boot_buf;
    head_p->boot_head.check_sum = STAMP_VALUE;
    loop = length >> 2;

    /* ���㵱ǰ�ļ����ݵġ�У��͡�*/
    for (i = 0, sum = 0;  i < loop;  i++)
        sum += buf[i];

    /* write back check sum */
    head_p->boot_head.check_sum = sum;

    return 0;
}
