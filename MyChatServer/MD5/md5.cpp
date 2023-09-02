//2.cpp文件: 
CMD5Encrypt::CMD5Encrypt()
{

}

CMD5Encrypt::~CMD5Encrypt()
{

}
//将BYTE数组转换成字符串
const char* CMD5Encrypt::Hex2ASC(const BYTE* Hex, int Len)
{
    static char ASC[4096 * 2];
    int i;

    for (i = 0; i < Len; i++)
    {
        ASC[i * 2] = "0123456789ABCDEF"[Hex[i] >> 4];
        ASC[i * 2 + 1] = "0123456789ABCDEF"[Hex[i] & 0x0F];
    }
    ASC[i * 2] = '\0';
    return ASC;
}
//32位进行MD5加密
const char* CMD5Encrypt::md5(const char* str)
{
    MD5_CTX ctx;//MD5运算使用的数据结构
    const unsigned char* buf
        = reinterpret_cast<const unsigned char*>(str);
    //判断加密字符串的长度
    int len = strlen(str);
    HINSTANCE hDLL;
    if ((hDLL = LoadLibrary(_T("advapi32.dll"))) > 0)
    {
        MD5Init = (PMD5Init)GetProcAddress(hDLL, "MD5Init");
        MD5Update = (PMD5Update)GetProcAddress(hDLL, "MD5Update");
        MD5Final = (PMD5Final)GetProcAddress(hDLL, "MD5Final");

        MD5Init(&ctx);
        MD5Update(&ctx, buf, len);
        MD5Final(&ctx);
    }
    return Hex2ASC(ctx.digest, 16);
}
