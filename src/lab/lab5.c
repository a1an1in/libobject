#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

#if (defined(WINDOWS_USER_MODE))
static char *
strtok_r (char *s, const char *delim, char **save_ptr)
{
    char *token;

    if (s == NULL)
        s = *save_ptr;

    /* Scan leading delimiters. */
    s += strspn (s, delim);
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }

    /* Find the end of the token. */
    token = s;
    s = strpbrk (token, delim);
    if (s == NULL)
        /* This token finishes the string. */
        *save_ptr = strchr (token, '\0');
    else {
        /* Terminate the token and make *SAVE_PTR point past it. */
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}
#endif

int test_str_cmp(TEST_ENTRY *entry)
{
    char *dest = "--file-path";
    int ret;

    ret = strcmp(dest, "--file");
    dbg_str(DBG_DETAIL, "cmp --file, ret=%d", ret);

    ret = strcmp(dest, "--file-path");
    dbg_str(DBG_DETAIL, "cmp --file-path, ret=%d", ret);

    ret = strcmp(dest, "--file-path=~/x.xml");
    dbg_str(DBG_DETAIL, "cmp --file-path=~/x.xml, ret=%d", ret);
}
REGISTER_TEST_FUNC(test_str_cmp);

static int bubble_sort(int *array, int len)
{
    int i, j, t;

    for (i = 0; i < len - 1; i++) {
        for (j = 0; j < len - 1 - i; j++) {
            if (array[j] > array[j + 1]) {
                t = array[j];
                array[j] = array[j + 1];
                array[j + 1] = t;
            }
        }
    }

    return 1;
}

int test_bubble_sort(TEST_ENTRY *entry)
{
    int ret;
    int array[]  = {900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500};
    int expect[] = {-234, -70, -58, 2, 3, 32, 34, 35, 43, 56, 76, 532, 543, 900, 2500};
    int i, j, t, len;

    len = sizeof(array) / sizeof(int);

    bubble_sort(array, len);

    for (i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    ret = assert_equal(array, expect, sizeof(array));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    }

    return ret;
}
REGISTER_TEST_CMD(test_bubble_sort);

int test_selection_sort(TEST_ENTRY *entry)
{
    int ret;
    int array[]  = {900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500};
    int expect[] = {-234, -70, -58, 2, 3, 32, 34, 35, 43, 56, 76, 532, 543, 900, 2500};
    int i, j, t, min, len;

    len = sizeof(array) / sizeof(int);

    for (i = 0; i < len; i++) {
        
        for (j = i, min = i; j < len; j++) {
            if (array[j] < array[min]) {
                min = j;
            }
        }

        if (i != min) {
            t = array[i];
            array[i] = array[min];
            array[min] = t;
        }
    }

    for (i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    ret = assert_equal(array, expect, sizeof(array));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    }

    return ret;
}
REGISTER_TEST_CMD(test_selection_sort);

int test_insertion_sort(TEST_ENTRY *entry)
{
    int ret;
    int array[]  = {900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500};
    int expect[] = {-234, -70, -58, 2, 3, 32, 34, 35, 43, 56, 76, 532, 543, 900, 2500};
    int i, j, t, cur, len;

    len = sizeof(array) / sizeof(int);

    for (i = 0; i < len; i++) {
        cur = array[i];
        for (j = i - 1; j >= 0 && array[j] > cur; j--) {
            array[j + 1] = array[j];
        }
        array[j + 1] = cur;
    }

    for (i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    ret = assert_equal(array, expect, sizeof(array));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    }

    return ret;
}
REGISTER_TEST_CMD(test_insertion_sort);

int __merge(int *array, int left, int mid, int right, int *tmp)
{
    int index = left;
    int i = left, j = mid + 1;

    while (i <= mid && j <= right ) {
        if (array[i] < array[j]) {
            tmp[index++] = array[i++];
        } else {
            tmp[index++] = array[j++];
        }
    }

    while (i <= mid) {
        tmp[index++] = array[i++];
    }

    while (j <= right) {
        tmp[index++] = array[j++];
    }

    memcpy(array + left, tmp + left, (right - left + 1) * sizeof(int));
}

