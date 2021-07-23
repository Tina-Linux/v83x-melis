/**
 * logging.c - Centralised logging.  Part of the Linux-FS project.
 *
 * Copyright (c) 2005 Richard Russon
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-FS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdarg.h>

#include "fsys_types.h"
#include "fs_logging.h"
#include "fsys_debug.h"
#include "fsys_libs.h"


#ifndef PATH_SEP
#define PATH_SEP '\\'
#endif

int fs_log_handler_fprintf(const char *function, const char *file,
                           int line, __u32 level, void *data, const char *format, va_list args);

/* Colour prefixes and a suffix */
static const char *col_green  = "\e[32m";
static const char *col_cyan   = "\e[36m";
static const char *col_yellow = "\e[01;33m";
static const char *col_red    = "\e[01;31m";
static const char *col_redinv = "\e[01;07;31m";
static const char *col_end    = "\e[0m";
int line_begin = 1;
extern __s32 fs_err;

/**
 * struct fs_logging - Control info for the logging system
 * @levels: Bitfield of logging levels
 * @flags:  Flags which affect the output style
 * @handler:    Function to perform the actual logging
 */
struct fs_logging
{
    __u32 levels;
    __u32 flags;
    fs_log_handler *handler;
};

#define FSLOG_MAX_FUNC_ITME 10
#define FSLOG_MAX_FUNNAME_LEN 32
struct fs_log_fun_item
{
    __s32 inuse;  /* -1: -tracef; 0: not used; 1: +tracef */
    char name[FSLOG_MAX_FUNNAME_LEN];
};
struct fs_log_funs
{
    __u32 cnt;
    struct fs_log_fun_item item[FSLOG_MAX_FUNC_ITME];
};

/**
 * fs_log - This struct controls all the logging in the library and tools.
 */
static struct fs_logging fs_log = (struct fs_logging)
{
    .levels = FS_LOG_LEVEL_QUIET |
              FS_LOG_LEVEL_WARNING |
              FS_LOG_LEVEL_ERROR |
              FS_LOG_LEVEL_CRITICAL |
              FS_LOG_LEVEL_PROGRESS |
              FS_LOG_LEVEL_PERROR ,

              .flags =  FS_LOG_FLAG_ONLYNAME |
                        FS_LOG_FLAG_PREFIX,
                        .handler = fs_log_handler_fprintf,
};

static struct fs_log_funs fs_log_funtbl = {0};

/**
 * fs_log_get_levels - Get a list of the current logging levels
 *
 * Find out which logging levels are enabled.
 *
 * Returns:  Log levels in a 32-bit field
 */
__u32 fs_log_get_levels(void)
{
    return fs_log.levels;
}

/**
 * fs_log_set_levels - Enable extra logging levels
 * @levels: 32-bit field of log levels to set
 *
 * Enable one or more logging levels.
 * The logging levels are named: FS_LOG_LEVEL_*.
 *
 * Returns:  Log levels that were enabled before the call
 */
__u32 fs_log_set_levels(__u32 levels)
{
    __u32 old;
    old = fs_log.levels;
    fs_log.levels |= levels;
    return old;
}

/**
 * fs_log_clear_levels - Disable some logging levels
 * @levels: 32-bit field of log levels to clear
 *
 * Disable one or more logging levels.
 * The logging levels are named: FS_LOG_LEVEL_*.
 *
 * Returns:  Log levels that were enabled before the call
 */
__u32 fs_log_clear_levels(__u32 levels)
{
    __u32 old;
    old = fs_log.levels;
    fs_log.levels &= (~levels);
    return old;
}


/**
 * fs_log_get_flags - Get a list of logging style flags
 *
 * Find out which logging flags are enabled.
 *
 * Returns:  Logging flags in a 32-bit field
 */
__u32 fs_log_get_flags(void)
{
    return fs_log.flags;
}

/**
 * fs_log_set_flags - Enable extra logging style flags
 * @flags:  32-bit field of logging flags to set
 *
 * Enable one or more logging flags.
 * The log flags are named: FS_LOG_LEVEL_*.
 *
 * Returns:  Logging flags that were enabled before the call
 */
