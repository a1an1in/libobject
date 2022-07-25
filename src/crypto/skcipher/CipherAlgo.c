/**
 * @file CipherAlgo.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/CipherAlgo.h>


static int __construct(CipherAlgo *algo, char *init_str)
{
    return 0;
}

static int __deconstruct(CipherAlgo *algo)
{
    return 0;
}

static class_info_entry_t cipher_algo_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, CipherAlgo, construct, __construct),
    Init_Nfunc_Entry(2, CipherAlgo, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, CipherAlgo, set_key, NULL),
    Init_Vfunc_Entry(4, CipherAlgo, encrypt, NULL),
    Init_Vfunc_Entry(5, CipherAlgo, decrypt, NULL),
    Init_End___Entry(6, CipherAlgo),
};
REGISTER_CLASS("CipherAlgo", cipher_algo_class_info);

static int
test_aes(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_alloc();
    CipherAlgo *algo;
	uint8_t key[] = {
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13,
		0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b,
		0x1c, 0x1d, 0x1e, 0x1f};

	uint8_t in[] = {
		0x00, 0x11, 0x22, 0x33,
		0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb,
		0xcc, 0xdd, 0xee, 0xff};
	
	uint8_t out[16] = {0}; // 128

    dbg_str(DBG_SUC, "test_aes in");
    algo = object_new(allocator, "Aes", NULL);
    algo->set_key(algo, key, sizeof(key));

    algo->encrypt(algo, in, out);

    for (int i = 0; i < 16; i++) {
        printf("%x ", out[i]);
    }
    printf("\n");
    algo->decrypt(algo, out, out);
    for (int i = 0; i < 16; i++) {
        printf("%x ", out[i]);
    }
    printf("\n");

    object_destroy(algo);

    return ret;
}
REGISTER_TEST_FUNC(test_aes);