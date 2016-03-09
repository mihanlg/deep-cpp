#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
//#include <errno.h>

#define BUFSIZE 32
#define TPLSIZE 10
#define NCCOEF 2
#define ERRMSG "[error]"
#define DIVO "<div>"
#define DIVC "</div>"
#define DIVOLEN 5
#define DIVCLEN 6
#define IND 4

void error(void) {
    printf("%s\n", ERRMSG);
}

void free_struct(char** structured) {
    if(structured != NULL) {
        size_t i = 0;
        while(structured[i] != NULL) {
            free(structured[i++]);
        }
        free(structured);
    }
}

char* resize_str(char *str, const size_t size, size_t *capacity) {
    if (size + BUFSIZE >= *capacity) {
        size_t newcapacity = *capacity == 0 ? 8*BUFSIZE : 2*(*capacity);
        char* tmpstr = (char*)realloc(str, newcapacity*sizeof(char));
        if (tmpstr == NULL) {
            free(str);
            return NULL;
        }
        str = tmpstr;
        *capacity = newcapacity;
    }
    return str;
}

char** resize_struct(char **structured, const size_t size, size_t *capacity) {
    if (size + BUFSIZE >= *capacity) {
        size_t newcapacity = *capacity == 0 ? 2*BUFSIZE : 2*(*capacity);
        char **tmpstruct = (char**)realloc(structured, newcapacity*sizeof(char*));
        if (tmpstruct == NULL) {
            free_struct(structured);
            return NULL;
        }
        structured = tmpstruct;
        *capacity = newcapacity;
    }
    return structured;
}

char* read_input(void) {
    char* input = NULL;
    size_t size = 0, capacity = 0;
    char buf[BUFSIZE];
    memset(buf, (char)0, BUFSIZE*sizeof(char));
    while (fgets(buf, BUFSIZE, stdin) != NULL) {
        if ((input = resize_str(input, size, &capacity)) == NULL) return NULL;
        memcpy(input+size, buf, BUFSIZE*sizeof(char));
        size += strlen(buf);
        memset(buf, (char)0, BUFSIZE*sizeof(char));
    }
    return input;
}

ptrdiff_t ptrcmp(char *ptr1, char *ptr2) {
    return ptr1 - ptr2;
}

char* strdup_alloc(char *str, size_t len) {
    char *newstr = malloc((len+1)*sizeof(char));
    if (newstr == NULL) return NULL;
    memcpy(newstr, str, len*sizeof(char));
    newstr[len] = (char)0;
    return newstr;
}

char* trim(char *str, size_t len) {
    char *p = str;
    while (isspace(p[len-1])) --len;
    while (len && isspace(*p)) ++p, --len;
    return len == 0 ? str : strdup_alloc(p, len);
}

char** struct_input(char* input) {
    char** structured = NULL;
    size_t size = 0, capacity = 0;
    char *ptr, *ptrO, *ptrC, *end;
    ptr = ptrO = ptrC = NULL;
    end = input + strlen(input);
    ptrdiff_t step = 0;
    
    while(ptrcmp(input, end) < 0) {
        ptrO = strstr(input, DIVO);
        ptrC = strstr(input, DIVC);
        if ((ptrcmp(ptrO, ptrC) < 0) && ptrO != NULL) {
            ptr = ptrO, step = DIVOLEN;
        }
        else if (ptrC != NULL) {
            ptr = ptrC, step = DIVCLEN;
        }
        else ptr = end;
        
        char *tmp = NULL;
        //ASCI-string before tag
        if (input != ptr && (tmp = trim(input, ptr-input)) != input) {
            if (tmp == NULL || (structured = resize_struct(structured, size, &capacity)) == NULL) {
                free(tmp);
                free_struct(structured);
                return NULL;
            }
            structured[size++] = tmp;
        }
        //TAG
        if (ptr != end) {
            tmp = strdup_alloc(ptr, step);
            if (tmp == NULL || (structured = resize_struct(structured, size, &capacity)) == NULL) {
                free(tmp);
                free_struct(structured);
                return NULL;
            }
            structured[size++] = tmp;
        }
        //NEXT ITERATION
        input = ptr + step;
    }
    return structured;
}

char** div_format(char** structured) {
    if (structured == NULL) return NULL;
    char **formated = NULL;
    size_t size = 0, capacity = 0;
    int stsize = 0;
    while (structured[size] != NULL) {
        if(strncmp(structured[size], DIVC, DIVCLEN) == 0) stsize--;
        if((stsize < 0) || (formated = resize_struct(formated, size, &capacity)) == NULL) {
            free_struct(formated);
            return NULL;
        }
        size_t len = strlen(structured[size]);
        size_t offset = IND*stsize;
        if ((formated[size] = (char *)malloc((offset+len+1)*sizeof(char))) == NULL) {
            free_struct(formated);
            return NULL;
        }
        memset(formated[size], ' ', offset*sizeof(char));
        memcpy(formated[size]+offset, structured[size], (len+1)*sizeof(char));
        if(strncmp(structured[size], DIVO, DIVOLEN) == 0) stsize++;
        size++;
    }
    if((formated = resize_struct(formated, size, &capacity)) == NULL) {
        free_struct(formated);
        return NULL;
    }
    formated[size] = NULL;
    if (stsize != 0) {
        free_struct(formated);
        return NULL;
    }
    return formated;
}

void print_struct(char** structured) {
    if(structured != NULL) {
        size_t i = 0;
        while(structured[i] != NULL) {
            printf("%s\n", structured[i++]);
        }
    }
}

int main(int argc, const char * argv[]) {
    char *input = NULL;
    if ((input = read_input()) == NULL) {
        error(); //memory allocation fail
        return 0;
    }
    //printf("%s\n", input);
    
    char **structured = struct_input(input);
    free(input);
    if (structured == NULL) {
        error(); //memory allocation fail
        return 0;
    }
    
    char **formated = div_format(structured);
    free_struct(structured);
    if (formated == NULL) {
        error(); //wrong format or memory allocation fail
        return 0;
    }
    print_struct(formated);
    free_struct(formated);
    return 0;
}
