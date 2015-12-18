#pragma once

#include "StringUtil.h"

class ByteArray {
public:
	ByteArray(bool bigEndian, unsigned int size=32);
	ByteArray(bool bigEndian, unsigned char* bytes, unsigned int size);
	virtual ~ByteArray();

	const unsigned char* __fastcall getBytes() const;

	unsigned int __fastcall getLength();
	void __fastcall setLength(unsigned int len);

	unsigned int __fastcall getPosition();
	void __fastcall setPosition(unsigned int pos);

	unsigned int __fastcall getBytesAvailable();

	char __fastcall readChar();
	unsigned char __fastcall readUnsignedChar();
	void __fastcall writeChar(char value);
	void __fastcall writeUnsignedChar(unsigned char value);

	short __fastcall readShort();
	unsigned short __fastcall readUnsignedShort();
	void __fastcall writeShort(short value);
	void __fastcall writeUnsignedShort(unsigned short value);

	int __fastcall readInt();
	unsigned int __fastcall readUnsignedInt();
	void __fastcall writeInt(int value);
	void __fastcall writeUnsignedInt(unsigned int value);

	float __fastcall readFloat();
	void __fastcall writeFloat(float value);

	void __fastcall readBytes(char* bytes, unsigned int offset = 0, unsigned int length = 0);
	void __fastcall readBytes(ByteArray* ba, unsigned int offset = 0, unsigned int length = 0);
	void __fastcall writeBytes(char* bytes, unsigned int offset, unsigned int length);
	void __fastcall writeBytes(ByteArray* ba, unsigned int offset = 0, unsigned int length = 0);

	std::string __fastcall readString();
	std::string __fastcall readString(unsigned int start, unsigned int end);
	void __fastcall writeString(const std::string& str);
	void __fastcall writeString(const char* str);

	bool __fastcall readBool();
	void __fastcall writeBool(bool b);

	void __fastcall popFront(unsigned int len);

protected:
	unsigned char* _bytes;
	unsigned int _position;
	unsigned int _length;
	unsigned int _rawLength;
	bool _bigEndian;
	bool _isExternalData;

	void __fastcall _resize(unsigned int len);
	void __fastcall _checkLength(unsigned int len);
};