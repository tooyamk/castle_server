#include "StringUtil.h"
#if (TARGET_PLATFORM == PLATFORM_IOS || TARGET_PLATFORM == PLATFORM_MAC)
#import <Foundation/Foundation.h>
#include <stdlib.h>
#endif

StringUtil::StringUtil() {
}

StringUtil::~StringUtil() {
}

bool StringUtil::isUTF8(const char* src) {
	int i = 0;
	int nBytes = 0;
	unsigned char chr;
	unsigned long slen = strlen(src);
	unsigned long asicount = 0;
	while (true) {
		chr = *(src + i);
		if (chr == '\0') break;
		i++;

		if ((chr & 0x80) == 0)asicount++;

		if (nBytes == 0) {
			if (chr >= 0x80) {
				if (chr >= 0xFC && chr <= 0xFD) {
					nBytes = 6;
				} else if (chr >= 0xF8) {
					nBytes = 5;
				} else if (chr >= 0xF0) {
					nBytes = 4;
				} else if (chr >= 0xE0) {
					nBytes = 3;
				} else if (chr >= 0xC0) {
					nBytes = 2;
				} else {
					return false;
				}
				nBytes--;
			}
		} else {
			if ((chr & 0xC0) != 0x80) {
				return false;
			}
			nBytes--;
		}
	}

	if (nBytes > 0) {
		return false;
	}

	if (asicount >= slen) {
		return false;
	}

	return true;
}

std::string StringUtil::GBKToUTF8(const char* gbk) {
	return StrToUTF8(gbk);
}

std::string StringUtil::UTF8ToGBK(const char* utf8) {
#if (TARGET_PLATFORM == PLATFORM_WIN32)
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
#endif
#if (TARGET_PLATFORM == PLATFORM_IOS || TARGET_PLATFORM == PLATFORM_MAC)
	NSStringEncoding en_utf8 = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
	NSStringEncoding en_gbk = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingGB_18030_2000);
	NSString* data = [[NSString alloc] initWithCString:utf8 encoding : en_utf8];
	std::string str = [data cStringUsingEncoding : en_gbk];
	return str;
#else
	return "";
#endif
}

std::string StringUtil::UnicodeToUTF8(const wchar_t* unicode) {
	return WStrToUTF8(unicode);
}

std::string StringUtil::WStrToUTF8(const wchar_t* src) {
#if (TARGET_PLATFORM == PLATFORM_WIN32)
	int len = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, src, -1, str, len, NULL, NULL);
	std::string strTemp(str);
	if (str) delete[] str;
	return strTemp;
#endif
#if (TARGET_PLATFORM == PLATFORM_IOS || TARGET_PLATFORM == PLATFORM_MAC)
	size_t len = wcslen(src);
	char *cSrc ;
	cSrc= (char*)malloc(len*sizeof(char));
	wcstombs(cSrc, src, len);
	NSStringEncoding en_unicode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUnicode);
	NSStringEncoding en_utf8 = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
	NSString* data = [[NSString alloc] initWithCString:cSrc encoding:en_unicode];
	std::string str = [data cStringUsingEncoding:en_utf8];
	return str;
#else
	return "";
#endif
}

std::string StringUtil::StrToUTF8(const char* src) {
	if (isUTF8(src)) return std::string(src);

#if (TARGET_PLATFORM == PLATFORM_WIN32)
	int len = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src, -1, wstr, len);

	std::string utf = WStrToUTF8(wstr);
	if (wstr) delete[] wstr;
	return utf;
#endif
#if (TARGET_PLATFORM == PLATFORM_IOS || TARGET_PLATFORM == PLATFORM_MAC)
	NSStringEncoding en_utf8 = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
	NSStringEncoding en_gbk = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingGB_18030_2000);
	NSString* data = [[NSString alloc] initWithCString:src encoding:en_gbk];
	std::string str = [data cStringUsingEncoding:en_utf8];
	return str;
#else
	return src;
#endif
}