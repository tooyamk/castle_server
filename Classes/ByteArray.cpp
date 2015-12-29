#include "ByteArray.h"

ByteArray::ByteArray(bool bigEndian, unsigned int size) {
	this->_bigEndian = bigEndian;

	_bytes = new char[size];
	_rawLength = size;
	_length = 0;
	_position = 0;
	_isExternalData = false;
}

ByteArray::ByteArray(bool bigEndian, char* bytes, unsigned int size) {
	this->_bigEndian = bigEndian;

	this->_bytes = bytes;
	_rawLength = size;
	_length = size;
	_position = 0;
	_isExternalData = true;
}

ByteArray::~ByteArray() {
	if (_bytes != nullptr) {
		if (!_isExternalData) {
			delete []_bytes;
		}
		_bytes = nullptr;
	}
}

const char* ByteArray::getBytes() const { return _bytes; }

unsigned int ByteArray::getBytesAvailable() {
	return _length - _position;
}

unsigned int ByteArray::getLength() { return _length; }

void ByteArray::setLength(unsigned int len) {
	len *= 1.3;

	if (len != _rawLength) {
		_resize(len);
	}

	_length = len;
	if (_position > _length) _position = _length;
}

void ByteArray::_resize(unsigned int len) {
	char* newBytes = new char[len];

	unsigned int min = len > _rawLength ? _rawLength : len;

	memcpy(newBytes, _bytes, min);
	_rawLength = len;

	if (_isExternalData) {
		_isExternalData = false;
	} else {
		delete[]_bytes;
	}

	_bytes = newBytes;
}

void ByteArray::_checkLength(unsigned int len) {
	len += _position;
	if (len > _length) {
		_length = len;
	}

	if (len > _rawLength) {
		_resize(len * 1.3);
	}
}

unsigned int ByteArray::getPosition() { return _position; }

void ByteArray::setPosition(unsigned int pos) {
	if (pos > _length) pos = _length;
	_position = pos;
}

char ByteArray::readChar() {
	if (_position + 1 > _length) return 0;

	char value;
	char* p = (char*)&value;

	p[0] = _bytes[_position++];

	return value;
}

unsigned char ByteArray::readUnsignedChar() {
	if (_position + 1 > _length) return 0;

	unsigned char value;
	char* p = (char*)&value;

	p[0] = _bytes[_position++];

	return value;
}

void ByteArray::writeChar(char value) {
	_checkLength(1);

	char* p = (char*)&value;
	_bytes[_position++] = p[0];
}

void ByteArray::writeUnsignedChar(unsigned char value) {
	_checkLength(1);

	char* p = (char*)&value;
	_bytes[_position++] = p[0];
}

short ByteArray::readShort() {
	if (_position + 2 > _length) return 0;

	short value;
	char* p = (char*)&value;

	if (_bigEndian) {
		p[1] = _bytes[_position++];
		p[0] = _bytes[_position++];
	} else {
		p[0] = _bytes[_position++];
		p[1] = _bytes[_position++];
	}

	return value;
}

unsigned short ByteArray::readUnsignedShort() {
	if (_position + 2 > _length) return 0;

	unsigned short value;
	char* p = (char*)&value;

	if (_bigEndian) {
		p[1] = _bytes[_position++];
		p[0] = _bytes[_position++];
	} else {
		p[0] = _bytes[_position++];
		p[1] = _bytes[_position++];
	}

	return value;
}

void ByteArray::writeShort(short value) {
	_checkLength(2);

	char* p = (char*)&value;

	if (_bigEndian) {
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[0];
	} else {
		_bytes[_position++] = p[0];
		_bytes[_position++] = p[1];
	}
}

void ByteArray::writeUnsignedShort(unsigned short value) {
	_checkLength(2);

	char* p = (char*)&value;

	if (_bigEndian) {
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[0];
	} else {
		_bytes[_position++] = p[0];
		_bytes[_position++] = p[1];
	}
}

int ByteArray::readInt() {
	if (_position + 4 > _length) return 0;

	int value;
	char* p = (char*)&value;

	if (_bigEndian) {
		p[3] = _bytes[_position++];
		p[2] = _bytes[_position++];
		p[1] = _bytes[_position++];
		p[0] = _bytes[_position++];
	} else {
		p[0] = _bytes[_position++];
		p[1] = _bytes[_position++];
		p[2] = _bytes[_position++];
		p[3] = _bytes[_position++];
	}

	return value;
}

unsigned int ByteArray::readUnsignedInt() {
	if (_position + 4 > _length) return 0;

	unsigned int value;
	char* p = (char*)&value;

	if (_bigEndian) {
		p[3] = _bytes[_position++];
		p[2] = _bytes[_position++];
		p[1] = _bytes[_position++];
		p[0] = _bytes[_position++];
	} else {
		p[0] = _bytes[_position++];
		p[1] = _bytes[_position++];
		p[2] = _bytes[_position++];
		p[3] = _bytes[_position++];
	}

	return value;
}

void ByteArray::writeInt(int value) {
	_checkLength(4);

	char* p = (char*)&value;

	if (_bigEndian) {
		_bytes[_position++] = p[3];
		_bytes[_position++] = p[2];
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[0];
	} else {
		_bytes[_position++] = p[0];
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[2];
		_bytes[_position++] = p[3];
	}
}

