#include "ASN1Util.h"
#include <stdio.h>
#include <iostream>
using namespace std;
/*
*
* TLC  T类型定义 L长度 V值
*
* *****************************
*类型定义解析：
*
bit8~bit7:
第一个字节 bit8~bit7 表明tag所属类型
00 通用级 universal class
01 应用级 application class
10 规范级 context-specific class
11 私有级 private class
bit6:
Primitive (P)	0	数据内容仅由一个数据元素组成
Constructed (C)	1	数据内容由多个数据元素组成
第一个字节 bit6 表明本tag的value部分是否包含子tag，1则表示是Constructed（复合）的，为0表示是Primitive（原始）；
第一个字节 bit5~bit1 表明是否有第二个字节一起表示本tag，11111 全为1表示有第二个字节，即非11111的都没有第二个字节；
第二个字节 bit8 为1，同时bit7~bit1 大于0 表示有下一个tag
第三~N个字节同第二个字节
bit5~bit1:
bit
类型
数据类型	编码类型	Tag number(十进制)	Tag number(十六进制)
Decimal	Hexadecimal
End-of-Content (EOC)	Primitive	0	0
BOOLEAN	Primitive	1	1
INTEGER	Primitive	2	2
BIT STRING	Both	3	3
OCTET STRING	Both	4	4
NULL	Primitive	5	5
OBJECT IDENTIFIER	Primitive	6	6
Object Descriptor	Both	7	7
EXTERNAL	Constructed	8	8
REAL (float)	Primitive	9	9
ENUMERATED	Primitive	10	A
EMBEDDED PDV	Constructed	11	B
UTF8String	Both	12	C
RELATIVE-OID	Primitive	13	D
TIME	Primitive	14	E
Reserved		15	F
SEQUENCE and SEQUENCE OF	Constructed	16	10
SET and SET OF	Constructed	17	11
NumericString	Both	18	12
PrintableString	Both	19	13
T61String	Both	20	14
VideotexString	Both	21	15
IA5String	Both	22	16
UTCTime	Both	23	17
GeneralizedTime	Both	24	18
GraphicString	Both	25	19
VisibleString	Both	26	1A
GeneralString	Both	27	1B
UniversalString	Both	28	1C
CHARACTER STRING	Constructed	29	1D
BMPString	Both	30	1E
DATE	Primitive	31	1F
TIME-OF-DAY	Primitive	32	20
DATE-TIME	Primitive	33	21
DURATION	Primitive	34	22
OID-IRI	Primitive	35	23
RELATIVE-OID-IRI	Primitive	36	24
******************************
L长度解析：
bit8：
1： 后面字节为长度
0:当前bit7~bit1表示长度
*/
namespace ht_asn1 {
 
	ASN1Util::ASN1Util() {
 
	}
 
 
	ASN1Util::~ASN1Util() {
 
		asn1Ctn.free();
	}
 
