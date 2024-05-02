#pragma once

namespace Funny
{
	const int MAX_STREAM_BYTES = 4096;

	class ByteStream
	{
	public:
		void setBuffer(void* inBuff, size_t inBytes);
		void getBuffer(void* outBuff, size_t outBuffSize, size_t& outBytes);

		bool endOfStream() { return m_StreamPos == m_Bytes; }

		int readInt();
		void writeInt(int writeInt);

		float readFloat();
		void writeFloat(float writeFloat);

		int readString(void* outBuff, size_t outBuffSize);
		void writeString(void* inBuff, size_t bytesToWrite);

		// These are probably exceedingly unsafe, but I
		// think they should be fine in the context of this
		// assignment not posing security risks.
		void readBytes(void* outBuff, size_t bytesToRead);
		void writeBytes(void* inBuff, size_t bytesToWrite);

	private:
		// Manage stream data
		char m_Buffer[4096];
		size_t m_Bytes = 0;

		// Positon in buffer, incremented via read/write
		size_t m_StreamPos = 0;
	};
}