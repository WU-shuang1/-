// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"
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

//第二步：定义MD5加密的类
//1.h文件： 

class CMD5Encrypt
{
public:
    CMD5Encrypt();
    virtual ~CMD5Encrypt();
    __declspec(dllexport) const char* md5(const char* str);
    __declspec(dllexport) const char* Hex2ASC(const BYTE* Hex, int Len);
    PMD5Init MD5Init;
    PMD5Update MD5Update;
    PMD5Final MD5Final;
};

#endif //PCH_H
