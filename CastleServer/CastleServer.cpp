// CastleServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../Classes/Server.h"
#include <windows.h>

/* 网络API的动态链接库 */
#pragma comment(lib, "ws2_32.lib")

int _tmain(int argc, _TCHAR* argv[]) {
	auto s = new Server();
	s->run();

	while (true) {
		Sleep(1);
	}

	return 0;
}