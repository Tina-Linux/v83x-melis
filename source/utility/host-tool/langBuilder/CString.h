#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#if defined(WIN32)
#include <windows.h>
//#elif defined(linux)
//#elif defined(_AIX)
//#include <extension.h>
#else

#define STR_PAGE_SIZE   1024
#define UCHAR_MAX   128
#define TRUE        true
#define FALSE       false

#include <string>
using namespace std;

char *strupr(char *src)
{
    while (*src != '\0')
    {
        if (*src >= 'a' && *src <= 'z')
            /*
             * ��ASCII�����д�ַ���ֵ�ȶ�ӦСд�ַ���ֵС32.
             * *p -= 0x20; // 0x20��ʮ���ƾ���32
             */
            *src -= 32;
        src++;
    }
    return (src);
}


char *strlwr(char *src)
{
    while (*src != '\0')
    {
        if (*src > 'A' && *src <= 'Z')
        {
            *src += 32;
        }
        src++;
    }
    return (src);
}

/* constructors */
class CString : public string
{
public:
    /* allocate buffers */
    char     *m_pBuffer;
    int m_nLength;
    int m_nSize;
    char    *m_pDataStart;
    char     *m_pDataEnd;

    CString(int iBufferNum)
    {
        m_pBuffer   = new char[iBufferNum * STR_PAGE_SIZE];         /* ��̬�����ڴ� */
        m_nLength   = 0;                                            /* ���� */
        m_nSize     = iBufferNum * STR_PAGE_SIZE;                   /* �ַ�����С��ʼ��ΪiBufferNum��4*1024 */
        m_pDataStart = m_pDataEnd = m_pBuffer;                      /* ��ʼ�ַ���ָ��ȫ��ָ��m_pBuffer */
    }


    CString()
    {
        m_pBuffer   = new char[STR_PAGE_SIZE];                      /* ��̬�����ڴ� */
        m_nLength   = 0;                                            /* ���� */
        m_nSize     = STR_PAGE_SIZE;                                /* �ַ�����С��ʼ��ΪiBufferNum��4*1024 */
        m_pDataStart    = m_pDataEnd = m_pBuffer;                       /* ��ʼ�ַ���ָ��ȫ��ָ��m_pBuffer */
    }


    /* new one character */
    CString(char c, int iBufferNum)                                         /* ����Ϊ�ַ�c��iBufferNum�����캯�� */
    {
        char data[2], len = 2;

        data[0] = c;                                                    /* ��c�����ַ�������λ */
        data[1] = 0;                                                    /* ���ַ�����data�ڶ�λ��Ϊ0 */
        if (len > iBufferNum * STR_PAGE_SIZE)                           /* ���iBufferNum��4*1024С��3 */
        {
            m_pBuffer   = new char[len + 1];                    /* Ϊ�ַ��������Ҫ�ַ���m_pBuffer��̬�����ڴ�ռ� */
            m_nSize     = len + 1;                              /* �ַ�����СΪ3 */
        }
        else
        {
            m_pBuffer   = new char[iBufferNum * STR_PAGE_SIZE]; /* �ַ��������ڴ�ռ� */
            m_nSize     = iBufferNum * STR_PAGE_SIZE;
        }

        /* copy data and set total length */
        CreateFromData(data, len);                                      /* �����ַ�����data��ʼ��������m_pDataStart��m_pDataEnd */
    }


    /* copy data from a null terminated string */
    CString(const char *data, int iBufferNum)
    {
        long len = strlen(data);

        if (len > iBufferNum * STR_PAGE_SIZE)           /* �ַ����Ĵ�С�����ƶ�������4*1024 */
        {
            m_pBuffer   = new char[len + 1];    /* ��̬�����ڴ�ռ� */
            m_nSize     = len + 1;
        }
        else
        {
            m_pBuffer   = new char[iBufferNum * STR_PAGE_SIZE];
            m_nSize     = iBufferNum * STR_PAGE_SIZE;
        }

        /* copy data and set total length */
        CreateFromData(data, len);                      /* �����ַ�����data��ʼ��������m_pDataStart��m_pDataEnd */
    }


