//------------------------------------------------------------------------------------------------------------
//
//2008-10-16 8:58:24
//
//scott
//
//config.cpp
//
//------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "inifile.h"
#define SCOTT_DEBUG

#include "basetypes.h"
#include "error.h"

#include "config_if.h"
#include "config_private.h"

//#define PLUGINAPI extern "C" __declspec(dllexport)
#include "config.h"

#define RAND_STRING         "!*&*%$#@!@#$$#$#$%^^*&^%$#@@*&!*"
#define DEFAULT_VALUE           0xffffffff
#define CFG_VERSION_100         0x00000100


#define TYPE_INI        0
#define TYPE_XML        1
#define TYPE_LUA        2

typedef struct tagCONFIG
{
    u32 version;                //������Ϣ�ṹ�İ汾
    u32 size;                   //���ṹ�Ĵ�С

    u32 type;                   //�����ļ�������
    s8  filename[MAX_PATH];         //�����ļ���·��
    u32 state;                  //�����ļ��ĵ�ǰ״̬
    u32 res;                    //����
} __attribute__((packed)) CONFIG;


/*
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
    return TRUE;
}
*/

/*
PLUGINAPI u32   QueryInterface  (const char * plug_guid)
{
    if (0 == strcmp(plug_guid, (const char *)CONFIG_PLUGIN_TYPE))
    {
        return OK;
    }
    else
    {
        return __LINE__;
    }

}
*/

void *GetConfigInterface()
{
    static CONFIG_IF __if;

    __if.OpenConfig         = OpenConfig;
    __if.CloseConfig        = CloseConfig;
    __if.ConfigDump         = ConfigDump;
    __if.GetSectionCount    = GetSectionCount;
    __if.GetFirstSection    = GetFirstSection;
    __if.GetNextSection = GetNextSection;
    __if.GetSectionKeyCount = GetSectionKeyCount;
    __if.GetSectionFirstKey = GetSectionFirstKey;
    __if.GetSectionNextKey  = GetSectionNextKey;
    __if.SetKeyValue        = SetKeyValue;
    __if.SetKeyString   = SetKeyString;
    __if.GetKeyValue        = GetKeyValue;
    __if.GetKeyString   = GetKeyString;
    __if.CreateSection  = CreateSection;
    __if.DeleteSection  = DeleteSection;
    __if.DeleteKey      = DeleteKey;
    return &__if;
}

//------------------------------------------------------------------------------------------------------------
// ����˵��
//     �������ļ����򿪳ɹ����ؾ��
//------------------------------------------------------------------------------------------------------------
HCONFIG OpenConfig(char *szFilePath, void *Param)
{
    CONFIG *config = NULL;
    config = (CONFIG *)malloc(sizeof(CONFIG));
    memset(config, 0, sizeof(CONFIG));

    config->version = CFG_VERSION_100;
    config->size    = sizeof(CONFIG);

    //����ʵ������Ҫ�������ļ��ĺ�׺���з������У�����param����˵��
    config->type    = TYPE_INI;
    strcpy((char *)config->filename, szFilePath);
    config->state   = 0;
    config->res = 0;

    return config;
}

u32 CloseConfig(HCONFIG hcfg)
{
    if (NULL == hcfg)
        return __LINE__;
    //�ͷž���ռ�
    free(hcfg);
    hcfg = NULL;
    return OK;
}

u32 GetSectionCount(HCONFIG hcfg, u32 *pSectionCount)
{
//  GetPrivateProfileSectionNames - �� ini �ļ��л�� Section ������
//  ��� ini �������� Section: [sec1] �� [sec2]���򷵻ص��� 'sec1',0,'sec2',0,0 ��

    CONFIG *pconfig = NULL;
    int iPos = 0;
    u32 i;
    u32 SectionCount = 0;
    TCHAR chSectionNames[MAX_ALLSECTIONS] = {0}; //�ܵ���������ַ���
    TCHAR chSection[MAX_SECTION] = {0};     //���һ��������
    DWORD dwLen = 0;
    u32 iMaxCount = 0;

    *pSectionCount = 0;
    if (NULL == hcfg)
        return __LINE__;

    pconfig = (CONFIG *) hcfg;
    memset(chSectionNames, 0, sizeof(TCHAR) * MAX_ALLSECTIONS);
    dwLen = GetPrivateProfileSectionNames(chSectionNames,
                                          sizeof(TCHAR) * MAX_ALLSECTIONS,
                                          (const char *)pconfig->filename);
    if (dwLen == 0)
        return __LINE__;

    if (dwLen < (MAX_ALLSECTIONS - 2))
    {
    }
    else if (dwLen == (MAX_ALLSECTIONS - 2))
    {
        printf("Section ���Ƴ��ȳ�����Χ!");
        return __LINE__;
    }
    else
    {
        printf("Section ���Ƴ��ȳ�����Χ!");
        return __LINE__;
    }

    //����ѭ�����ضϵ�����������0
    for (i = 0; i < dwLen; i++)
    {
        if (chSectionNames[i] == 0)
        {
            if (chSectionNames[i] == chSectionNames[i + 1])
                break;
        }
    }

    //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ������֡�
    iMaxCount = i + 1;
    iPos = 0;
    for (i = 0; i < iMaxCount; i++)
    {
        chSection[iPos++] = chSectionNames[i];
        if (chSectionNames[i] == 0)
        {
            SectionCount++;
            memset(chSection, 0, MAX_SECTION);
            iPos = 0;
        }
    }

    *pSectionCount = SectionCount;
    return OK;
}

