#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>

#define BUFSIZE 32
#define TPLSIZE 10
#define NCCOEF 2
#define ERRMSG "[error]"
#define DIVO "<div>"
#define DIVOLEN 5
#define DIVC "</div>"
#define DIVCLEN 6
#define IND 4

char* resize_str(char *str, size_t size, size_t capacity) {
    size_t len = strlen(str);
    if (size + len >= capacity) {
        size_t newcapacity = capacity == 0 ? BUFSIZE : NCCOEF*capacity;
        char *strtmp = (char *)realloc(str, newcapacity*sizeof(char));
        if (strtmp == NULL) { return NULL; }
        str = strtmp;
        capacity = newcapacity;
    }
    return str;
}

char* read_text() {
    char *str = NULL, buf[BUFSIZE];
    size_t size = 0, capacity = 0, len = 0;
    while (fgets(buf, BUFSIZE, stdin) != NULL) {
        len = strlen(buf);
        str = resize_str(str, size, capacity);
        if (size + len >= capacity) {
            size_t newcapacity = capacity == 0 ? BUFSIZE : NCCOEF*capacity;
            char *strtmp = realloc(str, newcapacity);
            if (strtmp == NULL) {
                free(str);
                return NULL;
            }
            str = strtmp;
            capacity = newcapacity;
        }
        memcpy(str+size, buf, len*sizeof(char));
        size += len;
        str[size] = '\0';
    }
    return str;
}

char* alloc(char *str, size_t len) {
    char *dynstr = (char *)malloc((len+1)*sizeof(char));
    if (dynstr == NULL) return NULL;
    memcpy(dynstr, str, len*sizeof(char));
    dynstr[len] = (char)0;
    return dynstr;
}

char* trim(char *str, size_t len) {
    char *p = str;
    while (isspace(p[len - 1]))
        --len;
    while (len && isspace(*p))
        ++p, --len;
    return len == 0 ? str : alloc(p, len);
}

ptrdiff_t ptrcmp(char *ptr1, char *ptr2) {
    return ptr1 - ptr2;
}

void free_text(char **arr) {
    size_t i = 0;
    if (arr != NULL) while (arr[i] != NULL) free(arr[i++]);
    free(arr);
}

char** resize_text(char **text, size_t size, size_t capacity) {
    if (size == capacity) {
        size_t newcapacity = capacity == 0 ? BUFSIZE : NCCOEF*capacity;
        char **texttmp = (char **)realloc(text, newcapacity*sizeof(char *));
        if (texttmp == NULL) { return NULL; }
        text = texttmp;
        capacity = newcapacity;
    }
    return text;
}

char** divide_text(char *str) {
    char *ptr, *ptr1, *ptr2, *start, *end;
    ptr = ptr1 = ptr2 = NULL;
    start = str;
    end = str + strlen(str);
    ptrdiff_t step = 0;
    char **text = NULL;
    size_t size = 0, capacity = 0;
    while (ptrcmp(start, end) < 0) {
        //Choose next pointer to <div> or </div> block, else point on the end;
        ptr1 = strstr(start, DIVO);
        ptr2 = strstr(start, DIVC);
        if ((ptrcmp(ptr1, ptr2) < 0) && ptr1 != NULL) {
            ptr = ptr1;
            step = DIVOLEN;
        }
        else if (ptr2 != NULL) {
            ptr = ptr2;
            step = DIVCLEN;
        }
        else ptr = end;
        //ASCI-str before any tag, or at the end of text
        char *tmp = NULL;
        if (start != ptr && (tmp = trim(start, ptr-start)) != start) {
            if (tmp == NULL || (text = resize_text(text, size, capacity)) == NULL) {
                free_text(text);
                return NULL;
            }
            text[size++] = tmp;
        }
        //Tag
        if (ptr != end) {
            tmp = alloc(ptr, step);
            if (tmp == NULL || (text = resize_text(text, size, capacity)) == NULL) {
                free_text(text);
                return NULL;
            }
            text[size++] = tmp;
        }
        //Next iteration
        start = ptr + step;
    }
    //Add NULL at the end
    if ((text = resize_text(text, size, capacity)) == NULL) { free_text(text); return NULL; }
    text[size] = NULL;
    return text;
}

char** div_format(char **text) {
    char **newtext = NULL;
    size_t size = 0, capacity = 0, i = 0;
    int stsize = 0;
    while (text[i] != NULL) {
        if(strcmp(text[i], DIVC) == 0) stsize--;
        if((stsize < 0) || (newtext = resize_text(newtext, size, capacity)) == NULL) {
            free_text(newtext);
            return NULL;
        }
        size_t len = strlen(text[i]);
        size_t offset = IND*stsize;
        if ((newtext[i] = malloc((offset+len+1)*sizeof(char))) == NULL) {
            free_text(newtext);
            return NULL;
        }
        memset(newtext[i], ' ', offset*sizeof(char));
        memcpy(newtext[i]+offset, text[i], len*sizeof(char));
        newtext[i][offset+len] = (char)0;
        if(strcmp(text[i], DIVO) == 0) stsize++;
        i++;
    }
    if((newtext = resize_text(newtext, size, capacity)) == NULL) {
        free_text(newtext);
        return NULL;
    }
    newtext[i] = NULL;
    if (stsize != 0) {
        free_text(newtext);
        return NULL;
    }
    return newtext;
}

int error(void) {
    printf("%s\n", ERRMSG);
    return 0;
}

int main(int argc, const char * argv[]) {
    char *str = NULL, **text, **mtext;
    text = mtext = NULL;
    if ((str = read_text()) == NULL) return 0;
    if ((text = divide_text(str)) == NULL) { free(str); return error(); }
    free(str);
    if ((mtext = div_format(text)) == NULL) { free_text(text); return error(); }
    size_t i = 0;
    while (mtext[i] != NULL) {
        printf("%s\n", mtext[i++]);
    }
    free_text(mtext);
    free_text(text);
    return 0;
}









