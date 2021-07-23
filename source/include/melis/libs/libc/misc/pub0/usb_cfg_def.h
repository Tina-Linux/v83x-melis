/*
**********************************************************************************************************************
*                                                    ePDK
*                                    the Easy Portable/Player Develop Kits
*                                              eLIBs Sub-System
*
*                                   (c) Copyright 2008-2009, Jerry.Wang China
*                                             All Rights Reserved
*
* Moudle  : eLibs board config module
* File    : usb_cfg_def.h
* By      : Jerry
* Version : v1.0
* Date    : 2009-10-19 15:52
**********************************************************************************************************************
*/
#ifndef  _XXXX_BOARD_USB_CFG_DEF_H_
#define  _XXXX_BOARD_USB_CFG_DEF_H_

/*

datax
|--------------------------------------------------------
|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
|--------------------------------------------------------
|    id_piog      |            id_no            |
|                 |                             |
|--------------------------------------------------------

|--------------------------------------------------------
|  15  |  14  |  13  |  12  |  11  |  10  |  9  |  8  |
|--------------------------------------------------------
|    vbus_piog       |            vbus_no             |
|                    |                                |
|--------------------------------------------------------

|--------------------------------------------------------
|  23  |  22  |  21  |  20  |  19  |  18  |  17  |  16  |
|--------------------------------------------------------
|    drv_vbus_piog   |            drv_vbus_no           |
|                    |                                  |
|--------------------------------------------------------

|--------------------------------------------------------
|  31  |  30  |  29  |  28  |  27  |  26  |  25  |  24  |
|--------------------------------------------------------
|      ����          |  det | port_type   | port        |
|                    |      |             |             |
|--------------------------------------------------------

bit4~0      :   id PIO �š�             ע: power(189): pin41 : 0, pin42 : 1, pin42 : 2, pin42 : 3,
bit7~5      :   id PIO ��š�           0: ������û��ʹ��PIO;    1: PIOA;    2: PIOB;    3: PIOC;
                                        4: PIOD;    5: PIOE;    6: PIOF;    7:  power(189)
��:
1��USB ID pin Ϊ PIOB 10�� ������Ϊ: bit0~7 = 0x4a.
2��USB ID pin Ϊ power(189)��pin42�� ������Ϊ: bit0~7 = 0xe1.

bit15~8     :   ��bit7~0������ͬ
bit23~16    :   ��bit7~0������ͬ

bit25~24    :   usb�˿ںš�             0: port��Ч;    1: port0;       2: port1;   else: port��Ч
bit27~26    :   usb�˿����� ��          0: device only; 1: host only;   2: otg;     else: device only
bit28       :   usb��ⷽʽ��           0: vbus/id���; 1: dp/dm���
bit31~29    :   ����

*/

#if (EPDK_ARCH == EPDK_ARCH_SUNI)
#define  BOARD_USBC_MAX_CTL_NUM    1
#elif (EPDK_ARCH == EPDK_ARCH_SUNII)
#define  BOARD_USBC_MAX_CTL_NUM    2
#elif (EPDK_ARCH == EPDK_ARCH_SUNIII)
#define  BOARD_USBC_MAX_CTL_NUM    3
#elif (EPDK_ARCH == EPDK_ARCH_SUNIV)
#define  BOARD_USBC_MAX_CTL_NUM    1

#endif

enum _usb_pio_group_type
{
    GROUP_TYPE_PIO = 0,
    GROUP_TYPE_P1
};

enum usb_port_type
{
    USB_PORT_TYPE_DEVICE = 0,
    USB_PORT_TYPE_HOST,
    USB_PORT_TYPE_OTG
};

enum usb_detect_type
{
    USB_DETECT_TYPE_VBUS_ID = 0,
    USB_DETECT_TYPE_DP_DM
};

enum usb_controller
{
    USB_CONTROLLER_NONE = 0,
    USB_CONTROLLER_0,
    USB_CONTROLLER_1,
    USB_CONTROLLER_ALL
};

/* pio��Ϣ */
typedef struct tag_borad_pio
{
    __u32 valid;            /* pio�Ƿ���á� 0:��Ч, !0:��Ч    */

    __u32 group_type;       /* pio����                          */
    __u32 group;            /* pio���                          */
    __u32 no;               /* pio pin��                        */
}
board_pio_t;

/* usb port ��Ϣ */
typedef struct tag_borad_usb_port
{
    __u32 valid;            /* port�Ƿ���á� 0:��Ч, !0:��Ч       */

    __u32 port;             /* usb�˿ں�                            */
    /* 0: port0;    1: port1;   else: port��Ч */
    __u32 port_type;            /* usb�˿����͡�                        */
    /* 0: device only; 1: host only; 2: otg */
    __u32 detect_type;      /* usb��ⷽʽ��                        */
    /* 0: vbus/id���;  1: dp/dm���        */

    board_pio_t id;         /* usb id pin��Ϣ                       */
    board_pio_t vbus;       /* usb vbus pin��Ϣ                     */
    board_pio_t drv_vbus;   /* usb drv_vbus pin��Ϣ                 */
} board_usb_port_t;

#endif  /*_XXXX_BOARD_USB_CFG_DEF_H_*/

