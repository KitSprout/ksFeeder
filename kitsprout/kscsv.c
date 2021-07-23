/**
 *      __            ____
 *     / /__ _  __   / __/                      __  
 *    / //_/(_)/ /_ / /  ___   ____ ___  __ __ / /_ 
 *   / ,<  / // __/_\ \ / _ \ / __// _ \/ // // __/ 
 *  /_/|_|/_/ \__//___// .__//_/   \___/\_,_/ \__/  
 *                    /_/   github.com/KitSprout    
 * 
 *  @file    kscsv.c
 *  @author  KitSprout
 *  @brief   
 * 
 */

/* Includes --------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <direct.h>

#include "kslog.h"
#include "kscsv.h"

/* Define ----------------------------------------------------------------------------------*/
/* Macro -----------------------------------------------------------------------------------*/
/* Typedef ---------------------------------------------------------------------------------*/
/* Variables -------------------------------------------------------------------------------*/

const char KSCSV_VERSION[] = KSCSV_VERSION_DEFINE;

const char KSCSV_TAG_STRING[KSCSV_IDX_TOTAL][MAX_TAG_STRING_LENGTH] =
{
    "sn", "ts", "dt",
    "gx", "gy", "gz", "grx", "gry", "grz", "gbx", "gby", "gbz",
    "ax", "ay", "az", "arx", "ary", "arz", "abx", "aby", "abz",
    "mx", "my", "mz", "mrx", "mry", "mrz", "mbx", "mby", "mbz",
    "p", "t",
    "agx", "agy", "agz", "alx", "aly", "alz",
    "rvx", "rvy", "rvz", "rvw",
    "rvgx", "rvgy", "rvgz", "rvgw",
    "rvmx", "rvmy", "rvmz", "rvmw",

    "unk"   // unknown
};

/* Prototypes ------------------------------------------------------------------------------*/
/* Functions -------------------------------------------------------------------------------*/

/**
 *  @brief  kscsv_get_tag_index
 */
static int kscsv_get_tag_index(char *tag)
{
    int idx = KSCSV_IDX_TOTAL;
    while (idx)
    {
        if (strcmp(KSCSV_TAG_STRING[--idx], tag) == 0)
        {
            return idx;
        }
    }
    return KSCSV_IDX_UNKNOWN;
}

/**
 *  @brief  kscsv_get_tag_string
 */
static int kscsv_get_tag_string(char *tagstr, int index)
{
    if (index < KSCSV_IDX_UNKNOWN)
    {
        strcpy(tagstr, KSCSV_TAG_STRING[index]);
    }
    else
    {
        sprintf(tagstr, "%s[%d]", KSCSV_TAG_STRING[KSCSV_IDX_UNKNOWN], index % KSCSV_IDX_UNKNOWN);
    }

    return strlen(tagstr);
}

/**
 *  @brief  kscsv_get_tag
 */
static int kscsv_get_tag(FILE *file, int **tags)
{
    // get first line
    char line[MAX_FILE_LINE_STRING_LENGTH] = {0};
    rewind(file);
#if ENABLE_FGETS_INSTEAD_OF_FSCANF
    fgets(line, sizeof(line), file);
    line[strlen(line)-1] = '\0';
#else
    fscanf(file, "%s\n", line);
#endif

    // get number of tags
    int tagcnt = -1;
    char *pline = line;
    while (*pline != 0)
    {
        if (*pline == ',')
        {
            tagcnt++;
        }
        pline++;
    }
    tagcnt = (tagcnt < 0) ? 0 : tagcnt + 2;

    // update tag index
    if (*tags == NULL)
    {
        *tags = (int *)calloc(tagcnt, sizeof(int));
    }
    char *tag = (char *)strtok(line, ",");
    int *ptags = *tags;
    while (tag != NULL) 
    {
        *(ptags++) = kscsv_get_tag_index(tag);
        tag = (char *)strtok(NULL, ",");
    }

    return tagcnt;
}

/**
 *  @brief  kscsv_get_line_count
 */
static int kscsv_get_line_count(FILE *file)
{
    char line[MAX_FILE_LINE_STRING_LENGTH] = {0};
    int lens = 0;
    rewind(file);
    while (!feof(file))
    {
#if ENABLE_FGETS_INSTEAD_OF_FSCANF
        memset(line, 0, MAX_FILE_LINE_STRING_LENGTH);
        fgets(line, sizeof(line), file);
#else
        fscanf(file, "%s\n", line);
#endif
        if (strlen(line) < 2)
        {
            break;
        }
        lens++;
#if 0
        klogd("[%d] %s\n", lens, line);
#endif
    }
    // rewind(file);
    return lens;
}

/**
 *  @brief  kscsv_get_size_string
 */