__u32 fs_log_set_flags(__u32 flags)
{
    __u32 old;
    old = fs_log.flags;
    fs_log.flags |= flags;
    return old;
}

/**
 * fs_log_clear_flags - Disable some logging styles
 * @flags:  32-bit field of logging flags to clear
 *
 * Disable one or more logging flags.
 * The log flags are named: FS_LOG_LEVEL_*.
 *
 * Returns:  Logging flags that were enabled before the call
 */
__u32 fs_log_clear_flags(__u32 flags)
{
    __u32 old;
    old = fs_log.flags;
    fs_log.flags &= (~flags);
    return old;
}

/**
 * fs_log_get_prefix - Default prefixes for logging levels
 * @level:  Log level to be prefixed
 *
 * Prefixing the logging output can make it easier to parse.
 *
 * Returns:  "string"  Prefix to be used
 */
static const char *fs_log_get_prefix(__u32 level)
{
    const char *prefix;

    switch (level)
    {
        case FS_LOG_LEVEL_DEBUG:
            prefix = "DBG: ";
            break;
        case FS_LOG_LEVEL_QUIET:
            prefix = "QUT: ";
            break;
        case FS_LOG_LEVEL_VERBOSE:
            prefix = "VBS: ";
            break;
        case FS_LOG_LEVEL_PROGRESS:
            prefix = "PRG: ";
            break;
        case FS_LOG_LEVEL_WARNING:
            prefix = "WRN! ";
            break;
        case FS_LOG_LEVEL_PERROR:
        case FS_LOG_LEVEL_ERROR:
            prefix = "ERR! ";
            break;
        case FS_LOG_LEVEL_CRITICAL:
            prefix = "CRTICAL! ";
            break;
        default:
            prefix = "";
            break;
    }

    return prefix;
}

/**
 * fs_log_redirect - Pass on the request to the real handler
 * @function:   Function in which the log line occurred
 * @file:   File in which the log line occurred
 * @line:   Line number on which the log line occurred
 * @level:  Level at which the line is logged
 * @data:   User specified data, possibly specific to a handler
 * @format: printf-style formatting string
 * @...:    Arguments to be formatted
 *
 * This is just a redirector function.  The arguments are simply passed to the
 * main logging handler (as defined in the global logging struct @fs_log).
 *
 * Returns:  -1  Error occurred
 *            0  Message wasn't logged
 *          num  Number of output characters
 */
int fs_log_redirect(const char *function, const char *file,
                    int line, __u32 level, void *data, const char *format, ...)
{
    int ret;
    va_list args;

    if ((fs_log.levels & FS_LOG_LEVEL_TRACEF) && (level &
            FS_LOG_LEVEL_TRACEX) && fs_log_funtbl.cnt)
    {
        int i;
        for (i = 0; i < FSLOG_MAX_FUNC_ITME; i++)
        {
            if (fs_log_funtbl.item[i].inuse)
            {
                if (!(eLIBs_strncmp(function, fs_log_funtbl.item[i].name, FSLOG_MAX_FUNNAME_LEN)))
                {
                    if (fs_log_funtbl.item[i].inuse > 0) /* +tracef item */
                        goto log;
                    else              /* -tracef item */
                        return 0;
                }
            }
        }
        /* ��ֹ����TRACEF */
        level &= ~FS_LOG_LEVEL_TRACEF;
    }
    if (!(fs_log.levels & level))       /* Don't log this message */
        return 0;

log:
    va_start(args, format);
    ret = fs_log.handler(function, file, line, level, data, format, args);
    va_end(args);

    return ret;
}

static char stack[32];
void __llint2str_dec(__u32 input_h, __u32 input_l, char *str)
{
    __s64 input = (0x00000000FFFFFFFF & ((__s64)input_l)) | (((__s64)input_h) << 32);
    char sign_flag = 0 ;      // 'sign_flag indicates wheater 'input' is positive or negative, default
    int i ;                           // value is 'POSITIVE'.
    int j ;

    if (input == 0)
    {
        str[0] = '0';
        str[1] = '\0';                   // 'str' must end with '\0'
        return ;
    }

    if (input < 0)                       // If 'input' is negative, 'input' is assigned to its absolute value.
    {
        sign_flag = 1 ;
        input = -input ;
    }

    for (i = 0; input > 0; ++i)
    {
        stack[i] = input % 10 + '0';    // characters in reverse order are put in 'stack' .
        input /= 10;
    }                                   // at the end of 'for' loop, 'i' is the number of characters.


    j = 0;
    if (sign_flag == 1)
        str[j++] = '-';                 // If 'input' is negative, minus sign '-' is placed in the head.
    for (--i  ; i >= 0; --i, ++j)
        str[j] = stack[i];
    str[j] = '\0';                      // 'str' must end with '\0'
}

