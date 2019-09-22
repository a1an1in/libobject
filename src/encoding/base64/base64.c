#include <libobject/core/String.h>
#include <libobject/encoding/base64.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

int base64_encode(String *src, String *dst)  
{  
    unsigned char *s;
    unsigned char *d;  
    long s_len, len;  
    int i, j;  
    unsigned char *base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  

    s = src->get_cstr(src);
    s_len = strlen(s);  
    if(s_len % 3 == 0)  
        len = s_len / 3 * 4;  
    else  
        len = (s_len / 3 + 1) * 4;  

    d = malloc(sizeof(unsigned char) * len + 1);  
    d[len] = '\0';  

    //以3个8位字符为一组进行编码  
    for(i = 0, j = 0 ; i < len - 2; j += 3, i += 4) {  
        d[i] = base64_table[s[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符  
        d[i + 1] = base64_table[(s[j] & 0x3) << 4 | (s[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
        d[i + 2] = base64_table[(s[j + 1] & 0xf) << 2 | (s[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
        d[i + 3] = base64_table[s[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符  
    }  

    switch(s_len % 3)  {  
        case 1:  
            d[i - 2] = '=';  
            d[i - 1] = '=';  
            break;  
        case 2:  
            d[i - 1] = '=';  
            break;  
    }  

    dst->assign(dst, d);
    free(d);

    return 0;  
}  

int base64_decode(String *src, String *dst)  
{  
    long len;  
    long str_len;  
    unsigned char *d;  
    int i, j;  
    char *s = src->get_cstr(src);
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

    //计算解码后的字符串长度  
    len = strlen(s);  
    //判断编码后的字符串后是否有 =   
    if(strstr(s, "=="))  
        str_len = len / 4 * 3 - 2;  
    else if(strstr(s, "="))  
        str_len = len / 4 * 3 - 1;  
    else  
        str_len = len / 4 * 3;  

    d = malloc(sizeof(unsigned char) * str_len + 1);  
    d[str_len] = '\0';  

    //以4个字符为一位进行解码  
    for(i = 0, j = 0; i < len - 2; j += 3, i += 4) {  
        d[j] = ((unsigned char)table[s[i]]) << 2 | (((unsigned char)table[s[i + 1]]) >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        d[j + 1] = (((unsigned char)table[s[i + 1]]) << 4) | (((unsigned char)table[s[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        d[j + 2] = (((unsigned char)table[s[i + 2]]) << 6) | ((unsigned char)table[s[i + 3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }  

    dst->assign(dst, d);
    free(d);

    return 0;  
}

static int
test_base64_encode(TEST_ENTRY *entry, void *argc, void *argv)
{
    char *expect = "aGVsbG8gd29ybGQ=";
    char *test = "hello world";
    char *result;
    unsigned short ret;
    allocator_t *allocator = allocator_get_default_alloc();
    String *src, *dst;

    src = object_new(allocator, "String", NULL);
    src->assign(src, test);
    dst = object_new(allocator, "String", NULL);

    base64_encode(src, dst);
    ret = assert_equal(expect, dst->get_cstr(dst), dst->get_len(dst));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    } else {
        dbg_str(DBG_ERROR, "expect:%s result:%s", expect,
                dst->get_cstr(dst));
    }

    object_destroy(src);
    object_destroy(dst);

    return ret;
}
REGISTER_TEST_FUNC(test_base64_encode);

static int
test_base64_decode(TEST_ENTRY *entry, void *argc, void *argv)
{
    char *test = "aGVsbG8gd29ybGQ=";
    char *expect = "hello world";
    char *result;
    unsigned short ret;
    allocator_t *allocator = allocator_get_default_alloc();
    String *src, *dst;

    src = object_new(allocator, "String", NULL);
    src->assign(src, test);
    dst = object_new(allocator, "String", NULL);

    base64_decode(src, dst);
    ret = assert_equal(expect, dst->get_cstr(dst), dst->get_len(dst));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    } else {
        dbg_str(DBG_ERROR, "expect:%s result:%s", expect,
                dst->get_cstr(dst));
    }

    object_destroy(src);
    object_destroy(dst);

    return ret;
}
REGISTER_TEST_FUNC(test_base64_decode);