    /* copy data from a binary data */
    CString(const char *data, long len, int iBufferNum)     /* ���캯���Ĳ���Ϊ�ַ��� */
    {
        if (len > iBufferNum * STR_PAGE_SIZE)           /* ����ַ����Ĵ�С�����ƶ�������4*1024 */
        {
            m_pBuffer   = new char[len + 1];    /* ��̬�����ڴ�ռ� */
            m_nSize     = len + 1;
        }
        else
        {
            m_pBuffer   = new char[iBufferNum * STR_PAGE_SIZE];
            m_nSize     = iBufferNum * STR_PAGE_SIZE;
        }

        /* copy data and set total length */
        CreateFromData(data, len);   /* �����ַ�����data��ʼ��������m_pDataStart��m_pDataEnd */
    }


    /* destructor */
    ~CString(void)
    {
        SafeDestroy();
    }


    /* destroy allocated memory space and init all pointers */
    void SafeDestroy(void)
    {
        /*
         * SAFEDELETE(m_pBuffer);
         * Important for InitPointers()
         */
        m_nSize = 0;
        Empty();
    }


    void Empty(void)
    {
#ifdef WIN32
        ZeroMemory(m_pBuffer, m_nSize);
#else                                                           /* Linux, AIX */
        memset(m_pBuffer, 0, m_nSize);
#endif
        m_pDataStart    = m_pDataEnd = m_pBuffer;       /* �����ַ�����ͷָ���βָ��ȫ��ָ���ַ�����ͷ�� */
        m_nLength   = 0;                            /* �ַ���������Ϊ0 */
    }


    char *GetData(void) const                               /* ���ָ����ַ�����ͷ��ָ�� */
    {
        return (m_pDataStart);
    }


    int CreateFromData(const char *data, long len)          /* ����data�ͳ���len�������� */
    {
        Empty();                                        /* ��� */

        if (len <= 0)
            return (TRUE);

        if (data == NULL)
            return (TRUE);

        /* actually, it's impossible */
        if (len > m_nSize)
            return (FALSE);

        /* copy data and set length */
        memcpy(m_pDataStart, data, len);                        /* ���ַ����鸴�Ƹ�m_pDataStartָ���ڴ�ռ� */
        m_nLength       = len;
        m_pDataStart[m_nLength] = '\0';
        m_pDataEnd      = &(m_pDataStart[m_nLength]);   /* ȡ��βָ��m_pDataEnd */
        return (TRUE);
    }


    long GetLength(void) const
    {
        return (m_nLength);
    }


    int IsEmpty(void) const
    {
        return (!m_nLength);
    }


    int Grow(int iBufferNum)
    {
        if (iBufferNum <= 0)
            return (0);
        AssignNewSpace(m_nSize + iBufferNum * STR_PAGE_SIZE, 1);   /* �����µ��ڴ�ռ䣬��Ϊԭ�����������ƶ�1�� */
        return (1);
    }


    int Append(const char *pSrc, int iLen)
    {
        int total_len;
        char    *pNewStart = NULL;

        if (iLen <= 0)
            return (0);

        total_len = m_nLength + iLen;                                                                   /* append����ܵ��ַ������� */

        /* if some space avaliable, defrag it firstly. */
        if (m_nSize > total_len)                                                                        /* ���ԭ�����ڴ�ռ䳤�ȴ���append����ַ������� */
        {
            /* ���ԭ��ʣ����пռ�С����������ַ����ĳ��ȣ����� */
            if (m_nSize - (m_pDataEnd - m_pBuffer) < iLen && m_pDataStart - m_pBuffer > 0)          /*����m_pDataStart��m_pBuffer�ĺ��� */
            {
                Defrag();                                                                       /* ����ԭ�����ַ��� */
            }
        }
        else                                                                                            /* ���ԭ�����ڴ�ռ䳤С��append����ַ������ȣ���Ҫ�����¿ռ� */
        {
            /* allocate new memory space and copy orignal data */
            AssignNewSpace(total_len + 1, 1);                                                       /* �����µ��ڴ�ռ䣬��Ϊԭ�����������ƶ�1�� */
        }

        /* get the merge point */
        pNewStart = m_pDataEnd;                                                                         /* ��ԭ���ַ�����ĩβָ��ת���������Ӵ��Ŀ�ʼָ�� */
        if (!pNewStart)
            return (0);
        /* copy data and adjust some pointers */
        memcpy(pNewStart, pSrc, iLen);                                                                  /* ������Ӵ�pSrc������ԭ��������ĩβ */
        m_nLength       = total_len;                                                            /* �ַ������ܳ��ȱ仯 */
        m_pDataStart[m_nLength] = 0;                                                                    /* �µ��ַ��������Ϊ��Ϊ0 */
        m_pDataEnd      = &(m_pDataStart[m_nLength]);                                           /* ��m_pDataEndָ���µ��ַ�����ĩβָ�� */
        return (1);
    }


