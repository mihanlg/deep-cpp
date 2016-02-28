#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define BUFSIZE 255

int main(int argc, const char * argv[]) {
    int s = 0;
    size_t len = 0, size = 0, capacity = 0;
    char *str = NULL, buf[BUFSIZE+1];
    memset(buf, 0, (BUFSIZE+1)*sizeof(char));
    char sh[5] = "%";
    sprintf(sh+1, "%ds", BUFSIZE);
    do
    {
        memset(buf, 0, (BUFSIZE)*sizeof(char));
        s = scanf(sh, buf);
        len = strlen(buf);
        if (size == capacity)
        {
            size_t newcapacity = capacity == 0 ? BUFSIZE : 2*capacity;
            char *strtmp = realloc(str, newcapacity);
            if (strtmp == NULL) {/*ERROR*/}
            str = strtmp;
            capacity = newcapacity;
        }
        memcpy(str+size, buf, len*sizeof(char));
        size += len;
    } while (len == BUFSIZE && s != EOF);
    str[size] = '\0';
    printf("%s\n", str);
    free(str);
    return 0;
}