void __lluint2str_dec(__u32 input_h, __u32 input_l, char *str)
{
    __u64 input = (((__u64)input_l) | (((__u64)input_h) << 32));
    int i ;
    int j ;

    if (input == 0)
    {
        str[0] = '0';
        str[1] = '\0';                   // 'str' must end with '\0'
        return ;
    }

    for (i = 0; input > 0; ++i)
    {
        stack[i] = input % 10 + '0';     // characters in reverse order are put in 'stack' .
        input /= 10;
    }                                    // at the end of 'for' loop, 'i' is the number of characters.

    for (--i, j = 0; i >= 0; --i, ++j)
        str[j] = stack[i];
    str[j] = '\0';                       // 'str' must end with '\0'
}

void __llint2str_hex(__u32 input_h, __u32 input_l, char *str, int hex_flag)
{
    __s64 input = (0x00000000FFFFFFFF & ((__s64)input_l)) | (((__s64)input_h) << 32);
    int i;
    int j;

    for (i = 0; i < 16; ++i)
    {
        if (hex_flag == HEX_X)
            stack[i] = "0123456789ABCDEF"[input & 0xf];
        else
            stack[i] = "0123456789abcdef"[input & 0xf];

        input >>= 4;
    }

    str[0] = '0';
    str[1] = hex_flag;                         // Hex format shoud start with "0x" or "0X".
    for (i = 15, j = 2; i >= 0; --i, ++j)
        str[j] = stack[i];
    str[j] = '\0';                             // 'str' must end with '\0'
}

static char string[32];
int __log_vsprintf(char *buffer, const char *str, va_list argp)
{
    char *p, *_p = buffer;
    __s32 hex_flag, ll_flag, gap_flag = 0;

    while (*str)
    {
        ll_flag = 0;
        if (line_begin)
        {
            _p[0] = 'f';
            _p[1] = 's';
            _p[2] = '>';
            _p[3] = ' ';
            _p += 4;
            line_begin = 0;
        }

        if (*str == '%')
        {
            if (str[1] >= '0' && str[1] <= '9') ++str;
            if (str[1] >= '0' && str[1] <= '9') ++str;
ll_check:
            ++str;
            p = string;
            hex_flag = HEX_X;
            switch (*str)
            {
                case 'd':
                case 'i':
                    if (ll_flag)
                        __llint2str_dec(va_arg(argp,  __s32),
                                        va_arg(argp,  __s32), string);
                    else
                    {
                        eLIBs_int2str_dec(va_arg(argp,  __s32), string);
                        gap_flag ^= 1;
                    }
                    eLIBs_strcpy(_p, p);
                    _p += eLIBs_strlen(p);
                    ++str;
                    break;
                case 'x':
                    hex_flag = HEX_x;          // jump to " case 'X' "
                case 'p':
                case 'X':
                    if (ll_flag)
                        __llint2str_hex(va_arg(argp,  __s32),
                                        va_arg(argp,  __s32), string, hex_flag);
                    else
                    {
                        eLIBs_int2str_hex(va_arg(argp,  __s32), string, hex_flag);
                        gap_flag ^= 1;
                    }
                    eLIBs_strcpy(_p, p);
                    _p += eLIBs_strlen(p);
                    ++str;
                    break;
                case 'u':
                    if (ll_flag)
                        __lluint2str_dec(va_arg(argp,  __u32),
                                         va_arg(argp,  __u32), string);
                    else
                    {
                        eLIBs_uint2str_dec(va_arg(argp,  __s32), string);
                        gap_flag ^= 1;
                    }
                    eLIBs_strcpy(_p, p);
                    _p += eLIBs_strlen(p);
                    ++str;
                    break;
                case 'c':
                    _p[0] = va_arg(argp,  __s32);
                    _p ++;
                    ++str;
                    gap_flag ^= 1;
                    break;
                case 's':
                    p =  va_arg(argp,  char *);
                    eLIBs_strcpy(_p, p);
                    _p += eLIBs_strlen(p);
                    ++str;
                    gap_flag ^= 1;
                    break;
                case 'l':
                    ++str;
                    if (*str == 'l')
                    {
                        /* para gap */
                        if (gap_flag) va_arg(argp,  __s32);
                        ll_flag = 1;
                        gap_flag = 0;
                    }
                    goto ll_check;
                default :
                    _p[0] = '%';
                    _p ++;
                    _p[0] = *str;
                    _p ++;
                    ++str;                        // let 'str' point to next character.
            }
        }

        else
        {
            if (*str == '\n')
                line_begin = 1;

            _p[0] = *str++;
            _p ++;
        }
    }

    _p[0] = '\0';

    return (_p - buffer);
}

