#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

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

int bubble_sort(int *array, int len)
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

