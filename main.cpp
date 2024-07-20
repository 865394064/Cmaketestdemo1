// CMakeProject2.cpp: 定义应用程序的入口点。
//



#include <string.h>
#include "main.h"
#include "ASN1Util.h"
#include <time.h>
#include <sys/time.h>
using namespace std;


#define FAM_OWNER_CRT_OTHER_TYPE        (0x2)
#define FAM_CRT_HEADER_MAGIC_NUM   (0x1a2b3c4d)


static void *(*cJSON_malloc)(size_t sz) = malloc;
static int pow2gt (int x)	{	--x;	x|=x>>1;	x|=x>>2;	x|=x>>4;	x|=x>>8;	x|=x>>16;	return x+1;	}

typedef struct {char *buffer; int length; int offset; } printbuffer;
static void (*cJSON_free)(void *ptr) = free;

static void dumpMemory(const void* pStart, uint32_t len)
{
    char* pBuf = NULL;          /* dump buffer. 用来以文本的形式("0xaa, 0xbb, ...") 表示 一行, 最多 16 字节的 十六进制数. */
    unsigned int bufLen = 0;    /* *pBuf 的字节长度, 包括末尾的 '\0'. */

    unsigned char* pSource = (unsigned char*)pStart;          /* 单次 sprintf 操作的源地址. */
    char* pTarget = NULL;                                     /* 目标地址. */

    unsigned int i, j;

#define BYTES_PER_LINE (16)                         /* dump 输出每行打印的 source bytes 的数目. */
    int BUF_SIZE_PER_SRC_BYTE  = strlen("0xXX, ");  // const. 每个待 dump 的 字节在 *pBuf 中需要的空间大小. 
                                                    // memory 中的每个字节被表示为 "0xXX, " 或者 "0xXX\n " 的形式. 
    
    unsigned int fullLines = len / BYTES_PER_LINE;  // 满 16 字节的行数. 
    unsigned int leftBytes = len % BYTES_PER_LINE;  // 最后可能的不满行的 字节的个数. 

    if ( NULL == pStart || 0 == len )
    {
        return;
    }
    
    bufLen = (BYTES_PER_LINE * BUF_SIZE_PER_SRC_BYTE) + 1;  
    pBuf = (char*)malloc(bufLen); 
    if ( NULL == pBuf )
    {
        printf("no enough memory.");
        return;
    }

    // LOGD("from %p; length %d : ", pStart, len);

    /* 处理所有的 full line, ... */
    for ( j = 0; j < fullLines; j++ )
    {
        pTarget = pBuf;
        /* 遍历每个待 log 的 字节, ... */
        for ( i = 0; i < BYTES_PER_LINE; i++ )
        {
            /* 打印到 *pTarget. */
            snprintf(pTarget, 10000, "0x%02x, ", *pSource);

            pTarget += BUF_SIZE_PER_SRC_BYTE;
            pSource++;
        }
        
        *(++pTarget) = '\0';
        /* log out 当前行. */
        printf("\t%s\n", pBuf);
    }
    
    /* 处理最后的 不满的行. */
    leftBytes = len % BYTES_PER_LINE;
    if ( 0 != leftBytes )
    {
        pTarget = pBuf;

        for ( i = 0; i < leftBytes; i++ )
        {
            snprintf(pTarget, 10000, "0x%02x, ", *pSource);
            pTarget += BUF_SIZE_PER_SRC_BYTE;
            pSource++;
        }

        *(++pTarget) = '\0';
        printf("\t%s \n", pBuf);
    }
    
    free(pBuf);
    
    return;
}

static void teststrncpyin_round(){
    char sectionName[65] = {0};
    int i = 1;		// index "N" 从 1 开始编码. 
	int j = 1;
	while ( 1 )
	{
		/* 构建当前可能的 section name : Uses-permission-"i". */
		memset(sectionName, 0x00, sizeof(sectionName));
		strncpy(sectionName, "Uses-permission-",sizeof(sectionName)-1);			
		snprintf(sectionName + strlen("Uses-permission-"), sizeof(sectionName)-strlen("Uses-permission-"),"%d", j);

		/* 尝试设置为当前 section, 若失败... */
		if (j>10 )
		{
			break;
		}
        j++;
    }
    printf("sectionName=%s size =%d\n",sectionName,sizeof(sectionName));
}

