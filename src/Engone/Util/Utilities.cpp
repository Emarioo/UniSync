
#include "Engone/Util/Utilities.h"

#include "stb/stb_image.h"

#include "Engone/Win32Includes.h"
#include <thread>

namespace engone {
	// TrackerId ItemVector::trackerId = "ItemVector";

	u32 FormatBytes(char* outStr, u32 length, i64 number){
		const int KB = pow(2,10);
		const int MB = pow(2,20);
		const int GB = pow(2,30);
		if(number/KB<1)
			return snprintf(outStr,length,"%lld B",number);
		else if(number/MB<1)
			return snprintf(outStr,length,"%.2f KB",(float)number/KB);
		else if(number/GB<1)
			return snprintf(outStr,length,"%.2f MB",(float)number/MB);
		else
			return snprintf(outStr,length,"%.2f GB",(float)number/GB);
	}
	u32 FormatNumber(char* outStr, u32 length, i64 number){
		const i64 K = pow(10,3);
		const i64 M = pow(10,6);
		const i64 G = pow(10,9);
		
		if(number<K)
			return snprintf(outStr,length,"%lld",number);
		else if(number<M)
			return snprintf(outStr,length,"%.2f K",(float)number/K);
		else if(number<G)
			return snprintf(outStr,length,"%.2f M",(float)number/M);
		else
			return snprintf(outStr,length," %.2f G",(float)number/G);
	}

	std::vector<std::string> SplitString(std::string text, std::string delim) {
		std::vector<std::string> out;

		int lastAt = 0;
		while (true) {
			int at = text.find(delim, lastAt);
			if (at == -1) {
				break;
			}
			std::string push = text.substr(lastAt, at - lastAt);
			out.push_back(push);
			lastAt = at + 1;
		}
		if (lastAt != text.size() || lastAt == 0)
			out.push_back(text.substr(lastAt));


		//unsigned int at = 0;
		//while ((at = text.find(delim)) != std::string::npos) {
		//	std::string push = text.substr(0, at);
		//	//if (push.size() > 0) {
		//	out.push_back(push);
		//	//}
		//	text.erase(0, at + delim.length());
		//}
		//if (text.size() != 0)
		//	out.push_back(text);
		return out;
	}

	/*
	void Insert4(float* ar, int ind, float f0, float f1, float f2, float f3) {
		ar[ind] = f0;
		ar[ind + 1] = f1;
		ar[ind + 2] = f2;
		ar[ind + 3] = f3;
	}*/
	float lerp(float a, float b, float c) {
		return (1 - c) * a + c * b;
	}
	float inverseLerp(float min, float max, float x) {
		return (x - min) / (max - min);
	}
	float distance(float x, float y, float x1, float y1) {
		return (float)sqrt(pow(x1 - x, 2) + pow(y1 - y, 2));
	}
	// float AngleDifference(float a, float b) {
	// 	float d = a - b;
	// 	if (d > glm::pi<float>()) {
	// 		d -= glm::pi<float>() * 2;
	// 	}
	// 	if (d < -glm::pi<float>()) {
	// 		d += glm::pi<float>() * 2;
	// 	}
	// 	return d;
	// }
	/*
	void insert(float* ar, int off, int len, float* data) { // carefull with going overboard
		//std::cout << "INSERT ";
		for (int i = 0; i < len; i++) {
			ar[off + i] = data[i];
			//std::cout << off + i << "_" << ar[off + i] << "   ";
		}
		//std::cout << std::endl;
	}*/
	// returned value goes from 0 to 1. x goes from xStart to xEnd. Function does not handle errors like 1/0
	float bezier(float x, float xStart, float xEnd) {
		float t = (x - xStart) / (xEnd - xStart);
		float va = /*(pow(1 - t, 3) + 3 * pow(1 - t, 2) * t) +*/(3 * (1 - t) * (float)pow(t, 2) + (float)pow(t, 3));
		return va;
	}
	float Min(float a, float b) {
		if (a < b) return a;
		return b;
	}
	float Max(float a, float b) {
		if (a > b) return a;
		return b;
	}
	/*
	std::string Crypt(const std::string& word, const std::string& key, bool encrypt) {
		std::string out = "";
		for (int i = 0; i < word.length();i++) {
			int at = word[i];
			for (int j = 0; j < key.length();j++) {
				int val = (i - j) * key[j];
				if (encrypt) at += val;
				else at -= val;
			}
			out += (at % 256);
		}
		return out;
	}*/

