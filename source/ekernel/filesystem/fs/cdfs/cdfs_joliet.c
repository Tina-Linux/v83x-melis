/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : cdfs_joliet.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-16
* Descript: isofs ioliet handling functions,
*           Joliet: Microsoft's Unicode extensions to iso9660.
* Update  : date                auther      ver     notes
*           2011-3-16 13:55:27  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "nls.h"
#include "page_pool.h"
#include "fstypedef.h"
#include "fsunaligned.h"
#include "cdfs.h"

/*
 * Convert Unicode 16 to UTF-8 or ASCII.
 */
static int iso_uni16_to_x8(unsigned char *ascii,
                           __be16 *uni,
                           int len,
                           struct nls_table *nls)
{
    __be16 *ip, ch;
    unsigned char *op;

    ip = uni;
    op = ascii;

    while ((ch = get_unaligned(ip)) && len)
    {
        int llen;
        llen = nls->uni2char(be16_to_cpu(ch), op, NLS_MAX_CHARSET_SIZE);
        if (llen > 0)
        {
            op += llen;
        }
        else
        {
            *op++ = '?';
        }
        ip++;

        len--;
    }
    *op = 0;
    return (op - ascii);
}

/* Convert big endian wide character string to utf8 */
static int iso_wcsntombs_be(__u8 *s, const __u8 *pwcs,
                            int inlen, int maxlen)
{
    const __u8 *ip;
    __u8 *op;
    int size;
    __u16 c;

    op = s;
    ip = pwcs;
    while ((*ip || ip[1]) && (maxlen > 0) && (inlen > 0))
    {
        c = (*ip << 8) | ip[1];
        if (c > 0x7f)
        {
            size = utf8_wctomb(op, c, maxlen);
            if (size == -1)
            {
                /* Ignore character and move on */
                maxlen--;
            }
            else
            {
                op += size;
                maxlen -= size;
            }
        }
        else
        {
            *op++ = (__u8) c;
        }
        ip += 2;
        inlen--;
    }
    return (op - s);
}

int get_joliet_filename(struct iso_directory_record *de,
                        unsigned char *outname,
                        struct inode *inode)
{
    unsigned char utf8;
    struct nls_table *nls;
    unsigned char len = 0;

    utf8 = ISOFS_SB(inode->i_sb)->s_utf8;
    nls = ISOFS_SB(inode->i_sb)->s_nls_iocharset;

    if (utf8)
    {
        len = iso_wcsntombs_be(outname, de->name,
                               de->name_len[0] >> 1, PAGE_SIZE);
    }
    else
    {
        len = iso_uni16_to_x8(outname, (__be16 *) de->name,
                              de->name_len[0] >> 1, nls);
    }
    if ((len > 2) && (outname[len - 2] == ';') && (outname[len - 1] == '1'))
    {
        len -= 2;
    }

    /*
     * Windows doesn't like periods at the end of a name,
     * so neither do we
     */
    while (len >= 2 && (outname[len - 1] == '.'))
    {
        len--;
    }

    return len;
}