static char* ensure(printbuffer *p,int needed)
{
	char *newbuffer;int newsize;
	if (!p || !p->buffer) return 0;
	needed+=p->offset;
	if (needed<=p->length) return p->buffer+p->offset;

	newsize=pow2gt(needed);
	newbuffer=(char*)cJSON_malloc(newsize);
	if (!newbuffer) {cJSON_free(p->buffer);p->length=0,p->buffer=0;return 0;}
	if (newbuffer) memcpy(newbuffer,p->buffer,p->length);
	cJSON_free(p->buffer);
	p->length=newsize;
	p->buffer=newbuffer;
	return newbuffer+p->offset;
}
 
// void showAsn1(ht_asn1::ASN1Base *base) {
 
// 	if (base->getType() == 0) {
// 		ht_asn1::ASN1Ctn* ctn = ((ht_asn1::ASN1Ctn*)base);
// 		for (int i = 0; i < ctn->getVal().size(); i++) {
// 			if (ctn->getVal()[i]->getType() == 0) {
// 				cout << "<" << ctn->getVal()[i]->getTypeStr() << ">" << endl;
// 					showAsn1(ctn->getVal()[i]);
// 				cout << "</" << ctn->getVal()[i]->getTypeStr() << ">" << endl;
// 			}
// 			else {
// 				cout << "<" << ctn->getVal()[i]->getTypeStr() << ">" << endl;
// 				if (ctn->getVal()[i]->getType() == 1) {
// 					ht_asn1::ASN1Byte * asn1 = (ht_asn1::ASN1Byte *)ctn->getVal()[i];
// 					printf("len: %d ,type:%d data:", asn1->getLen(), asn1->getStype());
// 					for (int j = 0; j < asn1->getLen(); j++) {
// 						printf("%02X", asn1->getVal()[j]);
// 					}
// 					printf("\n");
// 				}
// 				else if (ctn->getVal()[i]->getStype() == 12) {
// 					ht_asn1::ASN1String * asn1 = (ht_asn1::ASN1String*)ctn->getVal()[i];
// 					printf("len: %d ,type:%d data: %s", asn1->getLen(), asn1->getStype(), asn1->getVal().c_str());
					
// 				}
// 				else if (ctn->getVal()[i]->getType() == 2) {
// 					cout << ((ht_asn1::ASN1String*)ctn->getVal()[i])->getVal() << endl;
// 				}
// 				cout << "</" << ctn->getVal()[i]->getTypeStr() << ">"<<endl;
// 			}
// 		}
// 	}
	
// }
// void testAsn1() {
 
//     //test asn1结构
// 	const unsigned char pdata[] = {0x30,0x79,0x02,0x20,0x1A,0xF4,0xEE,0xBB,0x48,0xBA,0x93,0xE1,0xDB,0x9B,0xDC,0xCE,0x70,0x94,0x49,0x7E,0x8C,0x04,0x78,0x35,0x63,0xEF,0x88,0xC6,0x77,0x36,0x31,0x41,0x6A,0x6E,0x38,0x5D,0x02,0x21,0x00,0xF0,0x7C,0x45,0x31,0x4C,0x2D,0x09,0x36,0x65,0x05,0x6B,0x7E,0xD2,0x79,0x75,0xF6,0xEE,0xF3,0x60,0x0D,0xBD,0xB4,0x7B,0xF6,0x71,0x0A,0xF3,0x38,0x1C,0x45,0x74,0x0A,0x04,0x20,0x93,0xF3,0x30,0x18,0x0F,0x7B,0x48,0xE1,0x6A,0x8A,0xDD,0x56,0x36,0x53,0xFE,0xF5,0x6F,0x4C,0xCA,0xE0,0x89,0x91,0xFF,0x0B,0xB8,0x4F,0x05,0x75,0x01,0xBC,0xE7,0xBA,0x04,0x10,0x57,0x3D,0x43,0xCC,0xA8,0x6E,0x34,0xC8,0x07,0xAE,0x47,0xA5,0x41,0x44,0x35,0x03};
// 	int len = sizeof(pdata);
	
// 	ht_asn1::ASN1Util asn1util;
 
// 	asn1util.paramAsn1((unsigned char *)pdata, len);
 
// 	ht_asn1::ASN1Ctn*  ctn =asn1util.getAsn1();
 
// 	for (int i = 0; i < ctn->getVal().size(); i++) {
		
// 		showAsn1(ctn->getVal()[i]);
// 	}
 
// }

