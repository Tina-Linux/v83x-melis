//------------------------------------------------------------------------------------------------------------
//
// PlugIn.h
//
// 2009-05-26 14:20:17
//
// scott
//
//------------------------------------------------------------------------------------------------------------

#ifndef ___PLUGIN___H_____
#define ___PLUGIN___H_____

//------------------------------------------------------------------------------------------------------------
// ����ӿ�
//------------------------------------------------------------------------------------------------------------

#include "if/config_if.h"                   //�ӿ�
#include "if/resaccess_if.h"                    //�ӿ�
#include "if/scriptparser_if.h"             //�ӿ�
#include "if/facemaker_if.h"                    //�ӿ�
#include "if/imagebuilder_if.h"             //�ӿ�
#include "if/ImgDecode_if.h"                    //�ӿ�

#define CONFIG_PLUG         "config.dll"        //���
#define RESACCESS_PLUG      "resaccess.dll"     //���
#define SCRIPTPARSER_PLUG   "scriptparser.dll"  //���
#define FACEMAKER_PLUG      "facemaker.dll"     //���
#define IMGBUILDER_PLUG     "ImageBuilder.dll"  //���
#define IMGDECODE_PLUG      "imgdecode.dll"     //���

typedef struct tag_PLUG_INFO
{
    char *plug_guid;    //���ID
    char *plug_name;    //����ļ�
} PLUG_INFO_t;

#ifndef GUID_LEN
#define GUID_LEN    36
#endif //GUID_LEN
#endif //___PLUGIN___H_____
