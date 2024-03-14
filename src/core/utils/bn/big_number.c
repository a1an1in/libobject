/**
 * @file big_number.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-08-11
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libobject/core/utils/bn/big_number.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

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

/* 计算big number 有多少位， 因为高位有可能是0 */
int bn_size(uint8_t *dest, int dest_len, int *len)
{
    int i;

    for (i = dest_len - 1; i >= 0; i--) {
        if (dest[i] != 0) {
            break;
        }
    }

    if (i >= 0) {
        *len = i + 1;
    } else {
        *len = 0;
    }
    
    return 1;
}

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
            *neg_flag = 1;
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
        while (max_size && *(--dest) == 0) {
            max_size--;
        }
        
        *dest_size = max_size;
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "max_size:%d, dest_len=%d", max_size, dest_len);
    }

    return ret;
}

int bn_mul_u32(uint32_t *dest, int dest_len, uint32_t *a, int a_size, uint32_t word)
{
    int count, ret;
    uint32_t result, t1, t2, carry = 0;
    uint8_t *p1, *p2;

    TRY {
        count = (a_size + sizeof(int) - 1) / sizeof(int);
        
        while (count) {
            p1 = dest;
            p2 = a;
            t1 = p1[3] << 24 | p1[2] << 16 | p1[1] << 8 | p1[0];

            if (a_size >= 4) {
                t2 = p2[3] << 24 | p2[2] << 16 | p2[1] << 8 | p2[0];
            } else if (a_size == 3) {
                t2 = p2[2] << 16 | p2[1] << 8 | p2[0];
            } else if (a_size == 2) {
                t2 = p2[1] << 8 | p2[0];
            } else if (a_size == 1) {
                t2 = p2[0];
            } else {
                THROW(-1);
            }
            
            // dbg_str(DBG_DETAIL, "bn_mul_u32, t1:%x t2:%x, word:%x, carry:%x", t1, t2, word, carry);
            BN_MUL_U32(t1, t2, word, carry);
            dest[0] = t1;
            dest++;
            a++;
            count--;
            a_size -= sizeof(int);
        }

        if (carry > 0) {
            dest[0] = carry;
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "a_size:%x, word:%x, carry:%x, count:%x", 
                a_size, word, carry, count);
        dbg_buf(DBG_ERROR, "bn_mul_u32 multiplier:", a, a_size);
    }

    return ret;
}

int bn_mul(uint8_t *dest, int dest_len, int *dest_size, uint8_t *a, int a_size, uint8_t *b, int b_size)
{
    int ret, len, i, len_bak;
    uint8_t *tmp;
    uint32_t value;

    TRY {
        THROW_IF(dest == NULL || a == NULL || b == NULL, -1);

        EXEC(bn_size(a, a_size, &a_size));
        EXEC(bn_size(b, b_size, &b_size));
        THROW_IF(dest_len < a_size + b_size, -1);
        len_bak = dest_len;

        if (a_size < b_size) {
            tmp = a;
            len = a_size;
            a = b;
            a_size = b_size;
            b = tmp;
            b_size = len;
        }

        THROW_IF(b_size <= 0, 0);

        for (i = 0; b_size > 0; b_size -= 4) {
            if (b_size >= 4) {
                value = b[i + 3] << 24 | b[i + 2] << 16 | b[i + 1] << 8 | b[i + 0];
            } else if (b_size == 3) {
                value = b[i + 2] << 16 | b[i + 1] << 8 | b[i + 0];
            } else if (b_size == 2) {
                value = b[i + 1] << 8 | b[i + 0];
            } else if (b_size == 1) {
                value = b[i + 0];
            } else {
                THROW(-1);
            }
            
            // dbg_str(DBG_DETAIL, "bn_mul_u32 value:%x", value);
            // dbg_buf(DBG_DETAIL, "bn_mul_u32 a:", a, a_size);
            EXEC(bn_mul_u32(dest + i, dest_len - i, a, a_size, value));
            // dbg_buf(DBG_DETAIL, "result:", dest + i, dest_len - i);
            i += 4;
        }

        EXEC(bn_size(dest, len_bak, dest_size));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "dest_len:%d, a_size:%d, b_size:%d, i:%d", dest_len, a_size, b_size, i);
    }

    return ret;
}