POS GetFirstSection(HCONFIG hcfg)
{
    TCHAR chSectionNames[MAX_ALLSECTIONS] = {0}; //�ܵ���������ַ���
    TCHAR chSection[MAX_SECTION] = {0};    //���һ��������
    CONFIG *pconfig = NULL;
    POS_t *pos = NULL;
    int iPos = 0, i, iMaxCount;
    DWORD dwLen = 0;
    int len = 0;

    if (NULL == hcfg)
    {
        Err("GetFirstSection", __FILE__, __LINE__, "hcfg invalid");
        return NULL;
    }

    pconfig = (CONFIG *) hcfg;
    pos = (POS_t *)malloc(sizeof(POS_t));
    memset(pos, 0, sizeof(POS_t));

    pos->count  = 0;
    dwLen = GetPrivateProfileSectionNames(chSectionNames,
                                          MAX_ALLSECTIONS,
                                          (const char *)pconfig->filename);
    if (dwLen < (MAX_ALLSECTIONS - 2))
    {
    }
    else if (dwLen == (MAX_ALLSECTIONS - 2))
    {
        printf("Section ���Ƴ��ȳ�����Χ!");
        return NULL;
    }
    else
    {
        printf("Section ���Ƴ��ȳ�����Χ!");
        return NULL;
    }

    //����ѭ�����ضϵ�����������0
    for (i = 0; i < MAX_ALLSECTIONS; i++)
    {
        if (chSectionNames[i] == 0)
        {
            if (chSectionNames[i] == chSectionNames[i + 1])
                break;
        }
    }
    //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ�������
    iMaxCount = i + 1;
    iPos = 0;
    for (i = 0; i < iMaxCount; i++)
    {
        chSection[iPos++] = chSectionNames[i];
        if (chSectionNames[i] == 0)
        {
            len = strlen(chSection);
            if (len > 0)
            {
                ListItem *item = (ListItem *)malloc(sizeof(ListItem));
                item->String = (s8 *)malloc(len + 1);
                memset(item->String, 0, (len + 1));
                strcpy((char *)item->String, chSection);
                item->Next = NULL;

                AddItem(pos, item);
            }
            memset(chSection, 0, MAX_SECTION);
            iPos = 0;
        }
    }
    return pos;
}

POS GetNextSection(HCONFIG hcfg, POS pos, s8 *szSection)
{
    POS_t *post = NULL;

    if (NULL == hcfg || NULL == pos || NULL == szSection)
    {
        Err("GetNextSection", __FILE__, __LINE__, "!hcfg || !pos || !szSection\n");
        return NULL;
    }

    post = (POS_t *)pos;
    if (post->count == 0)
    {
        Err("GetNextSection", __FILE__, __LINE__, "post->count == 0\n");
        return NULL;
    }

    //����ĩβ��Ҫ�ͷſռ�
    if (post->count == post->index)
    {
        Delete(post);
        Err("GetNextSection", __FILE__, __LINE__, "post->count == post->index\n");
        return NULL;
    }

    if (post->current)
    {
        strcpy((char *)szSection, (const char *)post->current->String);
        post->current = post->current->Next;
        post->index++;
        return pos;
    }
    return pos;
}