	ItemVector::ItemVector(int size) {
		if (size != 0) {
			resize(size); // may fail but it's fine
		}
	}
	ItemVector::~ItemVector() {
		cleanup();
	}
	char ItemVector::readType() {
		if (m_writeIndex < m_readIndex + sizeof(char)) {
			return 0;
		}

		char type = *(m_data + m_readIndex);
		m_readIndex += sizeof(char);
		return type;
	}
	bool ItemVector::empty() const {
		return m_writeIndex == 0;
	}
	void ItemVector::clear() {
		m_writeIndex = 0;
		m_readIndex = 0;
	}
	void ItemVector::cleanup() {
		resize(0); // <- this will also set write and read to 0
	}
	bool ItemVector::resize(uint32_t size) {
		if (size == 0) {
			if (m_data) {
				Free(m_data, m_maxSize);
				// GetTracker().subMemory<ItemVector>(m_maxSize);
				m_data = nullptr;
				m_maxSize = 0;
				m_writeIndex = 0;
				m_readIndex = 0;
			}
		} else {
			if (!m_data) {
				char* newData = (char*)Allocate(size);
				if (!newData) {
					log::out << log::RED << "ItemVector failed allocation memory\n";
					return false;
				}
				// GetTracker().addMemory<ItemVector>(size);
				m_data = newData;
				m_maxSize = size;
			} else {
				char* newData = (char*)Reallocate(m_data, m_maxSize, size);
				if (!newData) {
					log::out << log::RED << "ItemVector failed reallocating memory\n";
					return false;
				}
				// GetTracker().subMemory<ItemVector>(m_maxSize);
				// GetTracker().addMemory<ItemVector>(size);
				m_data = newData;
				m_maxSize = size;
				if (m_writeIndex > m_maxSize)
					m_writeIndex = m_maxSize;
				if (m_readIndex > m_maxSize)
					m_readIndex = m_maxSize;
			}
		}
		return true;
	}
	/*
	void CountingTest(int times, int numElements, std::function<int()> func) {
		std::vector<int> occurrences(numElements);

		double bef = GetSystemTime();
		for (int i = 0; i < times; i++) {
			int index = func();
			if(index>-1&&index<numElements)
				occurrences[func()]++;
		}
		double aft = GetSystemTime();
		for (int i = 0; i < numElements; i++) {
			std::cout << i << " " << occurrences[i] << std::endl;
		}
		std::cout << "Time: " << (aft - bef) << std::endl;
	}*/

	// static std::unordered_map<int, DelayCode> delayers;
	// void DelayCode::Start(int id, float waitSeconds) {
	// 	delayers[id] = { waitSeconds };
	// }
	// void DelayCode::Stop(int id, float waitSeconds) {
	// 	auto find = delayers.find(id);
	// 	DelayCode* dc = nullptr;
	// 	if (find != delayers.end()) {
	// 		find->second.stop();
	// 	}
	// }
	// bool DelayCode::Run(int id, float deltaTime) {
	// 	auto find = delayers.find(id);
	// 	DelayCode* dc = nullptr;
	// 	if (find != delayers.end()) {
	// 		return find->second.run(deltaTime);
	// 	}
	// 	return false;
	// }
	// bool DelayCode::Run(int id, LoopInfo& info) {
	// 	return Run(id, info.timeStep);
	// }
	void ConvertWide(const std::string& in, std::wstring& out) {
		out.resize(in.length(), 0);
		for (int i = 0; i < in.length(); i++) {
			*(&out[i]) = in[i];
		}
	}
	void ConvertWide(const std::wstring& in, std::string& out) {
		out.resize(in.length(), 0);
		for (int i = 0; i < in.length(); i++) {
			*(&out[i]) = in[i];
		}
	}
}