    void Defrag(void)
    {
        /* Sure!    ���³�ʼ��ԭ���ַ���ͷָ��m_pDataStart��ָ��ͷ�� */
        memmove(m_pBuffer, m_pDataStart, m_nLength);            /* ��m_pDataStart���Ƹ�m_pBuffer */
        /* adjust those related pointers */
        m_pDataStart        = m_pBuffer;                    /* m_pDataStartָ��m_pBuffer */
        m_pDataStart[m_nLength] = 0;
        m_pDataEnd      = &(m_pDataStart[m_nLength]);   /* ���»��ԭ�����ַ���βָ��m_pDataEnd */
    }


    /* Append another CString to this one */

    int Append(CString *pNewStr)
    {
        char *pNewStart = NULL, *pSrc = NULL, *pDest = NULL;

        int len = pNewStr->GetLength();     /* ������Ӵ��ĳ��� */
        int total_len;

        if (len <= 0)
            return (0);

        /* const char * */
        pSrc = pNewStr->GetData();              /* ���Ҫ��ӵ��ַ�����ͷָ�� */
        if (!pSrc)
            return (0);

        total_len = m_nLength + len;            /* ���ַ������ܳ��� = ԭ���ַ������� + ����ַ������� */

        /* if some space avaliable, defrag it firstly. */
        if (m_nSize - (m_pDataEnd - m_pBuffer) < len && m_pDataStart - m_pBuffer > 0)
        {
            Defrag();
        }

        /* allocate new memory space */
        AssignNewSpace(total_len + 1, 1);                       /* //����total_len�����µ��ڴ�ռ� */
        /* get the merge point */
        pNewStart = m_pDataEnd;                                 /* ��ԭ���ַ�����ĩβָ��ת���������Ӵ��Ŀ�ʼָ�� */
        if (!pNewStart)
            return (0);

        /* copy data and adjust some pointers */
        memcpy(pNewStart, pSrc, len);                           /* ������Ӵ�pSrc������ԭ��������ĩβ */
        m_nLength       = total_len;
        m_pDataStart[m_nLength] = 0;
        m_pDataEnd      = &(m_pDataStart[m_nLength]);   /* ��m_pDataEndָ���µ��ַ�����ĩβָ�� */

        return (1);
    }


    /* Adjust start and end pointer of its buffer */

    /* Get one character at give position */

    char GetAt(int nIndex) const
    {
        if (nIndex >= m_nLength)
            return (-1);
        if (nIndex < 0)
            return (-1);

        return (m_pDataStart[nIndex]);
    }


    /* return single character at zero-based index */
    char operator[](int nIndex) const
    {
        if (nIndex >= m_nLength)
            return (-1);

        return (m_pDataStart[nIndex]);
    }


    /* return pointer to const string */
    const char  *LPCTSTR() const       /* �����ַ���ת��Ϊconst char*�ַ������� */
    {
        return ((const char *) m_pDataStart);
    }


    /* duplicate a string */
    CString *Duplicate(int iBufferNum) const
    {
        CString *pCStr = NULL;

        pCStr = new CString(m_pDataStart, m_nLength, iBufferNum);

        return (pCStr);
    }


