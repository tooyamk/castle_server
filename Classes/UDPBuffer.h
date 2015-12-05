#pragma once

#include <Winsock2.h>

class BaseUDP;

class UDPBuffer {
public:
	class DataBuffer {
	public:
		enum State {
			FREE,
			OCCUPY,
			FULL
		};

		DataBuffer() {
			state = FREE;
			next = nullptr;
		}
		const static int MAX_LEN = 1024;
		char buffer[MAX_LEN];
		unsigned short size;
		State state;
		DataBuffer* next;
		sockaddr addr;
	};

	UDPBuffer();
	~UDPBuffer();
	void __fastcall create();
	void __fastcall write(const char* data, int len);
	bool __fastcall read(char* buf, int len);
	bool __fastcall send(BaseUDP* udp);
	int __fastcall receive(BaseUDP* udp);
	void __fastcall clear();

protected:
	DataBuffer* _head;
	DataBuffer* _write;
	DataBuffer* _read;
};