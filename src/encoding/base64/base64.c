#include <libobject/core/String.h>
#include <libobject/encoding/base64.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

int base64_encode(uint8_t *src, int src_len, uint8_t *dst, int dst_len)  
{  
    int i, j, len;  
    uint8_t *base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 
    char t;  

    if(src_len % 3 == 0)  
        len = src_len / 3 * 4;  
    else  
        len = (src_len / 3 + 1) * 4;  

    if (len > dst_len) {
        return -1;
    }

    //以3个8位字符为一组进行编码  
    for(i = 0, j = 0; i < len - 2; j += 3, i += 4) {
        t = j + 2 > src_len ? 0 : (src[j + 1] >> 4);
        dst[i]     = base64_table[src[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符 
        dst[i + 1] = base64_table[(src[j] & 0x3) << 4 | t];
        dst[i + 2] = base64_table[(src[j + 1] & 0xf) << 2 | (src[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
        dst[i + 3] = base64_table[src[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符 
    }  

    switch(src_len % 3)  {  
        case 1:  
            dst[i - 2] = '=';  
            dst[i - 1] = '=';  
            break;  
        case 2:  
            dst[i - 1] = '=';  
            break;  
    }

    return 0;  
}  

int base64_decode(uint8_t *src, int src_len, uint8_t *dst, int dst_len)  
{    
    int i, j, len; 
    int table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 
        63, 52, 53, 54, 55, 56, 57, 58, 
        59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
        13, 14, 15, 16, 17, 18, 19, 20, 21, 
        22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 
        27, 28, 29, 30, 31, 32, 33, 34, 35, 
        36, 37, 38, 39, 40, 41, 42, 43, 44, 
        45, 46, 47, 48, 49, 50, 51
    };  

    //判断编码后的字符串后是否有 =   
    if(strstr(src, "=="))  
        len = src_len / 4 * 3 - 2;  
    else if(strstr(src, "="))  
        len = src_len / 4 * 3 - 1;  
    else  
        len = src_len / 4 * 3;  

    if (len > dst_len) {
        return -1;
    }

    //以4个字符为一位进行解码  
    for(i = 0, j = 0; i < src_len - 2; j += 3, i += 4) {  
        dst[j] = ((unsigned char)table[src[i]]) << 2 | (((unsigned char)table[src[i + 1]]) >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        dst[j + 1] = (((unsigned char)table[src[i + 1]]) << 4) | (((unsigned char)table[src[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        dst[j + 2] = (((unsigned char)table[src[i + 2]]) << 6) | ((unsigned char)table[src[i + 3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }

    return 0;  
}

static int
test_base64_encode(TEST_ENTRY *entry, void *argc, void *argv)
{
    // char *expect = "aGVsbG8gd29ybGQ=";
    // char *test = "hello world";
    uint8_t test[16] = {0xCB, 0x66, 0x73, 0xBD, 0x52, 0x55, 0x57, 0x6D, 0x6C, 0x63, 0x2E, 0x68, 0x0D, 0x29, 0x7E, 0xF8};
    char *expect = "y2ZzvVJVV21sYy5oDSl++A==";
    unsigned short ret;
    uint8_t dst[1024] = {0};

    TRY {
        EXEC(base64_encode(test, 16, dst, 1024));
        ret = assert_equal(expect, dst, strlen(expect));
        dbg_str(DBG_DETAIL, "dst:%s, expect:%s", dst, expect);

        SET_CATCH_STR_PARS(dst, expect);
        THROW_IF(ret != 1, -1);
    } CATCH (ret){
        dbg_str(DBG_ERROR, "test_base64_decode error, encode result=%s, expect=%s",
                ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
        dbg_buf(DBG_ERROR, "result:", dst, strlen(dst));
        dbg_buf(DBG_ERROR, "expect:", expect, strlen(expect));
    }

    return ret;
}
REGISTER_TEST_FUNC(test_base64_encode);

static int
test_base64_decode(TEST_ENTRY *entry, void *argc, void *argv)
{
    char *test = "aGVsbG8gd29ybGQ=";
    char *expect = "hello world";
    unsigned short ret;
    char dst[1024] = {0};

    TRY {
        EXEC(base64_decode(test, strlen(test), dst, 1024));
        ret = assert_equal(expect, dst, strlen(dst));
        dbg_str(DBG_DETAIL, "dst:%s, expect:%s", dst, expect);
        SET_CATCH_STR_PARS(dst, expect);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_base64_decode error, encode result=%s, expect=%s",
                ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}
REGISTER_TEST_FUNC(test_base64_decode);