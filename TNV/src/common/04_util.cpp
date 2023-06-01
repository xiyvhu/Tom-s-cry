//公共模块
//定义几个实用函数

#include <string.h>

#include "01_types.h"
#include "03_util.h"

//long long类型整数 主机序 转 网络序
void llton(long long ll,char* n){
	for(size_t i = 0;i < sizeof(ll);++i){
		n[i] = ll >> (sizeof(ll) - i - 1) * 8;
	}
}
/*
#include <arpa/inet.h>

long long htonll(long long value) {
    // 判断主机字节序
    int is_little_endian = 1;
    if (*(char *)&is_little_endian == 1) {
        // 如果是小端字节序，需要转换为大端字节序
        return (((long long)htonl(value)) << 32) + htonl(value >> 32);
    } else {
        // 如果是大端字节序，不需要转换
        return value;
    }
}
在上面的代码中，我们使用了arpa/inet.h头文件中的htonl函数
该函数可以将32位整数从主机字节序转换为网络字节序
对于long long类型整数，我们需要将其高32位和低32位分别进行转换，然后拼接起来得到最终结果
在判断主机字节序时，我们使用了一个小技巧
即定义一个int类型的变量is_little_endian，并将其地址转换为char类型指针
然后判断该指针所指向的值是否为1，如果是，则说明主机字节序为小端字节序，否则为大端字节序。
*/

//long long类型整数 网络序 转 主机序
long long ntoll(char const* n){
	long long ll = 0;
	for(size_t i = 0;i < sizeof(ll);i++){
		ll |= (long long)(unsigned char)n[i] << (sizeof(ll) - i - 1) * 8;
	}
	return ll;
}

//long类型整数 主机序 转 网络序
void lton(long l,char* n){
	for(size_t i = 0;i < sizeof(l);++i){
		n[i] = l >> (sizeof(l) - i - 1) * 8;
	}
}

//long类型整数 网络序 转 主机序
long ntol(char const* n){
	long l = 0;
	for(size_t i = 0;i < sizeof(l);i++){
		l |= (long)(unsigned char)n[i] << (sizeof(l) - i - 1) * 8;
	}
	return l;
}

//short类型整数 主机序 转 网络序
void ston(short s,char* n){
	for(size_t i = 0;i < sizeof(s);++i){
		n[i] = s >> (sizeof(s) - i - 1) * 8;
	}
}

//short类型整数 网络序 转 主机序
short ntos(char const* n){
	short s = 0;
	for(size_t i = 0;i < sizeof(s);i++){
		s |= (short)(unsigned char)n[i] << (sizeof(s) - i - 1) * 8;
	}
	return s;
}

//字符串是否合法，即是否只包含26个英文字母大小写和0到9十个阿拉伯数字
int valid(char const* str){
	if(!str){
		return ERROR;
	}
	
	size_t len = sizeof(str);
	if(!len){
		return ERROR;
	}
	
	for(size_t i = 0;i < len;i++){
		if(!(('a' <= str[i] && str[i] <= 'z') ||
		     ('A' <= str[i] && str[i] <= 'Z') ||
		     ('0' <= str[i] && str[i] <= '9'))){
				 return ERROR;
			 }
	}
	return OK;
}
/*
#include <ctype.h>

int is_valid_string(const char *str) {
    while (*str) {
        if (!isalnum(*str)) {
            // 如果不是字母或数字，返回0
            return 0;
        }
        str++;
    }
    return 1;
}
在上面的代码中，我们使用了ctype.h头文件中的isalnum函数，该函数可以判断一个字符是否为字母或数字
我们遍历字符串中的每个字符，如果发现有不是字母或数字的字符，就返回0，表示字符串不合法
如果遍历完整个字符串都没有发现不合法的字符，就返回1，表示字符串合法
需要注意的是，上述代码中的字符串参数是const char *类型，即指向常量字符的指针，这是因为我们不需要修改字符串中的内容，只需要读取它们的值进行判断即可。
*/

//以分号为分隔符将一个字符串拆分为多个子字符串
int split(char const* str,std::vector<std::string>& substrs){
	if(!str){
		return ERROR;
	}
	
	size_t len = sizeof(str);
	if(!len){
		return ERROR;
	}
	
	char* buf = new char[len + 1];
	strcpy(buf,str);
	
	char const* sep = ";";
	for(char* substr = strtok(buf,sep);substr;substr = strtok(NULL,sep)){
		substrs.push_back(substr);
	}
	
	delete[]buf;
	
	return OK;
}