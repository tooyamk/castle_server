#include "NetDataBuffer.h"
#include "BaseNet.h"
#include "ByteArray.h"

NetDataBuffer::NetDataBuffer() {
	_head = nullptr;
}

NetDataBuffer::~NetDataBuffer() {
}

void NetDataBuffer::create() {
	clear();

	_head = new BufferNode();
	BufferNode* tail = new BufferNode();
	_head->next = tail;
	_write = tail;
	_read = _head;
	_read->state = BufferNode::OCCUPY;
}

void NetDataBuffer::write(const char* data, int len) {
	if (_head == nullptr) return;

	BufferNode* cur = _write;

	for (int i = 0; i < len; i++) {
		cur->buffer[i] = data[i];
	}

	if (cur->next == nullptr) {
		if (_head->state == BufferNode::FREE) {
			cur->next = _head;
			_head = _head->next;
			cur->next->next = nullptr;
		} else {
			cur->next = new BufferNode();
		}
	}
	_write = cur->next;

	cur->size = len;
	cur->state = BufferNode::FULL;
}

bool NetDataBuffer::read(char* buf, int len) {
	BufferNode* cur = _read;
	BufferNode* next = cur->next;
	if (next != nullptr && next->state == BufferNode::FULL) {
		next->state = BufferNode::OCCUPY;

		if (buf != nullptr) {
			for (int i = 0; i < next->size; i++) {
				buf[i] = next->buffer[i];
			}
		}

		_read = next;
		cur->state = BufferNode::FREE;

		return true;
	} else {
		return false;
	}
}

bool NetDataBuffer::read(ByteArray* bytes) {
	BufferNode* cur = _read;
	BufferNode* next = cur->next;
	if (next != nullptr && next->state == BufferNode::FULL) {
		next->state = BufferNode::OCCUPY;

		bytes->writeBytes(next->buffer, 0, next->size);

		_read = next;
		cur->state = BufferNode::FREE;

		return true;
	} else {
		return false;
	}
}

bool NetDataBuffer::send(BaseNet* net) {
	if (net->getState() == ConnectState::CONNECTED) {
		BufferNode* cur = _read;
		BufferNode* next = cur->next;
		if (next != nullptr && next->state == BufferNode::FULL) {
			next->state = BufferNode::OCCUPY;

			net->sendData(next->buffer, next->size);

			_read = next;
			cur->state = BufferNode::FREE;

			return true;
		} else {
			return false;
		}
	} else {
		return read(nullptr, 0);
	}
}

int NetDataBuffer::receive(BaseNet* net) {
	if (_head == nullptr || net->getState() != ConnectState::CONNECTED) return -1;

	BufferNode* cur = _write;

	int len = net->receiveData(cur->buffer, BufferNode::MAX_LEN);
	if (len > 0) {
		if (cur->next == nullptr) {
			if (_head->state == BufferNode::FREE) {
				cur->next = _head;
				_head = _head->next;
				cur->next->next = nullptr;
			} else {
				cur->next = new BufferNode();
			}
		}
		_write = cur->next;

		cur->size = len;
		cur->state = BufferNode::FULL;
	}

	return len;
}

void NetDataBuffer::clear() {
	if (_head != nullptr) {
		BufferNode* data = _head;
		do {
			BufferNode* next = data->next;
			delete data;
			data = next;
		} while (data != nullptr);

		_head = nullptr;
	}
}