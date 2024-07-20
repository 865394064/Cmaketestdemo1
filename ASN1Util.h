#ifndef __ASN1_UTIL_H__
#define __ASN1_UTIL_H__
#include <string>
#include <vector>
//ANS.1 test
namespace ht_asn1 {
 
	//基础结构
	class ASN1Base {
	public:
		int getType() {
			return this->type;
		}
		int getStype() {
			return this->stype;
		}
		int getLen() {
			return this->len;
		}
		const char* getTypeStr();
		friend class ASN1Util;
	protected:
		int type;//类型 默认-1 0容器(序列） 1 字节存储类型 2字符串 3 集合 5 枚举
		int stype;//原始类型
		int len;
	};
 
 
	//容器结构
	class ASN1Ctn : public ASN1Base {
	public :
		ASN1Ctn();
 
		std::vector<ASN1Base*> getVal() {
			return this->val;
		}
 
		void add(ASN1Base* base);
		//释放
		void free();
		friend class ASN1Util;
	protected:
		std::vector<ASN1Base*> val;
	};
 
 
 
	//字符串
	class ASN1String : public ASN1Base {
	public:
		std::string getVal() {
			return this->val;
		}
		friend class ASN1Util;
	protected:
		std::string val;
	};
 
	//字节数组
	class ASN1Byte : public ASN1Base {
	public:
		~ASN1Byte();
	public:
		unsigned char* getVal() {
			return this->val;
		}
		friend class ASN1Util;
	protected:
		unsigned char* val;
	};
 
 
	//asn.1操作类
	class ASN1Util
	{
	public:
		ASN1Util();
		~ASN1Util();
	private:
		//解析值
		int paramVal(unsigned char* content, int* start, int contenlen, int len, int isZero, int sub, ASN1Ctn* asn1ctn);
	public:
 
		//解析整体结构
		int paramAsn1(unsigned char* buf, int len);
 
		ASN1Ctn* getAsn1();
	private:
		ASN1Ctn asn1Ctn;
	};
 
}





#endif