    /* copy from another CString */
    const CString &operator=(CString &stringSrc)                    /* ��ֵ������ */
    {
        long len = stringSrc.GetLength();                       /* ȡ��stringSrc�ַ������� */

        if (len >= m_nSize)                                     /* ����ռ��С���� */
        {
            AssignNewSpace(len + 1, 0);                     /* don't copy orignal data      //��������һ���µ��ڴ�ռ� */
        }

        /* copy data and adjust pointers */
        Empty();                                                /* ��� */
        memcpy(m_pDataStart, stringSrc.GetData(), len);         /* ���ַ���stringSrc���Ƹ��µ�m_pDataStart */
        m_pDataStart[len]   = 0;
        m_pDataEnd      = &(m_pDataStart[len]);         /* ����µ�m_pDataEnd */
        m_nLength       = len;

        return (*this);
    }


    /* set string content to single character */
    CString &operator=(char c)                      /* ��ֵ�����������ַ������ַ��� */
    {
        if (m_nSize < 2)                        /* c + '\0' length                      //����ڴ�ռ䲻�� */
        {
            /* it needs to realloc space, but we use new and delete pair */
            SafeDestroy();
            m_pBuffer   = new char[2];  /* ���������ڴ� */
            m_nSize     = 2;
            /* TODO, I don't check the value of this pointer, unkown result. :) */
        }
        m_pDataStart    = m_pBuffer;            /* ���ͷָ�� */
        m_pDataStart[0] = c;
        m_pDataStart[1] = 0;
        m_pDataEnd  = &(m_pDataStart[1]);   /* ���βָ�� */
        m_nLength   = 1;

        return (*this);
    }


    /* Set one character at give position */

    void SetAt(long pos, char ch)
    {
        if (pos < m_nLength)
            m_pDataStart[pos] = ch;

        return;
    }


    /* Get the string started from give position */

    void GetManyChars(char *buf, long pos, long len) const
    {
        if (buf == NULL)
            return;

        if (pos >= m_nLength)
            return;

        if (pos + len > m_nLength)
            len = m_nLength - pos;

        if (len > 0)
            memcpy(buf, m_pDataStart + pos, len);    /* �� m_pDataStart + pos��ʼ��Ϊlen���Ӵ����Ƹ�buf */
    }


    /* Compare itself with a new string, case-sensitive */

    int Compare(const char *pNewStr) const
    {
        if (pNewStr == NULL)
            return (-1);

        return (strcmp(m_pDataStart, pNewStr));
    }


    /* Compare itself with a new string, case-ignored */
    int CompareNoCase(const char *pNewStr) const
    {
        if (pNewStr == NULL)
            return (-1);

#ifndef WIN32
        return (strcasecmp(m_pDataStart, pNewStr));
#else
        return (stricmp(m_pDataStart, pNewStr));
#endif
    }


    /* find a character start from a give position */
    int Find(int ch, long pos) const
    {
        char *p = NULL;

        if (ch < 0)
            return (-1);

        if (ch > UCHAR_MAX)
            return (-1);

        if (pos < 0)
            return (-1);

        if (pos >= m_nLength)
            return (-1);

        p = (char *) memchr(m_pDataStart + pos, ch, m_nLength - pos);

        if (!p)
            return (-1);
        return (p - m_pDataStart);
    }


    /* find a string start from a give position */
    int Find(const char *str, long pos) const
    {
        long    len;
        char    *p = NULL;

        if (str == NULL)
            return (-1);

        len = strlen(str);


        if (len == 0)
            return (0);

        p = (char *) strstr(m_pDataStart + pos, str);

        if (p == NULL)
            return (-1);
        else
            return (p - m_pDataStart);
    }


    char *GetBuffer(int nMinBufLength)   /* ��ø��ַ���ͷָ�룬����˵�����ص��ڴ�ռ���Сֵ */
    {
        Defrag();
        if (nMinBufLength > m_nLength)
            AssignNewSpace(nMinBufLength, 1);
        return (m_pDataStart);
    }


    void ReleaseBuffer(int nNewLength)
    {
        return;
    }


