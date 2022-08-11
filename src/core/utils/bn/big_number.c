/**
 * @file big_number.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-08-11
 */

#include <libobject/core/utils/bn/big_number.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

int bn_add(uint8_t *dest, int dest_len, int *dest_size, uint8_t *add, int add_size) 
{
    uint8_t *n1, *n2, t1, t2, diff, l, i, carry = 0;
    int ret;

    TRY {
        THROW_IF(dest_size == NULL || dest == NULL || add == NULL, -1);
        THROW_IF(dest_len < (*dest_size > add_size ? *dest_size : add_size) + 1, -1);

        if (*dest_size >= add_size) {
            n1 = dest;
            n2 = add;
            diff = *dest_size - add_size;
        } else {
            n2 = dest;
            n1 = add;
            diff = add_size - *dest_size;
        }

        /* 1. compute the same len part */
        l = *dest_size > add_size ? add_size : *dest_size;
        for (i = 0; i < l; i++) {
            t1 = n1[0];
            t1 = (t1 + carry) & 0xff;
            carry = (t1 < carry);
            t2 = (t1 + n2[0]) & 0xff;
            carry += (t2 < t1);
            dest[0] = t2;
            dest++;
            n1++;
            n2++;
        }

        /* 2. compute the longer len part */
        while (diff--) {
            t1 = *(n1++);
            t2 = (t1 + carry) & 0xff;
            (*dest++) = t2;
            carry &= (t2 == 0);
        }
        *dest = carry;
        *dest_size += carry;
    } CATCH (ret) {
    }

    return ret;
}


int bn_sub(uint8_t *dest, int dest_len, int *dest_size, int *neg_flag, uint8_t *sub, int sub_size) 
{
    uint8_t *n1, *n2, t1, t2, diff, l, i, carry = 0;
    int max_size, ret;

    TRY {
        THROW_IF(dest == NULL || dest_size == NULL || sub == NULL, -1);
        max_size = *dest_size > sub_size ? *dest_size : sub_size;
        THROW_IF(dest_len < max_size, -1);

        EXEC(bn_cmp(dest, *dest_size, sub, sub_size, &ret));
        if (ret >= 0) {
            n1 = dest;
            n2 = sub;
            diff = *dest_size - sub_size;
        } else {
            n2 = dest;
            n1 = sub;
            diff = sub_size - *dest_size;
        }
        /* 1. compute the same len part */
        l = *dest_size > sub_size ? sub_size : *dest_size;
        for (i = 0; i < l; i++) {
            t1 = n1[0];
            t2 = n2[0];
            dest[0] = (t1 - t2 - carry) & 0xff;
            if (t1 != t2)
                carry = (t1 < t2);
            dest++;
            n1++;
            n2++;
        }

        /* 2. compute the longer len part */
        while (diff--) {
            t1 = *(n1++);
            t2 = (t1 - carry) & 0xff;
            (*dest++) = t2;
            carry &= (t2 == 0);
        }

        /* 3. compute number size */
        dbg_str(DBG_DETAIL, "number size:%d", max_size);
        dbg_str(DBG_DETAIL, "*(--dest):%x", *(dest -1));
        while (max_size && *(--dest) == 0) {
            dbg_str(DBG_DETAIL, "run at here");
            max_size--;
        }
        if (*dest_size < sub_size) {
            *neg_flag = 1;
        }
        
        *dest_size = max_size;
        dbg_str(DBG_DETAIL, "number size:%d", max_size);
    } CATCH (ret) {
    }

    return ret;
}

int bn_cmp(uint8_t *n1, int n1_len, uint8_t *n2, int n2_len, int *out) 
{
    int ret, i;

    TRY {
        THROW_IF(n1 == NULL || n2 == NULL, -1);

        if (n1_len > n2_len) {
            *out = 1;
            return 1;
        } else if (n1_len < n2_len) {
            *out = -1;
            return 1;
        }

        for (i = n1_len; i >= 0; i--) {
            if (n1[i - 1] > n2[i - 1]) {
                *out = 1;
                return 1;
            }
            if (n1[i - 1] < n2[i - 1]) {
                *out = -1;
                return 1;
            }
        }
        *out = 0;
        THROW(1);
    } CATCH (ret) {
    }

    return ret;
}

int bn_mul(uint8_t *dest, int dest_len, uint8_t *a, int a_len, int *b, int b_len)
{
    ;
}

static int
test_bn_cmp(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    uint8_t num1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint8_t num2[8] = {0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x78};

    TRY {
        EXEC(bn_cmp(num1, sizeof(num1), num2, sizeof(num2), &ret));
        THROW_IF(ret >= 0, 0);
        EXEC(bn_cmp(num2, sizeof(num2), num1, sizeof(num1), &ret));
        THROW_IF(ret < 0, 0);
        EXEC(bn_cmp(num1, sizeof(num1), num1, sizeof(num1), &ret));
        THROW_IF(ret != 0, 0);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_bn_cmp, ret=%d", ret);
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_bn_cmp);