void ByteArray::writeUnsignedInt(unsigned int value) {
	_checkLength(4);

	char* p = (char*)&value;

	if (_bigEndian) {
		_bytes[_position++] = p[3];
		_bytes[_position++] = p[2];
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[0];
	} else {
		_bytes[_position++] = p[0];
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[2];
		_bytes[_position++] = p[3];
	}
}

float ByteArray::readFloat() {
	if (_position + 4 > _length) return 0;

	float value;
	char* p = (char*)&value;

	if (_bigEndian) {
		p[3] = _bytes[_position++];
		p[2] = _bytes[_position++];
		p[1] = _bytes[_position++];
		p[0] = _bytes[_position++];
	} else {
		p[0] = _bytes[_position++];
		p[1] = _bytes[_position++];
		p[2] = _bytes[_position++];
		p[3] = _bytes[_position++];
	}

	return value;
}

void ByteArray::writeFloat(float value) {
	_checkLength(4);

	char* p = (char*)&value;

	if (_bigEndian) {
		_bytes[_position++] = p[3];
		_bytes[_position++] = p[2];
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[0];
	} else {
		_bytes[_position++] = p[0];
		_bytes[_position++] = p[1];
		_bytes[_position++] = p[2];
		_bytes[_position++] = p[3];
	}
}

void ByteArray::readBytes(char* bytes, unsigned int offset, unsigned int length) {
	if (length == 0) {
		if (_length <= _position) return;

		length = _length - _position;
	} else {
		unsigned int len = _length - _position;
		if (length > len) length = len;
	}

	for (unsigned int i = 0; i < length; i++) {
		bytes[offset++] = this->_bytes[_position++];
	}
}

void ByteArray::readBytes(ByteArray* ba, unsigned int offset, unsigned int length) {
	if (length == 0) {
		if (_length <= _position) return;

		length = _length - _position;
	} else {
		unsigned int len = _length - _position;
		if (length > len) length = len;
	}

	unsigned int pos = ba->getPosition();
	ba->setPosition(offset);
	ba->writeBytes((char*)_bytes, _position, length);
	ba->setPosition(pos);

	_position += length;
}

void ByteArray::writeBytes(char* bytes, unsigned int offset, unsigned int length) {
	if (length > 0) {
		_checkLength(length);

		for (unsigned int i = 0; i < length; i++) {
			this->_bytes[_position++] = bytes[offset++];
		}
	}
}

void ByteArray::writeBytes(ByteArray* ba, unsigned int offset, unsigned int length) {
	if (length == 0) {
		if (ba->getLength() <= offset) return;

		length = ba->getLength() - offset;
	}

	const char* baBytes = ba->getBytes();

	_checkLength(length);

	for (unsigned int i = 0; i < length; i++) {
		this->_bytes[_position++] = baBytes[offset++];
	}
}

std::string ByteArray::readString() {
	bool find = false;
	unsigned int pos = 0;

	for (unsigned int i = _position; i < _length; i++) {
		if (_bytes[i] == '\0') {
			pos = i;
			find = true;
			break;
		}
	}

	if (find) {
		unsigned int len = pos - _position;
		char* c = new char[len + 1];
		for (unsigned int i = 0; i <= len; i++) {
			c[i] = _bytes[_position++];
		}

		std::string str(c);
		delete[]c;

		return str;
	} else {
		return "";
	}
}

std::string ByteArray::readString(unsigned int start, unsigned int end) {
	if (start > _length - 1) start = _length - 1;
	if (end > _length - 1) end = _length - 1;
	if (end < start) end = start;

	unsigned int len = end - start + 1;
	bool hasEnd = _bytes[start + len - 1] == '\0';

	char* c = new char[len + (hasEnd ? 0 : 1)];
	for (unsigned int i = 0; i < len; i++) {
		c[i] = _bytes[start + i];
	}

	if (!hasEnd) c[len] = '\0';

	std::string str(c);
	delete[]c;

	return str;
}

void ByteArray::writeString(const std::string& str) {
	writeString(str.c_str());
}

void ByteArray::writeString(const char* str) {
	std::string s = StringUtil::StrToUTF8(str);
	unsigned int len = s.size();

	_checkLength(len + 1);

	const char* c = s.c_str();

	for (unsigned int i = 0; i < len; i++) {
		_bytes[_position++] = c[i];
	}
	_bytes[_position++] = '\0';
}

bool ByteArray::readBool() {
	if (_position + 1 > _length) return false;
	return _bytes[_position++] != 0;
}

void ByteArray::writeBool(bool b) {
	_checkLength(1);

	_bytes[_position++] = b ? 1 : 0;
}

void ByteArray::popFront(unsigned int len) {
	if (_length <= len) {
		_length = 0;
		_position = 0;
	} else {
		_length -= len;
		for (unsigned int i = 0; i < _length; i++) {
			_bytes[i] = _bytes[len + i];
		}
		if (_position <= len) {
			_position = 0;
		}
		else {
			_position -= len;
		}
	}
}