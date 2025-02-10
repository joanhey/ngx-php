/*
==============================================================================
Copyright (c) 2016-2020, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================
*/

#include "ngx_http_php_module.h"
#include "ngx_http_php_util.h"

size_t
occurrences(const char *needle, const char *haystack) {
  if (NULL == needle || NULL == haystack) return -1;

  char *pos = (char *)haystack;
  size_t i = 0;
  size_t l = strlen(needle);
  if (l == 0) return 0;

  while ((pos = strstr(pos, needle))) {
    pos += l;
    i++;
  }

  return i;
}

char *
str_replace(const char *str, const char *sub, const char *replace)
{
    char *pos = (char *)str;
    int count = occurrences(sub, str);

    /* 如果没有匹配，直接返回原字符串的副本 */
    if (count <= 0) {
        return strdup(str);
    }

    /* 计算结果字符串所需的最大空间 */
    int size = (int)(strlen(str) - (strlen(sub) * count)
                     + (strlen(replace) * count) + 1);

    char *result = (char *)malloc(size);
    if (result == NULL) {
        return NULL;
    }
    memset(result, 0, size);

    char *current = NULL;
    int offset = 0; /* 记录当前已经写入 result 的位置 */

   /** 解决潜在的缓冲区溢出 **/
    while ((current = strstr(pos, sub)) != NULL) {
        int len = (int)(current - pos);
        snprintf(result + offset, size - offset, "%.*s", len, pos);
        offset += len;
        
        snprintf(result + offset, size - offset, "%s", replace);
        offset += (int)strlen(replace);

        pos = current + strlen(sub);
    }

    /* 把剩余未匹配部分直接写入结果字符串 */
    if (*pos != '\0') {
        snprintf(result + offset, size - offset, "%s", pos);
    }

    return result;
}

