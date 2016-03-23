#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 32
#define NCCOEF 2
#define ERRMSG "[error]"

char cset[] = "1234567890";

struct stack {
    char c;
    struct stack *next;
};

struct vector {
    size_t dim;
    long long *vec;
};

struct stack_vec {
    struct vector v;
    struct stack_vec *next;
};

//========================[READ INPUT FUNCTIONS]==================================

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

char* read_input() {
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

//===========================[STACK FUNCTIONS]====================================

struct stack *push(struct stack *head, char c, size_t *err)
{
    struct stack *ptr = NULL;
    if((ptr = malloc(sizeof(struct stack))) == NULL) { *err = 1; return head; }
    ptr->c = c;
    ptr->next = head;
    return ptr;
}

char pop(struct stack **head)
{
    struct stack *ptr;
    if(*head == NULL) return '\0';
    ptr = *head;
    char a = ptr->c;
    *head = ptr->next;
    free(ptr);
    return a;
}


struct stack_vec *push_vec(struct stack_vec *head, struct vector v, size_t *err) {
    struct stack_vec *ptr;
    if((ptr = malloc(sizeof(struct stack_vec))) == NULL) {*err = 1; return head; }
    ptr->v = v;
    ptr->next = head;
    return ptr;
}

struct vector pop_vec(struct stack_vec **head) {
    struct stack_vec *ptr;
    struct vector vec;
    vec.dim = 0;
    vec.vec = NULL;
    if(*head == NULL) return vec;
    ptr = *head;
    vec = ptr->v;
    *head = ptr->next;
    free(ptr);
    return vec;
}

//========================================[SCAN STRING FUNCTIONS]================================================

int getsym(char **ptr) {
    while (isspace(**ptr)) (*ptr)++;
    return *((*ptr)++);
}

void ungetsym(char **ptr) {
    (*ptr)--;
}

//================================================[LEX]==========================================================

size_t prior(char c) {
    switch (c) {
        case '*':
            return 4;
        case '#':
            return 3;
        case '+':
        case '-':
            return 2;
        case '(':
            return 1;
        default:
            return 0;
    }
}

void scan_expr(char **ptr, char *mdf, size_t *len, size_t *err); void scan_op(char **ptr, char *mdf, size_t *len, size_t *err);
void scan_numb(char **ptr, char *mdf, size_t *len, size_t *err); void scan_vec(char **ptr, char *mdf, size_t *len, size_t *err);

void scan_expr(char **ptr, char *mdf, size_t *len, size_t *err) {
    if (!*err) {
        char c = getsym(ptr);
        if (isdigit(c)) {
            ungetsym(ptr);
            scan_numb(ptr, mdf, len, err);
            if ((c = getsym(ptr)) == '*') {mdf[(*len)++] = '#'; scan_expr(ptr, mdf, len, err);}
            else { *err = 1; return; }
        }
        else if (c == '{') {
            ungetsym(ptr);
            scan_vec(ptr, mdf, len, err); scan_op(ptr, mdf, len, err);
        }
        else if (c == '(') {
            mdf[(*len)++] = c;
            scan_expr(ptr, mdf, len, err);
            if ((c = getsym(ptr)) == ')') {
                mdf[(*len)++] = c;
                scan_op(ptr, mdf, len, err);
            }
            else { *err = 1; return; }
        }
        else { *err = 1; return; }
    }
}

void scan_op(char **ptr, char *mdf, size_t *len, size_t *err) {
    if (!*err) {
        char c = getsym(ptr);
        if (c == '+') {
            mdf[(*len)++] = c;
            scan_expr(ptr, mdf, len, err);
        }
        else if (c == '-') {
            mdf[(*len)++] = c;
            scan_expr(ptr, mdf, len, err);
        }
        else if (c == '*') {
            mdf[(*len)++] = c;
            scan_numb(ptr, mdf, len, err);
            scan_op(ptr, mdf, len, err);
        }
        else ungetsym(ptr);
    }
}

void scan_vec(char **ptr, char *mdf, size_t *len, size_t *err) {
    if (!*err) {
        char c = getsym(ptr);
        if (c == '{') {
            mdf[(*len)++] = c;
            scan_numb(ptr, mdf, len, err);
            if ((c = getsym(ptr)) == ',') {mdf[(*len)++] = c; scan_numb(ptr, mdf, len, err); }
            else { *err = 1; return; }
            while ((c = getsym(ptr)) != '}') {
                if (c == ',') { mdf[(*len)++] = c; scan_numb(ptr, mdf, len, err); }
                else { *err = 1; return; }
            }
            mdf[(*len)++] = c;
        }
        else { *err = 1; return; }
    }
}

void scan_numb(char **ptr, char *mdf, size_t *len, size_t *err) {
    if (!*err) {
        char c = getsym(ptr);
        if (isdigit(c)) {
            mdf[(*len)++] = c;
            while (isdigit(c = getsym(ptr))) { mdf[(*len)++] = c; }
            ungetsym(ptr);
        }
        else { *err = 1; return; }
    }
}

//================================================[EXECUTE]======================================================

size_t max_st(size_t n1, size_t n2) {
    return n1 > n2 ? n1 : n2;
}

size_t min_st(size_t n1, size_t n2) {
    return n1 < n2 ? n1 : n2;
}

struct vector mult(struct vector v1, struct vector v2) {
    struct vector vec;
    if (v2.dim == 1) {
        struct vector tmp = v1;
        v1 = v2;
        v2 = tmp;
    }
    vec.dim = v2.dim;
    vec.vec = malloc(sizeof(long long) * vec.dim);
    for (size_t i = 0; i < v2.dim; ++i) {
        vec.vec[i] = v1.vec[0] * v2.vec[i];
    }
    free(v1.vec); free(v2.vec);
    return vec;
}

struct vector add(struct vector v1, struct vector v2) {
    struct vector vec;
    if (v2.dim < v1.dim) {
        struct vector tmp = v1;
        v1 = v2;
        v2 = tmp;
    }
    vec.dim = v2.dim;
    vec.vec = malloc(sizeof(long long) * vec.dim);
    for (size_t i = 0; i < v1.dim; ++i) {
        vec.vec[i] = v1.vec[i] + v2.vec[i];
    }
    for (size_t i = v1.dim; i < vec.dim; ++i) {
        vec.vec[i] = v2.vec[i];
    }
    free(v1.vec); free(v2.vec);
    return vec;
}

struct vector sub(struct vector vv, struct vector vu) {
    struct vector vec;
    size_t max = max_st(vv.dim, vu.dim);
    size_t min = min_st(vv.dim, vu.dim);
    vec.dim = max;
    vec.vec = malloc(sizeof(long long) * vec.dim);
    for (size_t i = 0; i < min; ++i) {
        vec.vec[i] = vu.vec[i] - vv.vec[i];
    }
    if (vv.dim == min)
        for (size_t i = min; i < vec.dim; ++i) {
            vec.vec[i] = vu.vec[i];
        }
    else
        for (size_t i = min; i < vec.dim; ++i) {
            vec.vec[i] = -vv.vec[i];
        }
    free(vv.vec); free(vu.vec);
    return vec;
}

struct vector wrap_number(long long num, size_t *err) {
    struct vector vec;
    vec.dim = 0; vec.vec = NULL;
    long long *number = malloc(sizeof(long long));
    if (number == NULL) { *err = 1; return vec; }
    *number = num;
    vec.dim = 1;
    vec.vec = number;
    return vec;
    
}

long long get_number(char **ptr) {
    long long number = 0;
    sscanf(*ptr, "%lld", &number);
    *ptr += strspn(*ptr, cset);
    return number;
}

struct vector get_vec(char **ptr, size_t *err) {
    char c;
    struct vector vec;
    vec.dim = 0;
    vec.vec = NULL;
    long long *list = NULL;
    size_t size = 0; size_t capacity = 0;
    if ((c = getsym(ptr)) == '{') {
        long long num = 0;
        while (c != '}' && c != '\0') {
            sscanf(*ptr, "%lld", &num);
            if (size == capacity) {
                size_t newcapacity = capacity == 0 ? BUFSIZE : NCCOEF * capacity;
                long long *tmplist = realloc(list, newcapacity*sizeof(long long));
                if (tmplist == NULL) {free(list); *err = 1; return vec;}
                list = tmplist;
                capacity = newcapacity;
            }
            list[size++] = num;
            *ptr += strspn(*ptr, cset);
            c = getsym(ptr);
        }
        vec.dim = size;
        vec.vec = list;
        if (c != '}') {free(vec.vec); vec.dim = 0; vec.vec = NULL; *err = 1; return vec; }
    } else { *err = 1; return vec; }
    return vec;
}


void execute(char op, struct stack_vec **st_vec, size_t *err) {
    if (!*err) {
        struct vector vec1, vec2;
        vec1 = pop_vec(st_vec);
        vec2 = pop_vec(st_vec);
        if (op == '*') {
            *st_vec = push_vec(*st_vec, mult(vec1, vec2), err);
        }
        else if (op == '#') {
            *st_vec = push_vec(*st_vec, mult(vec1, vec2), err);
        }
        else if (op == '+') {
            *st_vec = push_vec(*st_vec, add(vec1, vec2), err);
        }
        else if (op == '-') {
            *st_vec = push_vec(*st_vec, sub(vec1, vec2), err);
        }
    }
}

void poliz(char *str, struct stack **st_ops, struct stack_vec **st_vec, size_t *err) {
    if (!*err) {
        char **ptr = &str;
        char c = getsym(ptr);
        while(c != '\0' && c != EOF)
        {
            if(c == ')') {
                while(((*st_ops)->c) != '(') {
                    execute(pop(st_ops), st_vec, err);
                    if (*err) return;
                }
                pop(st_ops);
            }
            else if(isdigit(c)) {
                ungetsym(ptr);
                struct vector vec = wrap_number(get_number(ptr), err);
                if (*err) return;
                *st_vec = push_vec(*st_vec, vec, err);
                if (*err) return;
            }
            else if(c == '{') {
                ungetsym(ptr);
                struct vector vec = get_vec(ptr, err);
                if (*err) return;
                *st_vec = push_vec(*st_vec, vec, err);
                if (*err) return;
            }
            else if(c == '(') {
                *st_ops = push(*st_ops, c, err);
                if (*err) return;
            }
            else if(c == '+'|| c == '-' || c == '*') {
                if(*st_ops == NULL) {
                    *st_ops = push(*st_ops, c, err);
                    if (*err) return;
                }
                else
                    if(prior((*st_ops)->c) < prior(c)) {
                        *st_ops = push(*st_ops, c, err);
                        if (*err) return;
                    }
                    else {
                        while((*st_ops != NULL) && (prior((*st_ops)->c) >= prior(c))) {
                            execute(pop(st_ops), st_vec, err);
                            if (*err) return;
                        }
                        *st_ops = push(*st_ops, c, err);
                        if (*err) return;
                    }
            }
            else if(c == '#') {
                if(*st_ops == NULL) {
                    *st_ops = push(*st_ops, c, err);
                    if (*err) return;
                }
                else {
                    while((*st_ops != NULL) && (prior((*st_ops)->c) > prior(c))) {
                        execute(pop(st_ops), st_vec, err);
                        if (*err) return;
                    }
                    *st_ops = push(*st_ops, c, err);
                    if (*err) return;
                }
            }
            c = getsym(ptr);
        }
        while(*st_ops != NULL) {
            execute(pop(st_ops), st_vec, err);
            if (*err) return;
        }
    }
}

//===========================================[PRINT FUNCTIONS]===================================================

void print_vec(struct vector res) {
    printf("{%lld", res.vec[0]);
    for (size_t i = 1; i < res.dim; ++i) {
        printf(",%lld", res.vec[i]);
    }
    printf("}\n");
}

void error() {
    printf("%s\n", ERRMSG);
}

void free_st(struct stack *st_ops) {
    while (st_ops != NULL) { pop(&st_ops); }
}

void free_st_vec(struct stack_vec *st_vec) {
    struct vector vec;
    while (st_vec != NULL) {
        vec = pop_vec(&st_vec);
        free(vec.vec);
    }
}

//================================================[MAIN]==========================================================

int main(int argc, const char * argv[]) {
    //GET EXPRESSION
    char *input = read_input();
    if (input == NULL) {error(); return 0;}
    
    //CHANGE INPUT STRING
    char *modified = (char *)malloc(sizeof(char) * (strlen(input)+1));
    if (modified == NULL) { free(input); error(); return 0; }
    size_t err = 0;
    size_t len = 0;
    char *ptr = input;
    scan_expr(&ptr, modified, &len, &err);
    if (err || getsym(&ptr) != (char)0) { free(input); free(modified); error(); return 0; }
    modified[len] = (char)0;
    free(input); input = NULL; ptr = NULL;
    
    //CALCULATE
    struct stack *st_ops = NULL;
    struct stack_vec *st_vec = NULL;
    poliz(modified, &st_ops, &st_vec, &err);
    free(modified); modified = NULL;
    if (err) { free_st(st_ops); free_st_vec(st_vec); error(); return 0; }
    
    //PRINT
    struct vector res = pop_vec(&st_vec);
    print_vec(res);
    free(res.vec);
    
    //TO BE SURE
    free_st(st_ops); free_st_vec(st_vec);
    return 0;
}
