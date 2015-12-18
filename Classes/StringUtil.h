#pragma once

#include <string>
#include "PlatformConfig.h"

#if (TARGET_PLATFORM == PLATFORM_WIN32)
#include <windows.h>
#endif

class StringUtil {
public:
	StringUtil();
	virtual ~StringUtil();

	static std::string __fastcall WStrToUTF8(const wchar_t* src);
	static std::string __fastcall StrToUTF8(const char* src);
	static bool __fastcall isUTF8(const char* src);

	static std::string __fastcall GBKToUTF8(const char* gbk);
	static std::string __fastcall UTF8ToGBK(const char* utf8);
	static std::string __fastcall UnicodeToUTF8(const wchar_t* unicode);
};