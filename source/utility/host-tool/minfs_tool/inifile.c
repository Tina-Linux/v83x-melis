#include "inifile.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

UINT
GetPrivateProfileInt(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT nDefault,
    LPCSTR lpFileName
)
{
    char lpReturnedString[100] = {0};
    DWORD nRet = GetPrivateProfileString(
                     lpAppName,
                     lpKeyName,
                     "",
                     lpReturnedString,
                     sizeof(lpReturnedString),
                     lpFileName
                 );
    if (nRet == 0)
        return nDefault;
    return atoi(lpReturnedString);
}

BOOL
WritePrivateProfileString(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString,
    LPCSTR lpFileName
)
{
    FILE *fp;
    static char szLine[10240] = {0};
    static char tmpstr[10240] = {0};
    memset(szLine, 0, sizeof(szLine));
    memset(tmpstr, 0, sizeof(tmpstr));
    int rtnval;
    int i = 0;
    int secFlag = 0;

    if ((fp = fopen(lpFileName, "rw+")) == NULL)
    {
#ifdef DEBUG
        printf("have no such file \n");
#endif
        return FALSE;
    }

    int lineLen = 0; //���г���
    int orgEqualPos = 0; //=����ԭ���е�λ��
    int equalPos = 0; //=����ȥ�ո���λ��
    strcpy(tmpstr, "[");
    strcat(tmpstr, lpAppName);
    strcat(tmpstr, "]");
    int endFlag;
    while (!feof(fp))
    {

        rtnval = fgetc(fp);
        if (rtnval == EOF)
        {
            //���һ�п����޻��з���
#ifdef DEBUG
            printf("EOF\n");
#endif
            rtnval = '\n';
            endFlag = 1;
        }
        //ע����
        if ('#' == rtnval || ';' == rtnval)
        {
            fgets(szLine, sizeof(szLine), fp);
            //reset
            i = 0;
            lineLen = 0;
            orgEqualPos = 0;
            equalPos = 0;
            memset(szLine, 0, sizeof(szLine));
            continue;
        }
        else if ('/' == rtnval)
        {
            szLine[i++] = rtnval;
            lineLen++;
            if ('/' == (rtnval = fgetc(fp))) //ע����
            {
                fgets(szLine, sizeof(szLine), fp);
                //reset
                i = 0;
                lineLen = 0;
                orgEqualPos = 0;
                equalPos = 0;
                memset(szLine, 0, sizeof(szLine));
                continue;
            }
        }

        if (rtnval != ' ' && rtnval != '\t')
        {
            szLine[i++] = rtnval;   //ȥ���ո��tab����ַ���
            if (rtnval == '=')
            {
                orgEqualPos = lineLen;
                equalPos = i - 1;
            }

        }

        lineLen++; //�ַ�

        if (rtnval == '\n')
        {
#ifdef DEBUG
            printf("Line %s\n", szLine);
#endif
            szLine[--i] = '\0';
            if (szLine[--i] == '\r')
                szLine[i--] = '\0';

            if ((equalPos != 0) && (secFlag == 1))
            {
                szLine[equalPos] = '\0';
                if (strcasecmp(szLine, lpKeyName) == 0)
                {
                    //�ҵ�key��Ӧ����
                    int leftPos = ftell(fp);
                    int writePos = leftPos - lineLen + orgEqualPos + 1;
                    fseek(fp, 0, SEEK_END);
                    int leftLen = ftell(fp) - leftPos;

                    char *pLeft[leftLen];
                    fseek(fp, leftPos, SEEK_SET);
                    fread(pLeft, leftLen, 1, fp);
                    fseek(fp, writePos, SEEK_SET);
                    fwrite(lpString, strlen(lpString), 1, fp);
                    fwrite("\n", sizeof(char), 1, fp);
                    fwrite(pLeft, leftLen, 1, fp);
                    fclose(fp);
                    return TRUE;
                }
            }

            else
            {
                if (strcasecmp(tmpstr, szLine) == 0)
                {
                    //�ҵ�section
                    secFlag = 1;
                }
                else if (secFlag == 1 && szLine[0] == '[' && szLine[i] == ']')
                {
                    //�����¸�section�ˣ�˵��û�ҵ�
                    int leftPos = ftell(fp) - lineLen;
                    int writePos = leftPos;
                    fseek(fp, 0, SEEK_END);
                    int leftLen = ftell(fp) - leftPos;
                    char *pLeft[leftLen];
                    fseek(fp, leftPos, SEEK_SET);
                    fread(pLeft, leftLen, 1, fp);
                    fseek(fp, writePos, SEEK_SET);
                    fwrite("\n", sizeof(char), 1, fp);
                    fwrite(lpKeyName, strlen(lpKeyName), 1, fp);
                    fwrite("=", sizeof(char), 1, fp);
                    fwrite(lpString, strlen(lpString), 1, fp);
                    fwrite("\n", sizeof(char), 1, fp);
                    fwrite(pLeft, leftLen, 1, fp);
                    fclose(fp);
                    return TRUE;
                }
            }
            //reset
            if (endFlag == 1)
                break;
            i = 0;
            lineLen = 0;
            orgEqualPos = 0;
            equalPos = 0;
            memset(szLine, 0, sizeof(szLine));
        }
    }
    //���ļ�β��
    if (secFlag)
    {
        //������section
        fseek(fp, 0, SEEK_END);
        fwrite("\n", sizeof(char), 1, fp);
        fwrite(lpKeyName, strlen(lpKeyName), 1, fp);
        fwrite("=", sizeof(char), 1, fp);
        fwrite(lpString, strlen(lpString), 1, fp);
        fwrite("\n", sizeof(char), 1, fp);
    }
    fclose(fp);
    return TRUE;
}
DWORD
GetPrivateProfileString(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
)
{
    FILE *fp;
    static char szLine[10240] = {0};
    static char tmpstr[10240] = {0};
    memset(szLine, 0, sizeof(szLine));
    memset(tmpstr, 0, sizeof(tmpstr));
    int rtnval;
    int i = 0;
    int secFlag = 0;

    if ((fp = fopen(lpFileName, "r")) == NULL)
    {
#ifdef DEBUG
        printf("have   no   such   file \n");
#endif
        return -1;
    }

    int equalPos = 0; //=����ȥ�ո���λ��
    strcpy(tmpstr, "[");
    strcat(tmpstr, lpAppName);
    strcat(tmpstr, "]");
    int endFlag;
    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        if (rtnval == EOF)
        {
            //���һ�п����޻��з���
#ifdef DEBUG
            printf("EOF\n");
#endif
            rtnval = '\n';
            endFlag = 1;
        }
        //ע����
        if ('#' == rtnval || ';' == rtnval)
        {
            fgets(szLine, sizeof(szLine), fp);
            //reset
            i = 0;
            equalPos = 0;
            memset(szLine, 0, sizeof(szLine));
            continue;
        }
        else if ('/' == rtnval)
        {
            szLine[i++] = rtnval;
            if ('/' == (rtnval = fgetc(fp))) //ע����
            {
                fgets(szLine, sizeof(szLine), fp);
                //reset
                i = 0;
                equalPos = 0;
                memset(szLine, 0, sizeof(szLine));
                continue;
            }
        }

        if (rtnval != ' ' && rtnval != '\t')
        {
            szLine[i++] = rtnval;   //ȥ���ո��tab����ַ���
            if (rtnval == '=')
            {
                equalPos = i - 1;
            }
        }

        if (rtnval == '\n')
        {

#ifdef DEBUG
            printf("Line %s\n", szLine);
#endif
            szLine[--i] = '\0';
            if (szLine[--i] == '\r')
                szLine[i--] = '\0';

            if ((equalPos != 0) && (secFlag == 1))
            {
                szLine[equalPos] = '\0'; //=�ű�0
                if (strcasecmp(szLine, lpKeyName) == 0)
                {
                    //�ҵ�key��Ӧ����
                    strncpy(lpReturnedString, szLine + equalPos + 1, nSize - 1);
                    lpReturnedString[nSize - 1] = '\0';
                    fclose(fp);
                    return 1;
                }
            }
            else
            {
                if (strcasecmp(tmpstr, szLine) == 0)
                {
                    //�ҵ�section
                    secFlag = 1;
                }
                else if (secFlag == 1 && szLine[0] == '[' && szLine[i] == ']')
                {
                    //�����¸�section�ˣ�˵��û�ҵ�
                    break;
                }
            }
            if (endFlag == 1)
                break;
            //reset
            i = 0;
            equalPos = 0;
            memset(szLine, 0, sizeof(szLine));

        }
    }
    fclose(fp);
    //û�ҵ�����Ĭ��
    strncpy(lpReturnedString, lpDefault, nSize - 1);
    lpReturnedString[nSize - 1] = '\0';
    return 0;

}