u32 GetSectionKeyCount(HCONFIG hcfg, s8 *szSection, u32 *pKeyCount)
{
//  GetPrivateProfileSection �� ini �ļ��л��һ��Section��ȫ��������ֵ��
//  ���ini����һ���Σ������� "��1=ֵ1" "��2=ֵ2"���򷵻ص��� '��1=ֵ1',0,'��2=ֵ2',0,0 �����㲻֪��
//  ���һ�����е����м���ֵ�����������

    CONFIG *pconfig = NULL;
    int iPos = 0, i, iMaxCount;
    u32   KeyCount  = 0;
    TCHAR chKeyNames[MAX_ALLKEYS] = {0}; //�ܵ���������ַ���
    TCHAR chKey[MAX_KEY]          = {0}; //�������һ������
    DWORD dwLen = 0;

    *pKeyCount = 0;
    if (NULL == hcfg)
        return __LINE__;

    pconfig = (CONFIG *) hcfg;
    dwLen = GetPrivateProfileSection((const char *)szSection,
                                     chKeyNames,
                                     MAX_ALLKEYS,
                                     (const char *)pconfig->filename);
    if (dwLen < (MAX_ALLKEYS - 2))
    {
    }
    else if (dwLen == (MAX_ALLKEYS - 2))
    {
        printf("Key ���Ƴ��ȳ�����Χ!");
        return __LINE__;
    }
    else
    {
        printf("Key ���Ƴ��ȳ�����Χ!");
        return __LINE__;
    }

    //����ѭ�����ضϵ�����������0
    for (i = 0; i < MAX_ALLKEYS; i++)
    {
        if (chKeyNames[i] == 0)
        {
            if (chKeyNames[i] == chKeyNames[i + 1])
            {
                break;
            }
        }
    }
    //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ������֡�
    iMaxCount = i + 1;

    for (i = 0; i < iMaxCount; i++)
    {
        chKey[iPos++] = chKeyNames[i];
        if (chKeyNames[i] == 0)
        {
            memset(chKey, 0, MAX_KEY);
            iPos = 0;
            KeyCount++;
        }
    }

    *pKeyCount = KeyCount;
    return OK;
}

POS GetSectionFirstKey(HCONFIG hcfg, s8 *szSection)
{
    CONFIG *pconfig = NULL;
    POS_t *pos = NULL;
    int iPos = 0, i = 0, iMaxCount = 0;
    TCHAR chKeyNames[MAX_ALLKEYS] = {0}; //�ܵ���������ַ���
    TCHAR chKey[MAX_KEY]          = {0}; //�������һ������
    DWORD dwLen = 0;

    if (NULL == hcfg || NULL == szSection)
    {
        return NULL;
    }

    if (strlen((const char *)szSection) == 0)
    {
        return NULL;
    }
    /*
    if (strlen(szSection) > 15)
        return NULL;
    */

    pconfig = (CONFIG *) hcfg;
    pos = (POS_t *)malloc(sizeof(POS_t));
    memset(pos, 0, sizeof(POS_t));

    dwLen = GetPrivateProfileSection((const char *)szSection,
                                     chKeyNames,
                                     MAX_ALLKEYS,
                                     (const char *)pconfig->filename);
    if (dwLen < (MAX_ALLKEYS - 2))
    {
    }
    else if (dwLen == (MAX_ALLKEYS - 2))
    {
        printf("Key ���Ƴ��ȳ�����Χ!");
        return NULL;
    }
    else
    {
        printf("Key ���Ƴ��ȳ�����Χ!");
        return NULL;
    }

    //����ѭ�����ضϵ�����������0
    for (i = 0; i < MAX_ALLKEYS; i++)
    {
        if (chKeyNames[i] == 0)
        {
            if (chKeyNames[i] == chKeyNames[i + 1])
            {
                break;
            }
        }
    }

    //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ�������
    iMaxCount = i + 1;

    for (i = 0; i < iMaxCount; i++)
    {
        chKey[iPos++] = chKeyNames[i];
        if (chKeyNames[i] == 0)
        {
            char key[MAX_KEY];
            memset(key, 0, MAX_KEY);
            {
                char *pdest;
                int result;
                char ch = '=';
                pdest = strchr(chKey, ch);
                result = pdest - chKey;
                if (pdest != NULL)
                {
                    strncpy(key, chKey, result);
                    Msg("Result:\t %c found at[%d] %s=%s\n", ch, result, key);
                }
                else
                {
                    Err("GetSectionFirstKey", __FILE__, __LINE__, "Result:\t = not found");
                    memset(key, 0, MAX_KEY);
                }
            }

            int len = strlen(key);
            if (len > 0)
            {
                ListItem *item = (ListItem *)malloc(sizeof(ListItem));
                item->String = (s8 *)malloc(len + 1);
                memset(item->String, 0, len + 1);
                strcpy((char *)item->String, key);
                item->Next = NULL;
                AddItem(pos, item);
            }
            memset(chKey, 0, MAX_SECTION);
            iPos = 0;
        }
    }
    return pos;
}