    /* Extracts the left part of a string. */
    CString Left(int count) const
    {
        if (count < 0)
            count = 0;
        if (count >= m_nLength)
            return (*this);

        CString dest(m_pDataStart, count);   /* ���ù��캯���½�һ�� */
        return (dest);
    }


    /* Extracts the right part of a string. */
    CString Right(int count) const
    {
        if (count < 0)
            count = 0;
        if (count >= m_nLength)
            return (*this);

        CString dest(&(m_pDataStart[m_nLength - count]), count);
        return (dest);
    }


    /* Converts all the characters in this string to uppercase characters. */
    void MakeUpper(void)
    {
        strupr(m_pDataStart);
    }


    /* Converts all the characters in this string to lowercase characters. */
    void MakeLower(void)
    {
        strlwr(m_pDataStart);
    }


    /* TODO: check the space left in the two pading of the whole buffer */

    /* trim the left spaces */
    void TrimLeft(void)   /* //���ַ����ұߵĿո�ȥ�� */
    {
        int start = 0;

        while (isspace(m_pDataStart[start]) && start < m_nLength)
            start++;

        if (start > 0)
        {
            m_pDataStart += start;
        }
    }


    /* trim the right spaces */
    void TrimRight(void)   /* ���ַ����ұߵĿո�ȥ�� */
    {
        int end = m_nLength - 1;

        while (isspace(m_pDataStart[end]) && end >= 0)
            end--;

        if (end < 0)
        {
            end     = 0;
            m_pDataEnd  = &(m_pDataStart[end]);
        }
        else
        {
            m_pDataEnd = &(m_pDataStart[end]);
        }
    }


    /* trim both sides */
    void Trim(void)   /* //���ַ����Ŀո�ȥ�� */
    {
        TrimLeft();
        TrimRight();
    }


    int Replace(const char *lpszOld, const char *lpszNew)
    {
        /* can't have empty or NULL lpszOld */
        if (!lpszOld)
            return (0);
        int nOldLen = strlen(lpszOld);                                                  /* ��þ��ַ����ĳ��� */
        if (nOldLen <= 0)
            return (0);
        int nNewLen = strlen(lpszNew);                                                  /* ������ַ����ĳ��� */

        /* loop once to figure out the size of the result string */
        int nCount      = 0;
        char    *lpszStart  = m_pDataStart;
        char    *lpszEnd    = m_pDataEnd;
        char    *lpszTarget;
        while (lpszStart < lpszEnd)                                                     /* ѭ������ԭ���ַ��� */
        {
            while ((lpszTarget = strstr(lpszStart, lpszOld)) != NULL)               /* ������ַ���lpszStart�з����Ӵ�lpszOld */
            {
                nCount++;                                                       /* �Ӵ�����+1 */
                lpszStart = lpszTarget + nOldLen;                               /* ����λ�ַ���lpszStart���ӵ�һ���Ӵ���ʼ */
            }
            lpszStart += strlen(lpszStart) + 1;                                     /* ������� */
        }

        /* if any changes were made, make them */
        if (nCount > 0)                                                                 /* ������ظ����ַ��� */
        {
            /* allocate a new buffer (slow but sure) */
            int nNewLength = m_nLength + (nNewLen - nOldLen) * nCount;              /* ���Ǻ����ַ����Ĵ�С */
            AssignNewSpace(nNewLength + 1, 1);                                      /* Ϊ�µ��ַ��������ڴ�ռ� */
            /* then we just do it in-place */
            lpszStart   = m_pDataStart;                                         /* ���³�ʼ��m_pDataStart��lpszStart��lpszEnd */
            lpszEnd     = m_pDataEnd;

            /* loop again to actually do the work */
            while (lpszStart < lpszEnd)                                             /* ѭ������ԭ�����ַ��� */
            {
                while ((lpszTarget = strstr(lpszStart, lpszOld)) != NULL)       /* ������ַ���lpszStart�з����Ӵ�lpszOld */
                {
                    int nBalance = lpszEnd - (lpszTarget + nOldLen);        /* �ַ���lpszTarget������ַ����� */
                    memmove(lpszTarget + nNewLen, lpszTarget + nOldLen,
                            nBalance * sizeof(char));                      /* ����lpszTargetԭ�����ַ�������ΪlpszTarget��������ΪnNewLen��С�ڴ� */
                    memcpy(lpszTarget, lpszNew, nNewLen * sizeof(char));    /* �����ַ���lpszNew���Ǿɵ��Ӵ�lpszTarget */
                    lpszStart       = lpszTarget + nNewLen;         /* Ѱ��Ŀ���ַ�������nNewLen */
                    lpszStart[nBalance] = '\0';
                }
                lpszStart += strlen(lpszStart) + 1;                             /* Ѱ��Ŀ���ַ��������� */
            }
            m_nLength = nNewLength;
        }

        return (nCount);
    }