DWORD
GetPrivateProfileSection(LPCSTR lpAppName, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
    FILE *fp;
    static char szLine[10240] = {0};
    static char tmpstr[10240] = {0};
    memset(szLine, 0, sizeof(szLine));
    memset(tmpstr, 0, sizeof(tmpstr));

    int rtnval;
    int i = 0;
    int secFlag = 0;

    if ((fp = fopen(lpFileName, "r")) == NULL)
    {
#ifdef DEBUG
        printf("have   no   such   file \n");
#endif
        return -1;
    }

    int equalPos = 0; //=����ȥ�ո���λ��
    strcpy(tmpstr, "[");
    strcat(tmpstr, lpAppName);
    strcat(tmpstr, "]");

    int endFlag;
    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        if (rtnval == EOF)
        {
            //���һ�п����޻��з���
#ifdef DEBUG
            printf("EOF\n");
#endif
            rtnval = '\n';
            endFlag = 1;
        }
        //ע����
        if ('#' == rtnval || ';' == rtnval)
        {
            fgets(szLine, sizeof(szLine), fp);
            //reset
            i = 0;
            equalPos = 0;
            memset(szLine, 0, sizeof(szLine));
            continue;
        }
        else if ('/' == rtnval)
        {
            szLine[i++] = rtnval;
            if ('/' == (rtnval = fgetc(fp))) //ע����
            {
                fgets(szLine, sizeof(szLine), fp);
                //reset
                i = 0;
                equalPos = 0;
                memset(szLine, 0, sizeof(szLine));
                continue;
            }
        }

        if (rtnval != ' ' && rtnval != '\t')
        {
            szLine[i++] = rtnval;   //ȥ���ո��tab����ַ���
            if (rtnval == '=')
            {
                equalPos = i - 1;
            }
        }

        if (rtnval == '\n')
        {

#ifdef DEBUG
            printf("Line %s\n", szLine);
#endif
            szLine[--i] = '\0';
            if (szLine[--i] == '\r')
                szLine[i--] = '\0';

            if ((equalPos != 0) && (secFlag == 1))
            {
                //szLine[equalPos] = '\0'; //=�ű�0
                strncpy(lpReturnedString, szLine, i + 2);

                lpReturnedString += (i + 2);
            }
            else
            {
                if (strcasecmp(tmpstr, szLine) == 0)
                {
                    //�ҵ�section
                    secFlag = 1;
                }
                else if (secFlag == 1 && szLine[0] == '[' && szLine[i] == ']')
                {
                    //�����¸�section�ˣ�˵��û�ҵ�
                    break;
                }
            }
            if (endFlag == 1)
                break;
            //reset
            i = 0;
            equalPos = 0;
            memset(szLine, 0, sizeof(szLine));

        }
    }
    fclose(fp);
    //û�ҵ�����Ĭ��
    *lpReturnedString = '\0';

    return 0;
}