int __log_sprintf(char *buffer, const char *str, ...)
{
    int res;
    va_list argp;

    va_start(argp, str);
    res = __log_vsprintf(buffer, str, argp);
    va_end(argp);

    return res;
}

/**
 * fs_log_handler_fprintf - Basic logging handler
 * @function:   Function in which the log line occurred
 * @file:   File in which the log line occurred
 * @line:   Line number on which the log line occurred
 * @level:  Level at which the line is logged
 * @data:   User specified data, possibly specific to a handler
 * @format: printf-style formatting string
 * @args:   Arguments to be formatted
 *
 * A simple logging handler.  This is where the log line is finally displayed.
 * It is more likely that you will want to set the handler to either
 * fs_log_handler_outerr or fs_log_handler_stderr.
 *
 * Note: For this handler, @data is a pointer to a FILE output stream.
 *       If @data is NULL, nothing will be displayed.
 *
 * Returns:  -1  Error occurred
 *            0  Message wasn't logged
 *          num  Number of output characters
 */
char stream[512];
int fs_log_handler_fprintf(const char *function, const char *file,
                           int line, __u32 level, void *data, const char *format, va_list args)
{
    int pos = 0;
    const char *col_prefix = NULL;
    const char *col_suffix = NULL;

    if ((fs_log.flags & FS_LOG_FLAG_ONLYNAME) &&
            (strchr(file, PATH_SEP)))       /* Abbreviate the filename */
        file = strrchr(file, PATH_SEP) + 1;

    if (fs_log.flags & FS_LOG_FLAG_PREFIX)  /* Prefix the output */
        pos += __log_sprintf(stream + pos, "%s", fs_log_get_prefix(level));

    if (level & (FS_LOG_LEVEL_PERROR | FS_LOG_LEVEL_ERROR))
    {
        pos += __log_sprintf(stream + pos, "%s ", file);
        pos += __log_sprintf(stream + pos, "%s(): ", function);
        pos += __log_sprintf(stream + pos, "(%d) ", line);
    }
    else
    {

        if (fs_log.flags & FS_LOG_FLAG_FILENAME)    /* Source filename */
            pos += __log_sprintf(stream + pos, "%s ", file);

        if ((fs_log.flags & FS_LOG_FLAG_FUNCTION))
            pos += __log_sprintf(stream + pos, "%s(): ", function);

        if (fs_log.flags & FS_LOG_FLAG_LINE)    /* Source line number */
            pos += __log_sprintf(stream + pos, "(%d) ", line);
    }

    if (format)
        pos += __log_vsprintf(stream + pos, format, args);

    if (level & FS_LOG_LEVEL_PERROR)
        pos += __log_sprintf(stream + pos, "[errno: %d]\n", fs_err);

    __inf(stream);

    return 0;
}

/**
 * fs_log_parse_option - Act upon command line options
 * @option: Option flag
 *
 * Delegate some of the work of parsing the command line.  All the options begin
 * with "--log-".  Options cause log levels to be enabled in @fs_log (the
 * global logging structure).
 *
 * Note: The "colour" option changes the logging handler.
 *
 * Returns:  TRUE  Option understood
 *          FALSE  Invalid log option
 */
