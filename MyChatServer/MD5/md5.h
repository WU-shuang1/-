#include <windows.h>
#include <tchar.h>
typedef struct {
    /* number of _bits_ handled mod 2^64 */
    ULONG i[2];
    /* scratch buffer */
    ULONG buf[4];
    /* input buffer */
    unsigned char in[64];
    /* actual digest after MD5Final call */
    unsigned char digest[16];
} MD5_CTX;

#define MD5DIGESTLEN 16
#define PROTO_LIST(list)    list
/*
* MTS: Each of these assumes MD5_CTX
* is locked against simultaneous use.
*/
typedef void(WINAPI* PMD5Init) PROTO_LIST((MD5_CTX*));
typedef void(WINAPI* PMD5Update) PROTO_LIST((MD5_CTX*
    , const unsigned char*
    , unsigned int));
typedef void(WINAPI* PMD5Final)PROTO_LIST((MD5_CTX*));
class CMD5Encrypt
{
public:
    CMD5Encrypt();
    virtual ~CMD5Encrypt();
    const char* md5(const char* str);
    const char* Hex2ASC(const BYTE* Hex, int Len);
    PMD5Init MD5Init;
    PMD5Update MD5Update;
    PMD5Final MD5Final;
};
