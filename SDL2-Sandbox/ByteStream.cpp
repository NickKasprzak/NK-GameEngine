#include "ByteStream.h"
#include <cassert>
#include <memory>

namespace Funny
{
	/*
	* Updates stream to contain the byte data passed in,
	* updating the streams data to reflect it.
	* 
	* Used for read operations when deserializing packets
	*/
	void ByteStream::setBuffer(void* inBuff, size_t inBytes)
	{
		assert(inBytes < MAX_STREAM_BYTES && "Given buffer is too large for stream to handle.");

		memcpy(m_Buffer, inBuff, inBytes);
		m_Bytes = inBytes;
		m_StreamPos = 0;
	}

	/*
	* Gets the byte data thats been written to the stream,
	* passing it to an external char array and int.
	* 
	* Used for write operations when passing serialized data
	* to a Send operation.
	*/
	void ByteStream::getBuffer(void* outBuff, size_t outBuffSize, size_t& outBytes)
	{
		assert(outBuffSize >= m_Bytes && "Given buffer is too small to accept the specified byte count.");
		
		memcpy(outBuff, m_Buffer, m_Bytes);
		outBytes = m_Bytes;
	}

	/*
	* Reads byte data from the byte stream as an int and
	* advances the stream's position by the bytes read.
	* 
	* Should return an integer.
	*/
	int ByteStream::readInt()
	{
		assert(m_StreamPos + sizeof(int) <= m_Bytes && "Buffer doesn't contain enough bytes to form an int.");

		int byteToInt = 0;
		memcpy(&byteToInt, (char*)(m_Buffer + m_StreamPos), sizeof(int));
		m_StreamPos += sizeof(int);
		return byteToInt;
	}

	/*
	* Writes the given integer to the byte stream as its
	* corresponding byte data and advances the stream's
	* byte position.
	*/
	void ByteStream::writeInt(int writeInt)
	{
		assert(m_StreamPos + sizeof(int) < MAX_STREAM_BYTES && "Buffer doesn't contain enough space to add an int.");

		memcpy((char*)(m_Buffer + m_StreamPos), &writeInt, sizeof(int));
		m_Bytes += sizeof(int);
		m_StreamPos += sizeof(int);
	}

	/*
	* Reads byte data from the byte stream as a float and
	* advances the stream's position by the bytes read.
	*
	* Should return a float.
	*/
	float ByteStream::readFloat()
	{
		assert(m_StreamPos + sizeof(float) <= m_Bytes && "Buffer doesn't contain enough bytes to form a float.");

		float byteToFloat = 0;
		memcpy(&byteToFloat, (char*)(m_Buffer + m_StreamPos), sizeof(float));
		m_StreamPos += sizeof(float);
		return byteToFloat;
	}

	/*
	* Writes the given float to the byte stream as its
	* corresponding byte data and advances the stream's
	* byte position.
	*/
	void ByteStream::writeFloat(float writeFloat)
	{
		assert(m_StreamPos + sizeof(float) < MAX_STREAM_BYTES && "Buffer doesn't contain enough space to add a float.");

		memcpy((char*)(m_Buffer + m_StreamPos), &writeFloat, sizeof(float));
		m_Bytes += sizeof(float);
		m_StreamPos += sizeof(float);
	}

	/*
	* Reads in however many bytes from the buffer that were
	* specified to be read by the length prefixed string into
	* the given output buffer.
	*/
	int ByteStream::readString(void* outBuff, size_t outBuffSize)
	{
		int bytesToRead = readInt();
		assert(bytesToRead < outBuffSize && "Given buffer doesn't contain enough space to store this string.");
		assert(m_StreamPos + bytesToRead <= m_Bytes && "Stream buffer doesn't contain enough bytes to form a string of the specified byte count.");

		memcpy((char*)outBuff, (char*)(m_Buffer + m_StreamPos), bytesToRead);
		m_StreamPos += bytesToRead;
		return bytesToRead;
	}

	/*
	* Writes however many bytes have been specified from the
	* given input buffer to the byte stream, advancing the
	* stream's position in the process.
	*/
	void ByteStream::writeString(void* inBuff, size_t bytesToWrite)
	{
		writeInt((int)bytesToWrite);
		assert(m_StreamPos + bytesToWrite < MAX_STREAM_BYTES && "Buffer doesn't contain enough space to add the specified byte count.");

		memcpy((char*)(m_Buffer + m_StreamPos), (char*)inBuff, bytesToWrite);
		m_Bytes += bytesToWrite;
		m_StreamPos += bytesToWrite;
	}

	/*
	* Reads however many bytes specified of an unknown type from
	* the byte stream to an output location of an unspecified type.
	*/
	void ByteStream::readBytes(void* outBuff, size_t bytesToRead)
	{
		assert(m_StreamPos + bytesToRead <= m_Bytes && "Buffer doesn't contain enough bytes to read the specified byte count.");

		memcpy(outBuff, m_Buffer + m_StreamPos, bytesToRead);
		m_StreamPos += bytesToRead;
	}

	/*
	* Writes however many bytes specified of an unknown type to
	* the byte stream from an input location of an unspecified type.
	*/
	void ByteStream::writeBytes(void* inBuff, size_t bytesToWrite)
	{
		assert(m_StreamPos + bytesToWrite <= MAX_STREAM_BYTES && "Buffer doesn't contain enough space to write the specified byte count.");

		memcpy((char*)(m_Buffer + m_StreamPos), inBuff, bytesToWrite);
		m_Bytes += bytesToWrite;
		m_StreamPos += bytesToWrite;
	}
}