DWORD
GetPrivateProfileSectionNames(
    LPSTR lpszReturnBuffer,
    DWORD nSize,
    LPCSTR lpFileName
)
{
    FILE *fp;
    static char szLine[10240] = {0};
    static char tmpstr[10240] = {0};
    memset(szLine, 0, sizeof(szLine));
    memset(tmpstr, 0, sizeof(tmpstr));
    int rPos = 0;
    memset(lpszReturnBuffer, 0, nSize);
    int rtnval;
    int i = 0;
    int endFlag;

    if ((fp = fopen(lpFileName, "r")) == NULL)
    {
#ifdef DEBUG
        printf("have   no   such   file \n");
#endif
        return -1;
    }

    while (!feof(fp))
    {
        rtnval = fgetc(fp);
        if (rtnval == EOF)
        {
            //���һ�п����޻��з���
#ifdef DEBUG
            printf("EOF\n");
#endif
            rtnval = '\n';
            endFlag = 1;
        }
        //ע����
        if ('#' == rtnval || ';' == rtnval)
        {
            fgets(szLine, sizeof(szLine), fp);
            //reset
            i = 0;
            memset(szLine, 0, sizeof(szLine));
            continue;
        }
        else if ('/' == rtnval)
        {
            szLine[i++] = rtnval;
            if ('/' == (rtnval = fgetc(fp))) //ע����
            {
                fgets(szLine, sizeof(szLine), fp);
                //reset
                i = 0;
                memset(szLine, 0, sizeof(szLine));
                continue;
            }
        }

        if (rtnval != ' ' && rtnval != '\t')
        {
            szLine[i++] = rtnval;   //ȥ���ո��tab����ַ���

        }

        if (rtnval == '\n')
        {

#ifdef DEBUG
            printf("Line %s\n", szLine);
#endif
            szLine[--i] = '\0';
            if (szLine[--i] == '\r')
                szLine[i--] = '\0';

            if (szLine[0] == '[' && szLine[i] == ']')
            {
                //�ҵ�section
                int j;
                for (j = 1; j < i && rPos < nSize - 1; j++)
                    lpszReturnBuffer[rPos++] = szLine[j];
                lpszReturnBuffer[rPos++] = '\0';
                if (rPos >= nSize)
                {
                    break;
                }
            }
            if (endFlag == 1)
                break;

            //reset
            i = 0;
            memset(szLine, 0, sizeof(szLine));

        }
    }
    lpszReturnBuffer[rPos] = '\0';
    fclose(fp);
    return 0;
}