    /* format a string */
    void Format(char *fmt, ...)
    {
        char TmpBuffer[STR_PAGE_SIZE];                          /* TODO, should calculate this size dynamically. */

        va_list argList;
        va_start(argList, fmt);
#ifdef WIN32
        _vsnprintf(TmpBuffer, STR_PAGE_SIZE, fmt, argList);     /* just not overwrite something */
#else
        vsnprintf(TmpBuffer, STR_PAGE_SIZE, fmt, argList);      /* just not overwrite something */
#endif
        va_end(argList);
    }


    /* copy string content from ANSI string (converts to TCHAR) */
    const CString &operator=(const char *lpsz)
    {
        int len = strlen(lpsz);

        if (m_nSize < len)      /* c + '\0' length */
        {
            /* it needs to realloc space, but we use new and delete pair */
            SafeDestroy();
            m_pBuffer   = new char[len + 1];
            m_nSize     = len + 1;
            /* TODO, I don't check the value of this pointer, unkown result. :) */
        }
        m_pDataStart = m_pBuffer;
        strcpy((char *) m_pDataStart, lpsz);
        m_pDataStart[len]   = 0;
        m_pDataEnd      = &(m_pDataStart[len]);
        m_nLength       = len;

        return (*this);
    }


    /* concatenate a UNICODE character after converting it to TCHAR */
    const CString &operator+=(const char *lpsz)
    {
        int len = strlen(lpsz);

        if (m_nSize < m_nLength + len + 1)
        {
            AssignNewSpace(m_nLength + len + 1, 1);   /* allocate new space and move orignal data */
        }
        Defrag();
        memcpy(m_pDataEnd, lpsz, len);
        m_pDataEnd  += len;
        *m_pDataEnd = 0;

        return (*this);
    }


    /* concatenate a single character */
    const CString &operator+=(char ch)
    {
        if (m_nSize < m_nLength + 1 + 1)
        {
            AssignNewSpace(m_nLength + 1 + 1, 1);   /* allocate new space and move orignal data */
        }
        Defrag();
        memcpy(m_pDataEnd, &ch, 1);
        m_pDataEnd  += 1;
        *m_pDataEnd = 0;

        return (*this);
    }


    /* concatenate from another CString */
    const CString &operator+=(CString &string)
    {
        if (m_nSize < m_nLength + string.GetLength() + 1)
        {
            AssignNewSpace(m_nLength + string.GetLength() + 1, 1);   /* allocate new space and move orignal data */
        }
        Defrag();
        memcpy(m_pDataEnd, string.GetData(), string.GetLength());
        m_pDataEnd  += string.GetLength();
        *m_pDataEnd = 0;

        return (*this);
    }


