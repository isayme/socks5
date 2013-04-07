#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "liblog.h"
#include "libconfig.h"

INT32 get_cfg_from_file(char *key, char *value, INT32 value_len, char *cfg_path)
{
    FILE *fp = NULL;
    char *line = NULL;
    char *real_line = NULL;
    char *tmp = NULL;
    size_t len = 0;
    size_t read;

    if (NULL == key || NULL == value || 0 == value_len || NULL == cfg_path)
    {
        PRINTF(LEVEL_ERROR, "%s argument error.\n", __func__);
        return R_ERROR;
    }

    memset(value, 0, value_len);
    fp = fopen(cfg_path, "rb");
    if (NULL == fp)
    {
        PRINTF(LEVEL_ERROR, "%s fopen error, path = [%s].\n", __func__, cfg_path);
        return R_ERROR;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        PRINTF(LEVEL_DEBUG, "%d:[%s]\n", read, line);

        real_line = line;

        // 去除行首空格/制表符等无意义字符
        while (0 != isspace((INT32)*(real_line))) real_line++;

        // 注释行，跳过
        if ('#' == real_line[0])
            continue;

        tmp = strstr(real_line, key);

        // 此行无key，跳过
        if (NULL == tmp)
            continue;

        // 跳过key，准备计算value值
        real_line = tmp + strlen(key);

        // 去除等号左测空格/制表符等无意义字符
        while (0 != isspace((INT32)*(real_line))) real_line++;
        // 必须是等号，是则跳过等号，否则跳过整行
        if ('=' != real_line[0])
            continue;
        else
            real_line++;
        // 去除等号右测空格/制表符等无意义字符
        while (0 != isspace((INT32)*(real_line))) real_line++;
        
        // 统计有效字符长度
        tmp = real_line;
        while (0 != isprint((INT32)*(real_line)) && 0 == isspace((INT32)*(real_line))) real_line++;
        
        if ((real_line == tmp) || ((real_line - tmp) > value_len))
        {
            PRINTF(LEVEL_WARNING, "value buf not enough or no valid value exsit.\n");
            continue;
        }

        strncpy(value, tmp, real_line - tmp);
        value[real_line - tmp] = '\0';

        PRINTF(LEVEL_DEBUG, "key:%s\tvalue:%s\n", key, value);
        fclose(fp);
        if (line)
            free(line);
        return R_OK;
    }

    fclose(fp);
    if (line)
        free(line);

    return R_ERROR;
}
