#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>

#define BUFSIZE 32
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

void free_struct(char** structured, size_t size) {
    if(structured != NULL) {
        size_t i = 0;
        while(i != size) {
            free(structured[i++]);
        }
        free(structured);
    }
}

char* resize_str(char *str, const size_t size, size_t *capacity) {
    if (size + BUFSIZE >= *capacity) {
        size_t newcapacity = *capacity == 0 ? NCCOEF*BUFSIZE : NCCOEF*(*capacity);
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
        size_t newcapacity = *capacity == 0 ? NCCOEF*BUFSIZE : NCCOEF*(*capacity);
        char **tmpstruct = (char**)realloc(structured, newcapacity*sizeof(char*));
        if (tmpstruct == NULL) {
            free_struct(structured, size);
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
    if ((input = resize_str(input, size, &capacity)) == NULL) return NULL;
    input[size] = (char)0;
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

char** struct_input(char* input, size_t *sz) {
    char** structured = NULL;
    size_t size = 0, capacity = 0;
    char *ptr, *ptrO, *ptrC, *end;
    ptr = ptrO = ptrC = NULL;
    //using strtok to divide lines
    char *sep_input = strtok(input, "\n");
    while (sep_input != NULL) {
        end = sep_input + strlen(sep_input);
        ptrdiff_t step = 0;
        while(ptrcmp(sep_input, end) < 0) {
            //finding first of <div> or </div> tags
            ptrO = strstr(sep_input, DIVO);
            ptrC = strstr(sep_input, DIVC);
            if (((ptrcmp(ptrO, ptrC) < 0) || ptrC == NULL) && ptrO != NULL) {
                ptr = ptrO, step = DIVOLEN;
            }
            else if (ptrC != NULL) {
                ptr = ptrC, step = DIVCLEN;
            }
            else ptr = end;
            
            //ASCI-string before tag
            char *tmp = NULL;
            if (sep_input != ptr && (tmp = trim(sep_input, ptr-sep_input)) != sep_input) {
                if (tmp == NULL || (structured = resize_struct(structured, size, &capacity)) == NULL) {
                    free(tmp);
                    free_struct(structured, size);
                    return NULL;
                }
                structured[size++] = tmp;
            }
            //TAG
            if (ptr != end) {
                tmp = strdup_alloc(ptr, step);
                if (tmp == NULL || (structured = resize_struct(structured, size, &capacity)) == NULL) {
                    free(tmp);
                    free_struct(structured, size);
                    return NULL;
                }
                structured[size++] = tmp;
            }
            //NEXT ITERATION
            sep_input = ptr + step;
        }
        sep_input = strtok (NULL, "\n");
    }
    //Add NULL to the end
    if((structured = resize_struct(structured, size, &capacity)) == NULL) {
        free_struct(structured, size);
        return NULL;
    }
    structured[size] = NULL;
    *sz = size;
    return structured;
}

char** div_format(char** structured, size_t sz) {
    if (structured == NULL) return NULL;
    char **formated = NULL;
    size_t size = 0, capacity = 0;
    int stsize = 0;
    //format structured text
    while (size != sz) {
        //move left if </div> met
        if(strncmp(structured[size], DIVC, DIVCLEN) == 0) stsize--;
        //check stack of tags
        if((stsize < 0) || (formated = resize_struct(formated, size, &capacity)) == NULL) {
            free_struct(formated, size);
            return NULL;
        }
        size_t len = strlen(structured[size]);
        size_t offset = IND*stsize;
        //put str
        if ((formated[size] = (char *)malloc((offset+len+1)*sizeof(char))) == NULL) {
            free_struct(formated, size);
            return NULL;
        }
        memset(formated[size], ' ', offset*sizeof(char));
        memcpy(formated[size]+offset, structured[size], (len+1)*sizeof(char));
        //move right if <div> met
        if(strncmp(structured[size], DIVO, DIVOLEN) == 0) stsize++;
        size++;
    }
    //Add NULL to the end
    if((formated = resize_struct(formated, size, &capacity)) == NULL) {
        free_struct(formated, size);
        return NULL;
    }
    formated[size] = NULL;
    //Check stack size == 0
    if (stsize != 0) {
        free_struct(formated, size);
        return NULL;
    }
    return formated;
}

void print_struct(char** structured, size_t size) {
    if(structured != NULL) {
        size_t i = 0;
        while(i != size) {
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
    
    size_t size = 0; //save size of structured text to this variable
    char **structured = struct_input(input, &size);
    free(input);
    if (structured == NULL) {
        error(); //memory allocation fail
        return 0;
    }
    
    char **formated = div_format(structured, size);
    free_struct(structured, size);
    if (formated == NULL) {
        error(); //wrong format or memory allocation fail
        return 0;
    }
    print_struct(formated, size);
    free_struct(formated, size);
    return 0;
}