int __merge_sort(int *array, int left, int right, int *tmp)
{
    int mid;

    if (left >= right) return 0;

    mid = left + (right - left) / 2;
    __merge_sort(array, left, mid, tmp);
    __merge_sort(array, mid + 1, right, tmp);
    __merge(array, left, mid, right, tmp);

}

int merge_sort(int *array, int len)
{
    int *tmp;

    tmp = malloc(len * sizeof(int));
    __merge_sort(array, 0, len - 1, tmp);
    free(tmp);

}

int test_merge_sort(TEST_ENTRY *entry)
{
    int ret;
    int array[]  = {900, 2, 3, -58, 34, 76, 32, 43, 56, -70, 35, -234, 532, 543, 2500};
    int expect[] = {-234, -70, -58, 2, 3, 32, 34, 35, 43, 56, 76, 532, 543, 900, 2500};
    int i, j, t, len;

    len = sizeof(array) / sizeof(int);

    merge_sort(array, len);

    for (i = 0; i < len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    ret = assert_equal(array, expect, sizeof(array));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    }

    return ret;
}
REGISTER_TEST_CMD(test_merge_sort);

typedef struct slist_node_s {
    struct slist_node_s *next;
    int d;
} slist_node_t;

static slist_node_t *slist_alloc() 
{
   slist_node_t *node;

   node = (slist_node_t *)malloc(sizeof(slist_node_t));

   node->next = NULL;

   return node;
}

static slist_node_t *new_slist_node(int d)
{
   slist_node_t *node;

   node = (slist_node_t *)malloc(sizeof(slist_node_t));

   node->next = NULL;
   node->d = d;

   return node;
}

static void slist_add(slist_node_t *new, slist_node_t *head)
{
    slist_node_t *cur, *tail;

    cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    
    tail = cur;
    tail->next = new;
}

int test_slist_add(TEST_ENTRY *entry)
{
    slist_node_t *head, *new, *pos;
    int i;

    head = slist_alloc();

    new = new_slist_node(1);
    slist_add(new, head);
    new = new_slist_node(2);
    slist_add(new, head);
    new = new_slist_node(3);
    slist_add(new, head);
    new = new_slist_node(4);
    slist_add(new, head);
    new = new_slist_node(5);
    slist_add(new, head);

    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_slist_add);

slist_node_t * slist_rotate(slist_node_t *head)
{
    slist_node_t *new_head, *tail, *n;

    new_head = head;
    tail = head;

    while (head != NULL) {
        n = head;
        head = head->next;
        n->next = new_head;
        new_head = n;
    }

    tail->next = NULL;

    return new_head;
}

int test_slist_rotate(TEST_ENTRY *entry)
{
    slist_node_t *head, *new, *pos;
    int i;

    head = slist_alloc();

    new = new_slist_node(1);
    slist_add(new, head);
    new = new_slist_node(2);
    slist_add(new, head);
    new = new_slist_node(3);
    slist_add(new, head);
    new = new_slist_node(4);
    slist_add(new, head);
    new = new_slist_node(5);
    slist_add(new, head);

    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }
    printf("\n");

    head = slist_rotate(head);
    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_slist_rotate);

slist_node_t * slist_rotate_n(slist_node_t *head, int c)
{
    slist_node_t *new_head, *tail, *n;
    int count = 0;

    new_head = head;
    tail = head;

    while (head != NULL && count != c) {
        count++;
        n = head;
        head = head->next;
        n->next = new_head;
        new_head = n;
    }

    if (count == c) {
        dbg_str(DBG_DETAIL, "run at here");
        tail->next = head;
    } else {
        dbg_str(DBG_DETAIL, "run at here");
        tail->next = NULL;
    }

    return new_head;
}

int test_slist_rotate_n(TEST_ENTRY *entry)
{
    slist_node_t *head, *new, *pos;
    int i;

    head = slist_alloc();

    new = new_slist_node(1);
    slist_add(new, head);
    new = new_slist_node(2);
    slist_add(new, head);
    new = new_slist_node(3);
    slist_add(new, head);
    new = new_slist_node(4);
    slist_add(new, head);
    new = new_slist_node(5);
    slist_add(new, head);

    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }
    printf("\n");

    head = slist_rotate_n(head, 2);
    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_slist_rotate_n);