POS GetSectionNextKey(HCONFIG hcfg, s8 *szSection, POS pos, s8 *szKey)
{
    POS_t *post = NULL;

    if (NULL == hcfg || NULL == pos || NULL == szSection)
    {
        Err("GetSectionNextKey", __FILE__, __LINE__, " error 1");
        return NULL;
    }

    post = (POS_t *)pos;
    if (post->count == 0)
    {
        Err("GetSectionNextKey", __FILE__, __LINE__, " error 2");
        return NULL;
    }

    //����ĩβ��Ҫ�ͷſռ�
    if (post->count == post->index)
    {
        Delete(post);
        Err("GetSectionNextKey", __FILE__, __LINE__, "delete all");
        return NULL;
    }

    if (post->current)
    {
        strcpy((char *)szKey, (const char *)post->current->String);
        post->current = post->current->Next;
        post->index++;
        return pos;
    }
    return pos;
}

/*
 * ����˵��
 *   ���ü�ֵ��bCreate��ָ����������δ����ʱ���Ƿ񴴽�
 */
u32 SetKeyString(HCONFIG hcfg, s8 *szSection, s8 *szKey, s8 *String, u32 bCreate)
{
    CONFIG *pconfig = NULL;

    if (NULL == hcfg)
    {
        return __LINE__;
    }
    pconfig = (CONFIG *) hcfg;

#ifdef CHECK_KEY
    //������key
    DWORD dwLen = 0;
    if (!bCreate)
    {
        dwLen = GetPrivateProfileString((const char *)szSection,
                                        (const char *)szKey,
                                        RAND_STRING,
                                        lpTemp,
                                        MAX_PATH,
                                        (const char *)pconfig->filename);
        if (0 == dwLen)
            return __LINE__;

        if (lstrcmp(lpTemp, RAND_STRING) == 0)
        {
            return __LINE__;
        }
    }
#endif //CHECK_KEY
    //write key string
    if (!WritePrivateProfileString((const char *)szSection,
                                   (const char *)szKey,
                                   (const char *)String,
                                   (const char *)pconfig->filename))
    {
        return __LINE__;
    }
    return OK;
}

u32 SetKeyValue(HCONFIG hcfg, s8 *szSection, s8 *szKey, u32 Value, u32 bCreate)
{
    CONFIG *pconfig = NULL;
    TCHAR String[MAX_VALUE] = {0};

    if (NULL == hcfg)
    {
        return __LINE__;
    }
    pconfig = (CONFIG *) hcfg;

#ifdef CHECK_KEY
    //������key
    DWORD dwLen = 0;
    if (!bCreate)
    {
        dwLen = GetPrivateProfileString((const char *)szSection,
                                        (const char *)szKey,
                                        RAND_STRING,
                                        lpTemp,
                                        MAX_PATH,
                                        (const char *)pconfig->filename);
        if (0 == dwLen)
            return __LINE__;

        if (lstrcmp(lpTemp, RAND_STRING) == 0)
        {
            return __LINE__;
        }
    }
#endif //CHECK_KEY
    memset(String, 0, sizeof(TCHAR) * MAX_VALUE);
    sprintf(String, "%d", Value);
    //write key string
    if (!WritePrivateProfileString((const char *)szSection,
                                   (const char *)szKey,
                                   (const char *)String,
                                   (const char *)pconfig->filename))
    {
        return __LINE__;
    }

    return OK;
}

u32 GetKeyValue(HCONFIG hcfg, s8 *szSection, s8 *szKey, u32 *pValue)
{
    CONFIG *pconfig = NULL;
    DWORD dValue = DEFAULT_VALUE;

    if (NULL == hcfg || NULL == szSection || NULL == szKey || NULL == pValue)
    {
        return __LINE__;
    }

    pconfig = (CONFIG *) hcfg;
    dValue =  GetPrivateProfileInt((const char *)szSection,
                                   (const char *)szKey,
                                   DEFAULT_VALUE,
                                   (const char *)pconfig->filename);
    if (dValue == DEFAULT_VALUE)
    {
        return __LINE__;
    }

    *pValue =  dValue;
    return OK;
}

