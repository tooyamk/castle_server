#pragma once

#include <Winsock2.h>

class BaseNet;
class ByteArray;

class NetDataBuffer {
public:
	class BufferNode {
	public:
		enum State {
			FREE,
			OCCUPY,
			FULL
		};

		BufferNode() {
			state = FREE;
			next = nullptr;
		}
		const static int MAX_LEN = 1024;
		char buffer[MAX_LEN];
		unsigned short size;
		State state;
		sockaddr_in addr;
		BufferNode* next;
	};

	NetDataBuffer();
	virtual ~NetDataBuffer();
	void __fastcall create();
	void __fastcall write(const char* data, int len, sockaddr_in* addr);
	void __fastcall write(bool kcp, const char* data, unsigned short len, sockaddr_in* addr);
	bool __fastcall read(char* buf, int len);
	bool __fastcall read(ByteArray* bytes, sockaddr* addr);
	bool __fastcall send(BaseNet* net);
	int __fastcall receive(BaseNet* net);
	void __fastcall clear();

protected:
	BufferNode* _head;
	BufferNode* _write;
	BufferNode* _read;
};