int main()
{
	char tlist_1[1024] = {0},tlist_2[1024]={0},tlist_3[3]={0};;
    char fname[7][8] = {"a1","b1","c1","d1","e1","f1","g1"};
    int i = 0, len_1,len_2 = 0;
 
    len_1 = snprintf(tlist_1,1024,"%s;",fname[0]);
    len_2 = snprintf(tlist_2,1024,"%s;",fname[0]);
 
    for(i=1;i<7;i++)
    {
        len_1 = snprintf(tlist_1,1024,"%s%s;",tlist_1,fname[i]);
        len_2 = sprintf(tlist_2,"%s%s;",tlist_2,fname[i]);
    }
	len_2 = sprintf(tlist_3,"%s%s;","testtesttesttest","tlist_3testtesttest");

    printf("hjz tlist_1: %s\n",tlist_1);
    printf("tlist_2: %s\n",tlist_2);
	printf("tlist_3: %s\n len_2==%d",tlist_3,len_2);
    char data[] = "This is a test data string for dumpMemory.";
    dumpMemory(data, strlen(data));

	// char a[]={'a','b','\0','c'};
	// printf("\n a isizeof =%d v==%d size_t =%d\n",sizeof(i),strlen(a),(size_t)(a));

	// static char warning[6] = {'w','h','z','z','h','j'};
    static char warning[64] = {'\0'};
    snprintf(warning, sizeof(warning) , "hjz get domain str err.");
	// snprintf(warning, 6, "get domain str err.");
    // snprintf(warning, 6, "xxxxxx");
	// printf("warning: %s sizeof(warning)=%d\n",warning,sizeof(warning));
    snprintf(warning, sizeof(warning),"unknown : 0x%x.", SFT_APK);
    printf("warning1: %s sizeof(warning)=%d\n",warning,sizeof(warning));

	char sectionName[65] = {0};
	strncpy(sectionName, "Uses-permission-",(sizeof(sectionName))-1);
    // strcpy(sectionName, "hjzUses-permission-");
    printf("sectionName=%s\n",sectionName);

    printf("hjz strlen=%d\n",strlen("Permission-"));
    teststrncpyin_round();
    char strtest1[20] = {0};
    snprintf(strtest1, sizeof(strtest1) , "hjz");
    printf("hjz strlen=%d sizeof=%d \n",strlen("strtest1"),sizeof(strtest1));
    snprintf(strtest1, sizeof(strtest1) , "hjz");

	time_t curr;
	struct tm *tTM = NULL;
	char pcurDate[24] = {0};

	time(&curr);
	tTM = localtime(&curr);
	snprintf(pcurDate,sizeof(pcurDate),"%04d-%d-%d %d:%d:%d\r\n",1900,12, 19,23,30,23);
    printf("hjz strlen=%d sprintf=%s \n",strlen(pcurDate),pcurDate);
    // char* pcurDates;
    // printf("hjz2 strlen=%d\n",strlen(pcurDates));
	char *strmoc=0;
	strmoc=(char*)cJSON_malloc(2);	/* special case for 0. */
	if (strmoc) strncpy(strmoc,"0",2);
    printf("strmoc=%s stringlen=%d \n",strmoc,strlen(strmoc));

    strncpy(strmoc,"\"\"",1);
    printf("strmoc44 =%s stringlen=%d \n",strmoc,strlen(strmoc));
    // int st=6;
	// printbuffer p;
	// p.buffer=(char*)cJSON_malloc(st);
	// p.length=st;
	// p.offset=0;
    // char *out=0;

    // out=ensure(&p,5);	if (out) strncpy(out,"null",5);
    // printf("ensuretest =%s stringlen=%d \n",out,strlen(out));


    static uint8_t domestic_crt_group_id[] = {0x06, 0x00};
    char srcPath[16] = {'\0'};
    strncpy(srcPath, "pDirPath",8);
    printf("srcPath2=%s stringlen=%d \n",srcPath,strlen(srcPath));
    strncat(srcPath, "SUFFIX_OFDASOHOFAFDADASDD",40);
    printf("srcPath3=%s stringlen=%d \n",srcPath,strlen(srcPath));
    printf("FAM_CRT_HEADER_MAGIC_NUM=%d \n",FAM_CRT_HEADER_MAGIC_NUM);

    printf("domestic_crt_group_id=%x \n",domestic_crt_group_id);

    //test for Asn1
    // testAsn1();

	return 0;
}