BOOL fs_log_parse_option(const char *option)
{
    if (strcmp(option, "+debug") == 0 || strcmp(option, "+DEBUG") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_DEBUG);
        return TRUE;
    }
    else if (strcmp(option, "-debug") == 0 || strcmp(option, "-DEBUG") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_DEBUG);
        return TRUE;
    }
    else if (strcmp(option, "+verbose") == 0 || strcmp(option, "+VERBOSE") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_VERBOSE);
        return TRUE;
    }
    else if (strcmp(option, "-verbose") == 0 || strcmp(option, "-VERBOSE") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_VERBOSE);
        return TRUE;
    }
    else if (strcmp(option, "+quiet") == 0 || strcmp(option, "+QUIET") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_QUIET);
        return TRUE;
    }
    else if (strcmp(option, "-quiet") == 0 || strcmp(option, "-QUIET") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_QUIET);
        return TRUE;
    }
    else if (strcmp(option, "+trace0") == 0 || strcmp(option, "+TRACE0") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_TRACE0);
        return TRUE;
    }
    else if (strcmp(option, "-trace0") == 0 || strcmp(option, "-TRACE0") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_TRACE0);
        return TRUE;
    }
    else if (strcmp(option, "+trace1") == 0 || strcmp(option, "+TRACE1") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_TRACE1);
        return TRUE;
    }
    else if (strcmp(option, "-trace1") == 0 || strcmp(option, "-TRACE1") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_TRACE1);
        return TRUE;
    }
    else if (strcmp(option, "+trace2") == 0 || strcmp(option, "+TRACE2") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_TRACE2);
        return TRUE;
    }
    else if (strcmp(option, "-trace2") == 0 || strcmp(option, "-TRACE2") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_TRACE2);
        return TRUE;
    }
    else if (strcmp(option, "+trace01") == 0 || strcmp(option, "+TRACE01") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_TRACE01);
        return TRUE;
    }
    else if (strcmp(option, "-trace01") == 0 || strcmp(option, "-TRACE01") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_TRACE01);
        return TRUE;
    }
    else if (strcmp(option, "+objname") == 0 || strcmp(option, "+OBJNAME") == 0)
    {
        fs_log_set_levels(FS_LOG_LEVEL_OBJNAME);
        return TRUE;
    }
    else if (strcmp(option, "-objname") == 0 || strcmp(option, "-OBJNAME") == 0)
    {
        fs_log_clear_levels(FS_LOG_LEVEL_OBJNAME);
        return TRUE;
    }
    else if (strncmp(option, "+tracef:", 8) == 0 || strncmp(option, "+TRACEF:", 8) == 0)
    {
        int i;

        /* omit the -tracef item if exist */
        for (i = 0; i < FSLOG_MAX_FUNC_ITME; i++)
        {
            if (fs_log_funtbl.item[i].inuse)
            {
                if (!strncmp(fs_log_funtbl.item[i].name,
                             option + 8, FSLOG_MAX_FUNNAME_LEN))
                {
                    if (fs_log_funtbl.item[i].inuse < 0)
                    {
                        fs_log_funtbl.item[i].inuse = 0;
                        if (fs_log_funtbl.cnt)
                            fs_log_funtbl.cnt--;
                        if (!fs_log_funtbl.cnt)
                            fs_log_clear_levels(FS_LOG_LEVEL_TRACEF);
                        return TRUE;
                    }
                    else     /* inuse < 0 */
                    {
                        fs_log_warning("+tracef item already exist!\n");
                        return TRUE;
                    }

                }
            }
        }

        /* add a +tracef if max item count not reached */
        if (fs_log_funtbl.cnt >= FSLOG_MAX_FUNC_ITME)
        {
            fs_log_warning("max fun trace item reached!\n");
            return FALSE;
        }
        for (i = 0; i < FSLOG_MAX_FUNC_ITME; i++)
        {
            if (fs_log_funtbl.item[i].inuse == 0)
            {
                strncpy(fs_log_funtbl.item[i].name, option + 8,
                        FSLOG_MAX_FUNNAME_LEN);
                fs_log_funtbl.item[i].inuse = 1;
                fs_log_funtbl.cnt++;
                fs_log_set_levels(FS_LOG_LEVEL_TRACEF);
                return TRUE;
            }
        }
        fs_log_error("max fun trace item not reached, "
                     "but can't fount free item!\n");
        return FALSE;
    }
    else if (strncmp(option, "-tracef:", 8) == 0 || strncmp(option, "-TRACEF:", 8) == 0)
    {
        int i;

        /* omit the +tracef item if exist */
        for (i = 0; i < FSLOG_MAX_FUNC_ITME; i++)
        {
            if (fs_log_funtbl.item[i].inuse)
            {
                if (!strncmp(fs_log_funtbl.item[i].name,
                             option + 8, FSLOG_MAX_FUNNAME_LEN))
                {
                    if (fs_log_funtbl.item[i].inuse > 0)
                    {
                        fs_log_funtbl.item[i].inuse = 0;
                        if (fs_log_funtbl.cnt)
                            fs_log_funtbl.cnt--;
                        if (!fs_log_funtbl.cnt)
                            fs_log_clear_levels(FS_LOG_LEVEL_TRACEF);
                        return TRUE;
                    }
                    else     /* inuse < 0 */
                    {
                        fs_log_warning("item already muted!\n");
                        return TRUE;
                    }

                }
            }
        }

        /* add a -tracef if max item count not reached */
        if (fs_log_funtbl.cnt >= FSLOG_MAX_FUNC_ITME)
        {
            fs_log_warning("max fun trace item reached!\n");
            return FALSE;
        }
        for (i = 0; i < FSLOG_MAX_FUNC_ITME; i++)
        {
            if (!fs_log_funtbl.item[i].inuse)
            {
                strncpy(fs_log_funtbl.item[i].name, option + 8,
                        FSLOG_MAX_FUNNAME_LEN);
                fs_log_funtbl.item[i].inuse = -1;
                fs_log_funtbl.cnt++;
                fs_log_set_levels(FS_LOG_LEVEL_TRACEF);
                return TRUE;
            }
        }
        fs_log_error("max fun trace item not reached, "
                     "but can't fount free item!\n");
        return FALSE;
    }
    else if (strcmp(option, "traceinfo") == 0 || strcmp(option, "TRACEINFO") == 0)
    {
        fs_log_info("trace func count: %d\n", fs_log_funtbl.cnt);
        if (fs_log_funtbl.cnt)
        {
            int i;
            for (i = 0; i < FSLOG_MAX_FUNC_ITME; i++)
            {
                if (fs_log_funtbl.item[i].inuse)
                {
                    fs_log_info("  %d  -> %c%s\n", i,
                                fs_log_funtbl.item[i].inuse > 0 ? '+' : '-', fs_log_funtbl.item[i].name);
                }
            }
        }
        fs_log_info("dev trace: %s\n", (fs_log.levels & FS_LOG_LEVEL_TRACE2) ? "on" : "off");
        fs_log_info("part trace: %s\n", (fs_log.levels & FS_LOG_LEVEL_TRACE1) ? "on" : "off");
        fs_log_info("vfs trace: %s\n", (fs_log.levels & FS_LOG_LEVEL_TRACE0) ? "on" : "off");
        fs_log_info("fs trace: %s\n", (fs_log.levels & FS_LOG_LEVEL_TRACE01) ? "on" : "off");
        return TRUE;
    }
    else if (strcmp(option, "+line") == 0 || strcmp(option, "+LINE") == 0)
    {
        fs_log_set_flags(FS_LOG_FLAG_LINE);
        return TRUE;
    }
    else if (strcmp(option, "-line") == 0 || strcmp(option, "-LINE") == 0)
    {
        fs_log_clear_flags(FS_LOG_FLAG_LINE);
        return TRUE;
    }
    else if (strcmp(option, "+function") == 0 || strcmp(option, "+FUNCTION") == 0)
    {
        fs_log_set_flags(FS_LOG_FLAG_FUNCTION);
        return TRUE;
    }
    else if (strcmp(option, "-function") == 0 || strcmp(option, "-FUNCTION") == 0)
    {
        fs_log_clear_flags(FS_LOG_FLAG_FUNCTION);
        return TRUE;
    }

    __inf("Unknown logging option '%s'\n", option);
    return FALSE;
}



