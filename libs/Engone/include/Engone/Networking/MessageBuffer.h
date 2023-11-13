#pragma once

//#include <stdint.h>
//#include <string>
//#include <iostream>

namespace engone {
	 //A message consists of the size of the body and then the body which is raw data.
	 //4.29 Gigabytes is the maximum of how many bytes you can send.
	 //Send multiple messages if you need to send more data.
	// Be careful when using copy constructor.
	class MessageBuffer {
	public:
		// msgType can be an enum which describes what type of message you are sending.
		// size is optional, if you know the size of the body this will would prevent
		// unnecessary reallocations of the internal buffer.

		// the reason i don't use this is because you need to manually delete the pointer unless you pass it to a send function.
		//static MessageBuffer* Create(uint32_t size=0);

		MessageBuffer() = default;
		MessageBuffer(uint32_t size);
		~MessageBuffer() { cleanup(); }

		MessageBuffer(const MessageBuffer&);
		//MessageBuffer operator=(const MessageBuffer&);

		// size of body
		uint32_t size() const {
			if (m_data) {
				return *((uint32_t*)m_data);
			}
			return 0;
		}

		// count will be pushed. You should then write data into the returned pointer.
		char* pushBuffer(uint32_t count);
		template<class T>
		void push(T* in, int count = 1) {
			if (m_dataSize < sizeof(uint32_t)+ size() + sizeof(T) * count) {
				if (!resize((m_dataSize + sizeof(T) * count) * 2)) {
					// resize will give error
					return;
				}
			}
			uint32_t head = sizeof(uint32_t)+size();
			std::memcpy(m_data + head, in, sizeof(T) * count);
			*((uint32_t*)m_data) += sizeof(T) * count;
		}
		// Careful with this. "push(vector.size())" is not uint32_t. It is size_t which is 8 bytes and not 4 bytes on 64-bit application.
		template<class T>
		void push(const T in) {
			if (m_dataSize < sizeof(uint32_t)+size() + sizeof(T)) {
				if (!resize((m_dataSize + sizeof(T)) * 2)) {
					// resize will give error
					return;
				}
			}
			uint32_t head = sizeof(uint32_t)+ size();
			std::memcpy(m_data + head, &in, sizeof(T));
			//log::out << "Pushed " << in<<" . "<< *(T*)(m_data + head) << "\n";
			*((uint32_t*)m_data) += sizeof(T);
		}

		void push(const std::string& in);
		void push(const char* in);

		// size of buffer will be written into pointer
		char* pullBuffer(uint32_t* count);
		template<class T>
		void pull(T* out, uint32_t count = 1) {
			if (sizeof(T) * count > size() - m_readHead) {
				// fishy
				log::out << log::YELLOW << "MessageBuffer::pull - Corrupted, did you pull string? (you are supposed to use a different function if so)\n";
				return;
			}
			std::memcpy(out, m_data + sizeof(uint32_t)+ m_readHead, sizeof(T) * count);
			m_readHead += sizeof(T) * count;
		}
		void pull(std::string& out, uint32_t max=-1);

		void moveReadHead(uint32_t byteIndex) {
			m_readHead = byteIndex;
		}

		// free the pointer like normal
		// returns nullptr if allocation failed.
		MessageBuffer* copy();

		void flush();

		char* getData() { return m_data; }
		uint32_t getMaxSize() const { return m_dataSize; }

		void cleanup();

		// The internal buffer is grouped with MessageBuffer
		//static TrackerId trackerId;
	private:
		char* m_data = nullptr;
		uint32_t m_dataSize = 0; // including size of header
		uint32_t m_readHead=0;
		bool m_sharing = false; // used when receiving

		// size should exclude size of header
		bool resize(uint32_t size);

		friend class Connection;
		friend class Client;
		friend class Server;
	};
}