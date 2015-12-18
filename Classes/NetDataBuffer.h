#pragma once

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
		BufferNode* next;
	};

	NetDataBuffer();
	virtual ~NetDataBuffer();
	void __fastcall create();
	void __fastcall write(const char* data, int len);
	bool __fastcall read(char* buf, int len);
	bool __fastcall read(ByteArray* bytes);
	bool __fastcall send(BaseNet* net);
	int __fastcall receive(BaseNet* net);
	void __fastcall clear();

protected:
	BufferNode* _head;
	BufferNode* _write;
	BufferNode* _read;
};