static int mystr_split(char *str, char *delims, char ***out) 
{
    int index = 0;
    char *ptr = NULL, **addr, *p;
    int i, j, count = 0, delim_len = strlen(delims);
    int str_len;

    str_len = strlen(str);

    for (i = 0; i < delim_len; i++) {
        for (j = 0; j < str_len; j++) {
            if (str[j] == delims[i]) {
                count++;
            }
        }
    }

    if (count == 0) return 0;

    addr = malloc(sizeof(void *) * count);
    p = malloc(str_len);
    strcpy(p, str);

    for (   p = strtok_r(p, delims, &ptr);
            p;
            p = strtok_r(NULL, delims, &ptr)) 
    {
        *(addr + index) = p;
        index++;
    }

    if (index > 0) {
        *out = addr;
    }

    free(p);

    return index;
}

int test_mystr_split(TEST_ENTRY *entry)
{
    char *str = "ASCE 789 123 456";
    char **out;
    int count, i;
    int *array;

    dbg_str(DBG_SUC, "test str split");
    count = mystr_split(str, " ", &out);

    array = malloc(sizeof(int) * (count - 1));

    for (i = 1; i < count; i++) {
        printf("%d %s\n", i, out[i]);
        array[i - 1] = atoi(out[i]);
        printf("array %d %d\n", i, array[i - 1]);
    }

    for (i = 0; i < count - 1; i++) {
        printf("%d %d\n", i, array[i]);
    }

    bubble_sort(array, count - 1);

    for (i = 0; i < count - 1; i++) {
        printf("%d %d\n", i, array[i]);
    }

    free(array);

    return 1;
}
REGISTER_TEST_CMD(test_mystr_split);

char* strchr_n(char *s, char c, int n)
{
    int count = 0;

    while(*s != '\0') {
        if (*s == c) {
            count++;
        }

        if (count == n) break;

        ++s;
    }

    if (count == n) {
        return s;
    } else return NULL;
}

int test_strchr(TEST_ENTRY *entry)
{
    char *str = "lbrnsepcfjzcpfgzqdiujo";
    char *p;
    int len, i;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        p = strchr_n(str, str[i], 2);
        if (p == NULL) {
            printf("found %c", str[i]);
            break;
        }
    }

}
REGISTER_TEST_CMD(test_strchr);


typedef struct dlist_node_s {
    struct dlist_node_s *next;
    struct dlist_node_s *prev;
    int d;
} dlist_node_t;

static dlist_node_t *dlist_alloc() 
{
   dlist_node_t *node;

   node = (dlist_node_t *)malloc(sizeof(dlist_node_t));

   node->next = node->prev = node;

   return node;
}

static dlist_node_t *new_dlist_node(int d)
{
   dlist_node_t *node;

   node = (dlist_node_t *)malloc(sizeof(dlist_node_t));

   node->next = node->prev = node;
   node->d = d;

   return node;
}

static void dlist_add(dlist_node_t *new, dlist_node_t *head)
{
    new->next = head->next;
    new->next->prev = new;
    head->next = new;
    new->prev = head;
}

int test_mydlist(TEST_ENTRY *entry)
{
    dlist_node_t *head, *new, *pos;
    int i;

    head = dlist_alloc();

    new = new_dlist_node(1);
    dlist_add(new, head);
    new = new_dlist_node(2);
    dlist_add(new, head);
    new = new_dlist_node(3);
    dlist_add(new, head);
    new = new_dlist_node(4);
    dlist_add(new, head);
    new = new_dlist_node(5);
    dlist_add(new, head);

    pos = head;
    for (i = 0; i < 5; i++) {
        pos = pos->next;
        printf("%d: %d ", i, pos->d);
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_mydlist);

int test_strtok_r(TEST_ENTRY *entry)
{
    char str[] = "alan is very best";
    char *temp;
    char *p;

    p = str;
    while( (p = strtok_r(p, " ", &temp)) != NULL) {
        printf("%s\n", p);
        p = NULL;
    }
}
REGISTER_TEST_CMD(test_strtok_r);


int test_print_cn(TEST_ENTRY *entry)
{
    char *str1 = "测试";

    printf("ret =%s\n", str1);
}
REGISTER_TEST_CMD(test_print_cn);

