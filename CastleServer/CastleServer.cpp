// CastleServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "../Classes/Server.h"
#include <windows.h>

/* ����API�Ķ�̬���ӿ� */
#pragma comment(lib, "ws2_32.lib")

int _tmain(int argc, _TCHAR* argv[]) {
	auto s = new Server();
	s->run();

	while (true) {
		Sleep(1);
	}

	return 0;
}