	int ASN1Util::paramVal(unsigned char* content, int* start, int contenlen, int len, int isZero, int sub, ASN1Ctn* asn1ctn) {
		int pos = 0;
		int tagclass = 0;
		int pc = 0;
		int type = 0;
		int val_len = 0;
		unsigned char* buf = (unsigned char*)(content + *start);
		len = len - *start;
		//printf("%s tag:%02X  len:(%d,%d) ", sub == 0 ? "sub" : "main", buf[0], contenlen, len);
		//求出tag
		tagclass = (buf[pos] & 0xC0) >> 6;
		pc = (buf[pos] & 0x20) >> 5;
		type = (buf[pos] & 0x1F);
		if ((type & 0x0F) == 0x0F) {
			type = 0;
			for (; pos < contenlen; pos++) {
				if ((buf[pos] & 0x80) == 0) {
					type = (type << 8) | buf[pos];
				}
				else {
					type = (type << 8) | (buf[pos] & 0x7F);
					break;
				}
			}
		}
		//cout << "tagclass:" << tagclass << ",pc:" << pc << ",type:" << type;
		pos++;
		//求出长度
		//L长度解析：
		//	bit8：
		//	1： 后面字节为长度
		//	0:当前bit7~bit1表示长度
		val_len = 0;
		if ((buf[pos] & 0x80) != 0) {
			int byte_len = (buf[pos] & 0x7F);
			if (byte_len == 0x7F || byte_len == 0x00) {
				val_len = contenlen;
				isZero = (byte_len == 0x7F ? 1 : 0);
				pos++;
			}
			else {
				pos++;
				for (int i = 0; i < byte_len && pos < contenlen; i++, pos++) {
					val_len = (val_len << 8) | (buf[pos]);
				}
			}
		}
		else {
			val_len = (val_len << 8) | (buf[pos++]);
		}
		//cout << ",len:" << val_len << ",pos:" << pos<<",contenlen:"<< contenlen << endl;
		//结构包含结构
		if (pc == 1) {
			sub = 1;
			do {
				ASN1Ctn* asnctn = new ASN1Ctn();
				if (asnctn == NULL) {
					return -1;
				}
				asn1ctn->add(asnctn);
				paramVal(buf, &pos, sub == 0 ? contenlen : val_len, len, isZero, sub, asnctn);
				sub = 0;
			} while (pos < len);
		}
		else {
			if (type == 22 || type == 12) {//字符串
				ASN1String* asn1 = new ASN1String();
				if (asn1 == NULL) {
					return -1;
				}
				asn1->type = 2;
				asn1->stype = type;
				asn1->len = val_len;
				asn1->val.append((char*)(buf + pos), val_len);
				asn1ctn->add(asn1);
			}
			else {
				//if (type == 2 || type == 4 || type == 3) {//整形1
				ASN1Byte* asn1 = new ASN1Byte();
				if (asn1 == NULL) {
					return -1;
				}
				asn1->type = 1;
				asn1->stype = type;
				asn1->val = new unsigned char[val_len];
				if (asn1->val == NULL) {
					delete asn1;
					return -1;
				}
				asn1->len = val_len;
				memcpy(asn1->val, buf + pos, val_len);
				asn1ctn->add(asn1);
			}
			//int outPos = pos;
			//printf("val=");
			//for (int i = 0; i < val_len && outPos < contenlen; i++, outPos++) {
			//	printf("0x%02X ", buf[outPos]);
			//}
			//cout << endl;
			pos += val_len;
			//cout << "pos:" << pos << "，contenlen：" << contenlen << endl;
			//if (pos > contenlen) pos = contenlen;
		}
		*start = pos + *start;
 
		return 0;
	}
	int ASN1Util::paramAsn1(unsigned char* buf, int len) {
		int pos = 0;
		//1111 1111
		do {
			/*
			* bit8~bit7:
				第一个字节 bit8~bit7 表明tag所属类型
				bit6:
				Primitive (P)	0	数据内容仅由一个数据元素组成
				Constructed (C)	1	数据内容由多个数据元素组成
				第一个字节 bit6 表明本tag的value部分是否包含子tag，1则表示是Constructed（复合）的，为0表示是Primitive（原始）；
				第一个字节 bit5~bit1 表明是否有第二个字节一起表示本tag，11111 全为1表示有第二个字节，即非11111的都没有第二个字节；
				第二个字节 bit8 为1，同时bit7~bit1 大于0 表示有下一个tag
				第三~N个字节同第二个字节
			*/
 
			if (paramVal(buf, &pos, len, len, -1, 0, &asn1Ctn) < 0){
				return -1;
			}
		} while (pos < len);
 
		return 0;
	}
 
	// 默认-1 0容器(序列） 1 字节存储类型 2字符串 3 集合 4 枚举
	const char* ASN1Base::getTypeStr() {
		if (type == 0) {
			return "SEQUENCE";
		}
		else if (type == 1) {
			return "bytes";
		}
		else if (type == 2) {
			return "string";
		}
		else if (type == 3) {
			return "set";
		}
		else if (type == 4) {
			return "enum";
		}
 
		return "unkown";
	}
	ASN1Ctn* ASN1Util::getAsn1() {
		return &(this->asn1Ctn);
	}
 
 
	ASN1Byte::~ASN1Byte() {
		if (val != NULL) {
			delete val;
		}
	}
 
	ASN1Ctn::ASN1Ctn() {
		this->type = 0;
		this->len = 0;
	}
	void ASN1Ctn::add(ASN1Base* base) {
		val.push_back(base);
		this->len++;
	}
 
	void ASN1Ctn::free() {
		for (auto it = val.begin(); it != val.end(); ++it) {
			// *it 获取当前迭代器指向的值
			if (((ASN1Base*)*it)->getType() == 0) {
				((ASN1Ctn*)*it)->free();
			}
			else if (((ASN1Base*)*it)->getType() == 1) {
				delete ((ASN1Byte*)*it)->getVal();
			}
			delete* it;
			*it = NULL;
		}
	}
}