/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*                                                  All Rights Reserved
*
* File Name   : boo1.h
*
* Author      : Gary.Wang
*
* Version     : 1.1.0
*
* Date        : 2009.05.21
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2009.05.21       1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef  __boo1_h
#define  __boo1_h


#include "egon_def.h"


#define BOOT1_MAGIC                     "eGON.BT1"
#define MAGIC_FLAG                      "PHOENIX_CARD_IMG"


#define BOOT_SCRIPT_BUFFER_SIZE           (32 * 1024)
//����洢����
#define   START_FROM_NAND_FLASH       0
#define   START_FROM_SDCARD           1
#define   START_FROM_NOR_FLASH        2


//SD��������ݽṹ
typedef struct _boot_sdcard_info_t
{
    __s32               card_ctrl_num;                //�ܹ��Ŀ��ĸ���
    __s32               boot_offset;                  //ָ��������֮���߼�����������Ĺ���
    __s32               card_no[4];                   //��ǰ�����Ŀ���, 16-31:GPIO��ţ�0-15:ʵ�ʿ����������
    __s32               speed_mode[4];                //�����ٶ�ģʽ��0�����٣�����������
    __s32               line_sel[4];                  //�������ƣ�0: 1�ߣ�������4��
    __s32               line_count[4];                //��ʹ���ߵĸ���
}
boot_sdcard_info_t;


typedef struct _boot_core_para_t
{
    __u32  user_set_clock;                 // ����Ƶ�� M��λ
    __u32  user_set_core_vol;              // ���ĵ�ѹ mV��λ
    __u32  vol_threshold;                  // �������޵�ѹ
}
boot_core_para_t;
/******************************************************************************/
/*                              file head of Boot1                            */
/******************************************************************************/

typedef struct __BOOT_DRAM_PARA
{
    __u32 base;
    __u32 size;
    __u32 clk;
    __u32 access_mode;
    __u32 cs_num;
    __u32 ddr8_remap;
    __u32 sdr_ddr;
    __u32 bwidth;
    __u32 col_width;
    __u32 row_width;
    __u32 bank_size;
    __u32 cas;
} boot_dram_para_t;


typedef struct _boot1_private_head_t
{
    __u32              prvt_head_size;
    __u8               prvt_head_vsn[4];                // the version of Boot1_private_hea
    __s32                       uart_port;              // UART���������
    normal_gpio_cfg             uart_ctrl[2];           // UART������(���Դ�ӡ��)GPIO��Ϣ
    boot_dram_para_t            dram_para;              // dram init para
    char                        script_buf[32 * 1024];  // �ű�����
    boot_core_para_t            core_para;              // �ؼ�����
    __s32                       twi_port;               // TWI���������
    normal_gpio_cfg             twi_ctrl[2];            // TWI������GPIO��Ϣ�����ڿ���TWI
    __s32                       debug_enable;           // debugʹ�ܲ���
    __s32                       debug_log;           // debugʹ�ܲ���
    __s32                       hold_key_min;           // hold key��Сֵ
    __s32                       hold_key_max;           // hold key���ֵ
    __u32                       work_mode;              // ģʽ������������������
    __u32                       storage_type;           // �洢��������  0��nand   1��sdcard    2: spinor
    normal_gpio_cfg             storage_gpio[32];       // �洢�豸 GPIO��Ϣ
    char                        storage_data[512 - sizeof(normal_gpio_cfg) * 32];      // �û�����������Ϣ
    //boot_nand_connect_info_t    nand_connect_info;    // nand ����
} boot1_private_head_t;

typedef struct _boot1_file_head_t
{
    boot_file_head_t      boot_head;
    boot1_private_head_t  prvt_head;
} boot1_file_head_t;


#endif     //  ifndef __boo1_h

/* end of boo1.h */
