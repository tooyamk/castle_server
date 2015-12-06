#include "UDPBuffer.h"
#include "BaseUDP.h"

UDPBuffer::UDPBuffer() {
	_head = nullptr;
}

UDPBuffer::~UDPBuffer() {
}

void UDPBuffer::create() {
	clear();

	_head = new DataBuffer();
	DataBuffer* tail = new DataBuffer();
	_head->next = tail;
	_write = tail;
	_read = _head;
	_read->state = DataBuffer::OCCUPY;
}

void UDPBuffer::write(const char* data, int len) {
	if (_head == nullptr) return;

	DataBuffer* cur = _write;

	for (int i = 0; i < len; i++) {
		cur->buffer[i] = data[i];
	}

	if (cur->next == nullptr) {
		if (_head->state == DataBuffer::FREE) {
			cur->next = _head;
			_head = _head->next;
			cur->next->next = nullptr;
		} else {
			cur->next = new DataBuffer();
			_write = cur->next;
		}
	} else {
		_write = cur->next;
	}

	cur->size = len;
	cur->state = DataBuffer::FULL;
}

bool UDPBuffer::read(char* buf, int len, sockaddr* addr) {
	DataBuffer* cur = _read;
	DataBuffer* next = cur->next;
	if (next != nullptr && next->state == DataBuffer::FULL) {
		next->state = DataBuffer::OCCUPY;

		if (buf != nullptr) {
			for (int i = 0; i < next->size; i++) {
				buf[i] = next->buffer[i];
			}
		}
		if (addr != nullptr) {
			addr->sa_family = next->addr.sa_family;
			for (int i = 0; i < 14; i++) {
				addr->sa_data[i] = next->addr.sa_data[i];
			}
		}

		_read = next;
		cur->state = DataBuffer::FREE;

		return true;
	} else {
		return false;
	}
}

bool UDPBuffer::send(BaseUDP* udp) {
	if (udp->getState() == UDPState::CONNECTED) {
		DataBuffer* cur = _read;
		DataBuffer* next = cur->next;
		if (next != nullptr && next->state == DataBuffer::FULL) {
			next->state = DataBuffer::OCCUPY;

			udp->send(next->buffer, next->size, &next->addr);

			_read = next;
			cur->state = DataBuffer::FREE;

			return true;
		} else {
			return false;
		}
	} else {
		return read(nullptr, 0, nullptr);
	}
}

int UDPBuffer::receive(BaseUDP* udp) {
	if (_head == nullptr || udp->getState() != UDPState::CONNECTED) return -1;

	DataBuffer* cur = _write;

	int len = udp->receive(cur->buffer, DataBuffer::MAX_LEN, &cur->addr);
	if (len > 0) {
		if (cur->next == nullptr) {
			if (_head->state == DataBuffer::FREE) {
				cur->next = _head;
				_head = _head->next;
				cur->next->next = nullptr;
			} else {
				cur->next = new DataBuffer();
				_write = cur->next;
			}
		} else {
			_write = cur->next;
		}

		cur->size = len;
		cur->state = DataBuffer::FULL;
	}

	return len;
}

void UDPBuffer::clear() {
	if (_head != nullptr) {
		DataBuffer* data = _head;
		do {
			DataBuffer* next = data->next;
			delete data;
			data = next;
		} while (data != nullptr);

		_head = nullptr;
	}
}