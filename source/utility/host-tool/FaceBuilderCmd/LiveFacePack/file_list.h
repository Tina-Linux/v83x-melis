//file_list.h
//2010-02-04 15:57:46

#ifndef __FILE__LIST___H___
#define __FILE__LIST___H___

#include "resource.h"

//------------------------------------------------------------------------------------------------------------
// ��Դ�ļ����ݽṹ
//------------------------------------------------------------------------------------------------------------

typedef unsigned int u32;

#pragma pack(1)
typedef struct tag_RESID_FILE
{
    u32     id;                     //��ԴID
    char    filename[MAX_PATH];     //�ļ���
} RESID_FILE_t;
#pragma pack()


//#define USBDRIVER_DIR             "UsbDriver"             //

//------------------------------------------------------------------------------------------------------------
// ��Դ�ļ�����
//------------------------------------------------------------------------------------------------------------
#define FILENAME_CONFIG_DLL            "config.dll"
#define FILENAME_CXIMAGECRT_DLL        "cximagecrt.dll"
#define FILENAME_CXIMAGECRTD_DLL       "cximagecrtd.dll"
#define FILENAME_ENGLISH_DLL           "English.dll"
#define FILENAME_FACE_DOCUMENT         "face.document"
#define FILENAME_FACEMAKER_DLL         "FaceMaker.dll"
#define FILENAME_FACE_BIN_WORK         "face_bin.work"
#define FILENAME_FACE_SCRIPT_WORK      "face_script.work"
#define FILENAME_IMAGEBUILDER_DLL      "ImageBuilder.dll"
#define FILENAME_IMGDECODE_DLL         "ImgDecode.dll"
#define FILENAME_LIVEFACE_EXE          "LiveFace.exe"
#define FILENAME_LIVEFACE_REG          "LiveFace.reg"
#define FILENAME_LIVEFACEGUIDE_PDF     "LiveFaceGuide.pdf"
#define FILENAME_RESACCESS_DLL         "ResAccess.dll"
#define FILENAME_SCRIPTPARSER_DLL      "ScriptParser.dll"
#define FILENAME_SINGLE_DLL            "Single.dll"



//------------------------------------------------------------------------------------------------------------
// ��Դ�ļ��б�
//------------------------------------------------------------------------------------------------------------
RESID_FILE_t resArray[] =
{
    {IDR_FILE_CONFIG_DLL        ,   FILENAME_CONFIG_DLL       },    //
    {IDR_FILE_CXIMAGECRT_DLL    ,   FILENAME_CXIMAGECRT_DLL   },    //
    {IDR_FILE_CXIMAGECRTD_DLL   ,   FILENAME_CXIMAGECRTD_DLL  },    //
    {IDR_FILE_ENGLISH_DLL       ,   FILENAME_ENGLISH_DLL      },    //
    {IDR_FILE_FACE_DOCUMENT     ,   FILENAME_FACE_DOCUMENT    },    //
    {IDR_FILE_FACEMAKER_DLL     ,   FILENAME_FACEMAKER_DLL    },    //
    {IDR_FILE_FACE_BIN_WORK     ,   FILENAME_FACE_BIN_WORK    },    //
    {IDR_FILE_FACE_SCRIPT_WORK  ,   FILENAME_FACE_SCRIPT_WORK },    //
    {IDR_FILE_IMAGEBUILDER_DLL  ,   FILENAME_IMAGEBUILDER_DLL },    //
    {IDR_FILE_IMGDECODE_DLL     ,   FILENAME_IMGDECODE_DLL    },    //
    {IDR_FILE_LIVEFACE_EXE      ,   FILENAME_LIVEFACE_EXE     },    //
    {IDR_FILE_LIVEFACE_REG      ,   FILENAME_LIVEFACE_REG     },    //
    {IDR_FILE_LIVEFACEGUIDE_PDF ,   FILENAME_LIVEFACEGUIDE_PDF},    //
    {IDR_FILE_RESACCESS_DLL     ,   FILENAME_RESACCESS_DLL    },    //
    {IDR_FILE_SCRIPTPARSER_DLL  ,   FILENAME_SCRIPTPARSER_DLL },    //
    {IDR_FILE_SINGLE_DLL        ,   FILENAME_SINGLE_DLL       }     //

};

#endif //__FILE__LIST___H___