static int kscsv_get_size_string(char *sizestr, uint64_t bytes)
{
    double size = bytes;
    int level = 0;
    while (level < 3)
    {
        if (size < 1024)
        {
            break;
        }
        size /= 1024;
        level++;
    }

    switch (level)
    {
        case 0: // Byte
        {
            sprintf(sizestr, "%.0f B", size);
            break;
        }
        case 1: // KByte
        {
            sprintf(sizestr, "%.3f KB", size);
            break;
        }
        case 2: // MByte
        {
            sprintf(sizestr, "%.3f MB", size);
            break;
        }
        case 3: // GByte
        {
            sprintf(sizestr, "%.3f GB", size);
            break;
        }
    }
    return level;
}

/**
 *  @brief  kscsv_register_unknown_tag
 */
static int kscsv_register_unknown_tag(int *tags, int tagcnt)
{
    int tagcntunk = 0;
    for (int i = 0; i < tagcnt; i++)
    {
        if (tags[i] == KSCSV_IDX_UNKNOWN)
        {
            tags[i] += tagcntunk++;
        }
    }
    return tagcntunk;
}

/**
 *  @brief  kscsv_raw_malloc
 */
static int kscsv_raw_malloc(kscsv_t *csv, int size)
{
    // memory allocation
    csv->raw.size = size;
    csv->raw.index = -1;
    if (csv->tagcntunk != 0)
    {
        csv->raw.unknown = (double **)calloc(csv->tagcntunk, sizeof(double));
    }
    for (int i = 0; i < csv->tagcnt; i++)
    {
        int idx = csv->tags[i];
        if (idx < KSCSV_IDX_UNKNOWN)
        {
            csv->mem[idx] = (unsigned int)calloc(size, sizeof(double));
            if (csv->mem[idx] == 0)
            {
                return KS_ERROR;
            }
        }
        else
        {
            int unkidx = idx % KSCSV_IDX_UNKNOWN;
            csv->raw.unknown[unkidx] = (double *)calloc(size, sizeof(double));
            if (csv->raw.unknown[unkidx] == NULL)
            {
                return KS_ERROR;
            }
        }
    }
    return KS_OK;
}

/**
 *  @brief  kscsv_raw_free
 */
static void kscsv_raw_free(kscsv_t *csv)
{
    for (int i = 0; i < csv->tagcntunk; i++)
    {
        free((double *)csv->raw.unknown[i]);
    }
    for (int i = 0; i < csv->tagcnt; i++)
    {
        free((double *)csv->mem[i]);
        csv->mem[i] = 0;
    }
}

/**
 *  @brief  kscsv_split_fullname
 */
static int kscsv_split_fullname(const char *string, char *path, char *name, char *type)
{
    // find last slash and last dot character
    int slash = -1, dot = -1;
    for (int i = 0; i < strlen(string); i++)
    {
        if ((string[i] == '/') || (string[i] == '\\'))
        {
            slash = i;
        }
        else if (string[i] == '.')
        {
            dot = i;
        }
    }

    if (dot < 0)
    {
        return KS_ERROR;
    }
    // path, name, type
    strcpy(type, &string[dot+1]);
    type[dot] = '\0';
    if (slash < 0)
    {
        sprintf(path, "./");
        strcpy(name, string);
        name[dot] = '\0';
    }
    else
    {
        strcpy(path, string);
        path[slash+1] = '\0';
        strcpy(name, &string[slash+1]);
        name[dot-slash-1] = '\0';
    }
    return KS_OK;
}

/**
 *  @brief  kscsv_open
 */
int kscsv_open(kscsv_t *csv, const char *filename)
{
    char fullname[MAX_PATH_STRING_LENGTH] = {0};
    if (kscsv_split_fullname(filename, csv->path, csv->name, csv->type) != KS_OK)
    {
        return KS_ERROR;
    }
    sprintf(fullname, "%s%s.%s", csv->path, csv->name, csv->type);

    csv->fp = fopen(fullname, "rb");
    if (csv->fp == NULL)
    {
        return KS_ERROR;
    }
    csv->lens = kscsv_get_line_count(csv->fp) - 1;
    csv->tagcnt = kscsv_get_tag(csv->fp, &csv->tags);
    csv->tagcntunk = kscsv_register_unknown_tag(csv->tags, csv->tagcnt);
    kscsv_raw_malloc(csv, csv->lens);
    return KS_OK;
}

/**
 *  @brief  kscsv_create
 */