    void AssignNewSpace(int iNewTotalSize, int iNeedMove)
    {
        char *pNewSpace = NULL;                                         /* �µ��ַ���ָ�룬��ʼ��NULL */

        if (iNewTotalSize <= m_nSize)                                   /* ȷ���µ��ڴ�ռ����ԭ�����ڴ�ռ� */
            return;

        /* allocate new space */
        pNewSpace = new char [iNewTotalSize];                           /* pNewSpace��̬�����ڴ�ռ� */
        if (pNewSpace == NULL)
            return;

        if (iNeedMove)
        {
            memcpy(pNewSpace, m_pDataStart, m_nLength + 1);         /* ��ԭ���ַ������Ƹ��������ڴ� */
        }
        /* SAFEDELETE(m_pBuffer);              //��ȫɾ��ԭ�е��ַ���m_pBuffer */
        m_pBuffer   = pNewSpace;
        m_pDataStart    = m_pBuffer;
        m_pDataEnd  = &(m_pDataStart[m_nLength]);                   /* ����m_pBuffer��m_pDataStart��m_pDataEnd */

        /* adjust new size */
        m_nSize = iNewTotalSize;
    }

    /*
     * NOTE: "operator+" is done as friend functions for simplicity
     *      There are three variants:
     *          CString + CString
     * and for ? = TCHAR, LPCTSTR
     *          CString + ?
     *          ? + CString
     */
    void ConcatCopy(const char *str1, int nSrc1Len, const char *str2, int nSrc2Len)
    {
        int nNewLen = nSrc1Len + nSrc2Len;

        AssignNewSpace(nNewLen + 1, 0);

        /* append two string */
        Append(str1, nSrc1Len);
        Append(str2, nSrc2Len);
    }


    /*
     * friend methods
     * Class + Class
     */


    CString operator+(const CString &string1)
    {
        /* CString s; */

        /*
         * s.ConcatCopy(string1.GetData(), string1.GetLength(), string2.GetData(), string2.GetLength());
         * return s;
         */
        if (m_nSize < m_nLength + string1.GetLength() + 1)
        {
            AssignNewSpace(m_nLength + string1.GetLength() + 1, 1);   /* allocate new space and move orignal data */
        }
        Defrag();
        memcpy(m_pDataEnd, string1.GetData(), string1.GetLength());
        m_pDataEnd  += string1.GetLength();
        *m_pDataEnd = 0;

        return (*this);
    }


    /* Class + char */
    CString operator+(char ch)
    {
        if (m_nSize < m_nLength + 1 + 1)
        {
            AssignNewSpace(m_nLength + 1 + 1, 1);   /* allocate new space and move orignal data */
        }
        Defrag();
        memcpy(m_pDataEnd, &ch, 1);
        m_pDataEnd  += 1;
        *m_pDataEnd = 0;

        return (*this);
    }



    /* Class + char * */
    CString operator+(const char *lpsz)
    {
        if (m_nSize < m_nLength + strlen(lpsz) + 1)
        {
            AssignNewSpace(m_nLength + strlen(lpsz) + 1, 1);     /* allocate new space and move orignal data */
        }
        Defrag();
        memcpy(m_pDataEnd, lpsz, strlen(lpsz));
        m_pDataEnd  += strlen(lpsz);
        *m_pDataEnd = 0;

        return (*this);
    }

    /* Compare operators */
    bool operator==(const CString &s2)
    {
        return (this->Compare(s2.GetData()) == 0);
    }


    bool operator==(const char *s2)
    {
        return (this->Compare(s2) == 0);
    }

    bool operator!=(const CString &s2)
    {
        return (this->Compare(s2.GetData()) != 0);
    }


    bool operator!=(const char *s2)
    {
        return (this->Compare(s2) != 0);
    }

    bool operator>(const CString &s2)
    {
        return (this->Compare(s2.GetData()) > 0);
    }


    bool operator>(const char *s2)
    {
        return (this->Compare(s2) > 0);
    }

    bool operator<(const CString &s2)
    {
        return (this->Compare(s2.GetData()) < 0);
    }


    bool operator<(const char *s2)
    {
        return (this->Compare(s2) < 0);
    }


    bool operator>=(const CString &s2)
    {
        return (this->Compare(s2.GetData()) >= 0);
    }


    bool operator>=(const char *s2)
    {
        return (this->Compare(s2) >= 0);
    }


    bool operator<=(const CString &s2)
    {
        return (this->Compare(s2.GetData()) <= 0);
    }


    bool operator<=(const char *s2)
    {
        return (this->Compare(s2) <= 0);
    }
};
#endif