int bn_div(uint8_t *quotient, int quotient_len, int *quotient_size, 
           uint8_t *remainder, int remainder_len, int *remainder_size,
           uint8_t *dividend, int dividend_size, uint8_t *divisor, int divisor_size)
{
    int ret = 0, len, i, len_bak, multiple, tmp_size, dividend_len;
    int *neg_flag, compare_result;
    uint8_t *tmp, tmp_len;
    uint32_t value;

    TRY {
        THROW_IF(quotient == NULL || remainder == NULL, -1);
        THROW_IF(remainder_len < dividend_size || quotient_len < dividend_size, -1);

        dividend_len = dividend_size;
        tmp = remainder;
        tmp_len = remainder_len;
        EXEC(bn_size(dividend, dividend_size, &dividend_size));
        EXEC(bn_size(divisor, divisor_size, &divisor_size));
        EXEC(bn_cmp(dividend, dividend_size, divisor, divisor_size, &ret));
        if (ret < 0) {
            quotient[0] = 0;
            THROW(1);
        } else if (ret == 0) {
            quotient[0] = 1;
            dividend_size = 0;  // inorder to copy remainder
            THROW(1);
        }

        for (i = dividend_size - divisor_size; i >= 0; i = dividend_size - divisor_size) {
            /* 1. 猜测商的最高位 */
            dbg_buf(DBG_DETAIL, "bn_div dividend:", dividend, dividend_len);
            dbg_str(DBG_DETAIL, "bn_div dividend size:%d", dividend_size);
            EXEC(bn_cmp(dividend + dividend_size - divisor_size, divisor_size, divisor, divisor_size, &compare_result));
            if (compare_result >= 0) {
                multiple = dividend[dividend_size - 1] / divisor[divisor_size - 1];
            } else if (dividend_size >= 2) {
                multiple = (dividend[dividend_size - 1] << 8 |  dividend[dividend_size - 2]) / divisor[divisor_size - 1];
                i--;
            } else {THROW(1);}
            
            /* 2. 计算商的最高位倍数 */
            do {
                memset(tmp, 0, tmp_len);
                EXEC(bn_mul_u32(tmp, tmp_len, divisor, divisor_size, multiple));
                EXEC(bn_size(tmp, tmp_len, &tmp_size));
                THROW_IF(dividend_size < tmp_size, 1);
                EXEC(bn_cmp(dividend + dividend_size - tmp_size, tmp_size, tmp, tmp_size, &compare_result));
                if (compare_result >= 0) { break; }
                multiple--;
            } while (multiple > 0);

            /* 3. 减去倍数被除数，然后产生新的被除数，进入下一次循环, 直到被除数小于除数 */
            THROW_IF(multiple == 0, -1);
            quotient[i] = multiple;
            memset(tmp, 0, remainder_len);
            EXEC(bn_mul(tmp, tmp_len, &tmp_size, divisor, divisor_size, quotient, i + 1));
            EXEC(bn_size(tmp, tmp_len, &tmp_size));
            dbg_buf(DBG_DETAIL, "bn_div tmp:", tmp, tmp_size);
            EXEC(bn_sub(dividend, dividend_size, &dividend_size, &neg_flag, tmp, tmp_size));
            dbg_str(DBG_INFO, "bn_div multiple:%x", multiple);
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "remainder_len:%d", remainder_len);
        dbg_str(DBG_ERROR, "tmp_len:%d", tmp_len);
        dbg_str(DBG_ERROR, "dividend_len:%d", dividend_len);
        dbg_str(DBG_ERROR, "dividend_size:%d", dividend_size);
        dbg_str(DBG_ERROR, "tmp_size:%d", tmp_size);
        dbg_buf(DBG_ERROR, "dividend:", dividend, dividend_size);
        dbg_buf(DBG_ERROR, "divisor:", divisor, divisor_size);
    } FINALLY {
        if (quotient_size) {bn_size(quotient, quotient_len, quotient_size);}
        dbg_str(DBG_DETAIL, "bn_div dividend size:%d", dividend_size);
        
        if (remainder_size) {
            memset(remainder, 0, remainder_len); //需要清空，前面计算tmp有使用过。
            memcpy(remainder, dividend, dividend_size);
            dbg_buf(DBG_DETAIL, "bn_div remainder:", remainder, dividend_len);
            *remainder_size = dividend_size;
        }
    }

    return ret;
}

int bn_rand(uint8_t *dest, int dest_len, int *dest_size, int bits, int top, int bottom)
{
    int bytes;
    int i, bit, mask, ret;
    time_t t;

    TRY {
        bytes = (bits + 7) / 8;
        bit = (bits - 1) % 8;
        mask = 0xff << (bit + 1);

        srand((unsigned) time(&t));
        for (i = 0; i < bytes; i++) {
            dest[i] = rand() % 0xff;
        }

        if (top == 0) {
            dest[bytes - 1] |= (1 << bit);
        } else if (top == 1 && bit == 0) {
            dest[bytes - 1] = 1;
            dest[bytes - 2] |= 0x80;
        } else if (top == 1 && bit != 0) {
            dest[bytes - 1] |= (3 << (bit -1));
        } else {
            THROW(-1);
        }
        dest[bytes - 1] &= ~mask;

        if (bottom) {
            dest[0] |= 1;
        }

        *dest_size = bytes;
        dbg_buf(DBG_DETAIL, "rand:", dest, bytes);
    } CATCH (ret) { }
}

int bn_prime(uint8_t *dest, int dest_len, int *dest_size, int bits)
{

}
