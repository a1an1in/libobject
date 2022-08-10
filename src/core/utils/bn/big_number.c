#include <libobject/core/utils/bn/big_number.h>
#include <libobject/core/try.h>

int bn_add(uint8_t *dest, int dest_len, int *dest_size, uint8_t *add, int add_size) 
{
    uint8_t *n1, *n2, tmp1, tmp2, diff, l, i, carry = 0;
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
            tmp1 = n1[0];
            tmp1 = (tmp1 + carry) & 0xff;
            carry = (tmp1 < carry);
            tmp2 = (tmp1 + n2[0]) & 0xff;
            carry += (tmp2 < tmp1);
            dest[0] = tmp2;
            dest++;
            n1++;
            n2++;
        }

        /* 2. compute the longer len part */
        n1 += l;
        dest += l;
        while (diff--) {
            tmp1 = *(n1++);
            tmp2 = (tmp1 + carry) & 0xff;
            (*dest++) = tmp2;
            carry &= (tmp2 == 0);
        }
        *dest = carry;
        *dest_size += carry;
    } CATCH (ret) {
    }

    return ret;
}
