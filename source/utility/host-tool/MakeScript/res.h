//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
//                                                Scott System                                              //
//                                                                                                          //
//                               (c) Copyright 2007-2008, Scottyu China                                     //
//                                                                                                          //
//                                           All Rights Reserved                                            //
//                                                                                                          //
// File    : res.h                                                                                          //
// By      : scottyu                                                                                        //
// Version : V1.00                                                                                          //
// Time    : 2008-07-14 14:51:58                                                                            //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
// HISTORY                                                                                                  //
//                                                                                                          //
// 1 2008-07-14 14:52:02                                                                                    //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//
#ifndef __LION_RES__H__
#define __LION_RES__H__     1

//????????????
#define RES_SEC_NAME_SIZE   8
#define RES_SEC_NAME        ".resthem"  //8????????
#define RES_VERSION     0x0100      //??????????????????
#define RES_ALIGN       32

//????????????
#define SYSRES          0
#define LIBRES          1
#define APPRES          2

//??????????????
typedef struct tag_RESHEAD
{
    s8  SecName[8];     //????
    u16 version;        //????0x0100
    u16 size;           //??????????????
    u32 StyleOffset;            //??????????????????
    u32 StyleNum;       //????????
    u32 ResNum;             //????????
    u32 align;          //????????????????
    u32 flags;          //??????SYSRES LIBRES  APPRES
} __attribute__((packed))RESHEAD;

#define RESHEAD_SIZE    sizeof(RESHEAD) //32 byte

#define STYLE_NAME_SIZE 18

typedef struct tag_STYLE
{
    u16 ID;         //????ID?? ????
    s8  Name[STYLE_NAME_SIZE];  //??????????ASCII??
    u32 ResTableOffset;         //????????????????????
    u32 ResOffset;      //??????????????????????
    u32 ResSize;        //????????????
} __attribute__((packed)) STYLE;

#define STYLE_SIZE  sizeof(STYLE)   //32 byte

//??????????????
#define RES_NAME_SIZE    20

typedef  struct tag_RESITEM
{
    u16 type;           //????????
    u16 ID;         //????ID,??????????????????ID????
    s8  Name[RES_NAME_SIZE];
    i   //??????????????????????
    u32 offset;             //??????????????res????????
    u32 size;           //??????size
} __attribute__((packed))RESITEM;

#define RESITEM_SIZE    sizeof(RESITEM) //32

#endif //__LION_RES__H__