int kscsv_create(kscsv_t *csv, const char *relatepath, const char *filetag, char **tag, int tagcnt)
{
    char res[2][MAX_PATH_STRING_LENGTH] = {0};
    char str[MAX_PATH_STRING_LENGTH] = {0};
    int enablerelate = KS_TRUE;

    // only write
    if (csv->fp == NULL)
    {
        enablerelate = KS_FALSE;
        if (kscsv_split_fullname(relatepath, csv->path, csv->name, csv->type) != KS_OK)
        {
            return KS_ERROR;
        }
    }

    if ((relatepath != NULL) && enablerelate)
    {
        strcpy(res[0], relatepath);
    }
    if (filetag != NULL)
    {
        strcpy(res[1], filetag);
    }
    sprintf(str, "%s%s", csv->path, res[0]);
    _mkdir(str);
    sprintf(str, "%s%s%s%s.%s", csv->path, res[0], csv->name, res[1], csv->type);

    csv->fpw = fopen(str, "wb");
    if (csv->fpw == NULL)
    {
        return KS_ERROR;
    }

    char *pstr = str;
    int lens = 0;
    lens = sprintf(pstr, "%s", tag[0]);
    for (int i = 1; i < tagcnt; i++)
    {
        lens += sprintf(pstr+lens, ",%s", tag[i]);
    }
    *(pstr+lens) = '\0';
    fputs(str, csv->fpw);

    return KS_OK;
}

/**
 *  @brief  kscsv_close
 */
int kscsv_close(kscsv_t *csv)
{
    free(csv->tags);
    csv->tags = NULL;
    kscsv_raw_free(csv);
    if (csv->fpw != NULL)
    {
        fclose(csv->fpw);
    }
    return fclose(csv->fp);
}

/**
 *  @brief  kscsv_read_single
 */
static int kscsv_read_single(kscsv_t *csv, int index)
{
    if (index >= csv->raw.size)
    {
        return KS_ERROR;
    }
    csv->raw.index = (index != 0) ? index : csv->raw.index + 1;

    int idx, unkidx;

    // read data
    for (int i = 0; i < csv->tagcnt-1; i++)
    {
        idx = csv->tags[i];
        if (idx < KSCSV_IDX_UNKNOWN)
        {
            fscanf(csv->fp, "%lf,", &((double*)csv->mem[idx])[csv->raw.index]);
        }
        else
        {
            unkidx = idx % KSCSV_IDX_UNKNOWN;
            fscanf(csv->fp, "%lf,", &csv->raw.unknown[unkidx][csv->raw.index]);
        }
    }

    idx = csv->tags[csv->tagcnt-1];
    if (idx < KSCSV_IDX_UNKNOWN)
    {
        fscanf(csv->fp, "%lf\n", &((double*)csv->mem[idx])[csv->raw.index]);
    }
    else
    {
        int unkidx = idx % KSCSV_IDX_UNKNOWN;
        fscanf(csv->fp, "%lf\n", &csv->raw.unknown[unkidx][csv->raw.index]);
    }
    return KS_OK;
}

/**
 *  @brief  kscsv_read
 */
int kscsv_read(kscsv_t *csv, int lens)
{
    if (feof(csv->fp))
    {
        return KS_ERROR;
    }

    // start read
    int idx = 0;
    if (lens == 0)
    {
        // single read
        kscsv_read_single(csv, 0);
    }
    else if (lens < 0)
    {
        // continuous read (all)
        do
        {
            kscsv_read_single(csv, idx++);
        }
        while (!feof(csv->fp));
    }
    else
    {
        // continuous read (lens)
        do
        {
            kscsv_read_single(csv, idx);
            if (++idx >= lens)
            {
                break;
            }
        }
        while (!feof(csv->fp));
    }

    return KS_OK;
}

/**
 *  @brief  kscsv_write
 */
int kscsv_write(kscsv_t *csv, const char *fmt, ...)
{
    int lens = 0;
    char str[MAX_PATH_STRING_LENGTH] = {0};
    char *pstr = str;
    *pstr++ = '\n';
    va_list aptr;
    va_start(aptr, fmt);
    lens = vsprintf(pstr, fmt, aptr);
    va_end(aptr);
    fputs(str, csv->fpw);

    return lens;
}

/**
 *  @brief  kscsv_info
 */
void kscsv_info(kscsv_t *csv)
{
    char str[MAX_FILE_LINE_STRING_LENGTH] = {0};
    uint64_t memsize = csv->tagcnt * csv->raw.size * sizeof(double);
    klogd("\n");
    klogd(">> csv information\n");
    sprintf(str, "%s%s.%s", csv->path, csv->name, csv->type);
    klogd("fullanme    %s\n", str);
    klogd("tags        %d (unknown = %d)  >>>  ", csv->tagcnt, csv->tagcntunk);
    for (int i = 0; i < csv->tagcnt-1; i++)
    {
        kscsv_get_tag_string(str, csv->tags[i]);
        klogd("%s,", str);
    }
    kscsv_get_tag_string(str, csv->tags[csv->tagcnt-1]);
    klogd("%s\n", str);
    klogd("data        %d x tags\n", csv->lens);
    kscsv_get_size_string(str, memsize);
    klogd("memory      %s (%d x tags x 8)\n", str, csv->raw.size);
}

/*************************************** END OF FILE ****************************************/