u32 GetKeyString(HCONFIG hcfg, s8 *szSection, s8 *szKey, s8 *String)
{
    CONFIG *pconfig = NULL;
    TCHAR lpValue[MAX_PATH] = {0};
    memset(lpValue, 0, sizeof(TCHAR) * MAX_PATH);

    if (NULL == hcfg || NULL == szSection || NULL == szKey || NULL == String)
    {
        return __LINE__;
    }
    //key���ƻ�Section���Ƴ��ȳ�����󳤶�15�ֽڣ�ϵͳ���ܴ���

    pconfig = (CONFIG *) hcfg;
    GetPrivateProfileString((const char *)szSection,
                            (const char *)szKey,
                            RAND_STRING,
                            lpValue,
                            sizeof(TCHAR) * MAX_PATH,
                            (const char *)pconfig->filename);
    if (strcmp(lpValue, RAND_STRING) == 0)
    {
        Msg("GetPrivateProfileString failed !\n");
        return __LINE__;
    }

    strcpy((char *)String, (const char *)lpValue);
    return OK;
}

u32 CreateSection(HCONFIG hcfg, s8 *szSection)
{
    CONFIG *pconfig = NULL;

    if (NULL == hcfg || NULL == szSection)
    {
        return __LINE__;
    }

    pconfig = (CONFIG *) hcfg;
    // create new section
    if (!WritePrivateProfileSection((const char *)szSection,
                                    ";author=scott", //note
                                    (const char *)pconfig->filename))
    {
        return __LINE__;
    }
    return OK;
}

u32 DeleteSection(HCONFIG hcfg, s8 *szSection)
{
    CONFIG *pconfig = NULL;

    if (NULL == hcfg || NULL == szSection)
    {
        return __LINE__;
    }

    pconfig = (CONFIG *) hcfg;
    // delete section
    if (WritePrivateProfileString((const char *)szSection,
                                  NULL, //note
                                  NULL, //note
                                  (const char *)pconfig->filename))
    {
        return __LINE__;
    }
    return OK;
}

u32 DeleteKey(HCONFIG hcfg, s8 *szSection, s8 *szKey)
{
    CONFIG *pconfig = NULL;

    if (NULL == hcfg || NULL == szSection || NULL == szKey)
    {
        return __LINE__;
    }

    pconfig = (CONFIG *) hcfg;
    // delete key
    if (WritePrivateProfileString((const char *)szSection,
                                  (const char *)szKey,
                                  NULL, //note
                                  (const char *)pconfig->filename))
    {
        return __LINE__;
    }

    return OK;
}

u32 ConfigDump(HCONFIG hcfg)
{
    s8 key[MAX_KEY];
    s8 keyString[MAX_KEY];
    s8 SectionName[MAX_SECTION];
    u32 SectionCount = 0;
    u32 KeyCount = 0;
    POS pos = NULL;
    POS kpos = NULL;
    u32 ret = OK;

    memset(SectionName, 0, MAX_SECTION);
    ret = GetSectionCount(hcfg, &SectionCount);
    if (OK != ret)
    {
        Err("ConfigDump", __FILE__, __LINE__, "GetSectionCount failed ! %d", ret);
        return __LINE__;
    }
    //Msg("\nSectionCount=%d\n", SectionCount);

    pos = GetFirstSection(hcfg);
    if (NULL == pos)
    {
        Err("ConfigDump", __FILE__, __LINE__, "GetFirstSection failed !");
        return __LINE__;
    }

    for (u32 i = 0; i < SectionCount && pos != NULL; i++)
    {
        memset(SectionName, 0, MAX_SECTION);
        pos = GetNextSection(hcfg, pos, (s8 *)SectionName);
        if (pos == NULL)
        {
            Err("ConfigDump", __FILE__, __LINE__, "GetNextSection(%d) failed !", i);
            return __LINE__;
        }
        Msg("\n%d[%s]\n", i, SectionName);

        ret = GetSectionKeyCount(hcfg, (s8 *)SectionName, &KeyCount);
        if (OK != ret)
        {
            Err("ConfigDump", __FILE__, __LINE__, "GetSectionKeyCount(%d) failed !", ret);
            return __LINE__;
        }

        Msg("KeyCount=%d\n", KeyCount);
        kpos = GetSectionFirstKey(hcfg, SectionName);
        for (u32 j = 0; j < KeyCount; j++)
        {
            memset(key, 0, MAX_KEY);
            memset(keyString, 0, MAX_KEY);
            kpos = GetSectionNextKey(hcfg, SectionName, kpos, key);
            if (OK == GetKeyString(hcfg, SectionName, key, keyString))
            {
                Msg("[%d]%s=%s\n", j, key, keyString);
            }
            else
            {
                return __LINE__;
            }
        }
        memset(SectionName, 0, MAX_SECTION);
    }

    return OK;
}
