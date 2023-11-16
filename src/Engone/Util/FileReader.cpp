#include "Engone/Util/FileReader.h"

namespace engone {
	FileReader::FileReader(const std::string& path, bool binaryForm) : binaryForm(binaryForm) {
		m_file = engone::FileOpen(path.c_str(),&m_fileSize,engone::FILE_ONLY_READ);
		if (!m_file) {
			m_error = FileNotFound;
			return;
		}
		bool yes = m_buffer.resize(BYTES_PER_READ);
		if (!yes) {
			m_error = AllocFailure;
			return;
		}
		m_path = path;
		u64 bytes = engone::FileRead(m_file, m_buffer.data, BYTES_PER_READ);
		m_buffer.used=bytes;
		
		if(bytes==m_fileSize){
			close();
		}
	}
	FileReader::~FileReader() {
		cleanup();
	}
	void FileReader::close() {
		engone::FileClose(m_file);
		m_file = {};
	}
	void FileReader::cleanup() {
		m_path.clear();
		close();
		m_fileSize = 0;
		m_fileHead = 0;	
		m_error = NoError;
		m_bufferHead = 0;
		m_buffer.resize(0);
	}
	bool FileReader::read(void* ptr, u64 bytes){
		Assert(ptr);
		u64 bytesInBuffer = m_buffer.used - m_bufferHead;
		if(bytesInBuffer >= bytes){
			// Bytes exist in buffer, read them
			memcpy(ptr,m_buffer.data,bytes);
			m_bufferHead += bytes;
			return true;
		}
		
		u64 bytesInFile = m_fileSize - m_fileHead;
		if(bytesInBuffer+bytesInFile < bytes || !m_file){
			m_error = EndOfFile;
			return false; // not enough bytes in buffer and file
		}
		
		memcpy(ptr,m_buffer.data,bytesInBuffer);
		char* outPtr = (char*)ptr+bytesInBuffer;
		m_bufferHead = 0;
		
		u64 bytesLeft = bytes-bytesInBuffer;
		while(true){
			u64 readBytes = FileRead(m_file,m_buffer.data,BYTES_PER_READ);
			if(readBytes==-1){
				// there should be bytes left, we checked m_fileSize, something else is going on
				m_error = DataWasLost;
				Assert(("FileReader : DataWasLost",false));
				return false;
			}
			m_fileHead += readBytes;
			m_buffer.used = readBytes;
			
			if(readBytes>=bytesLeft){
				// we done
				memcpy(outPtr,m_buffer.data,bytesLeft);
				m_bufferHead += bytesLeft;
				return true;
			} else if(readBytes==BYTES_PER_READ){
				// cool but we need more	
				memcpy(outPtr,m_buffer.data,BYTES_PER_READ);
				outPtr+=BYTES_PER_READ;
			} else {
				// no more ):
				m_error = DataWasLost;
				return false;
			}
		}
		return false;
	}
		
	u64 FileReader::readLine(std::string& line) {
		line.clear();
		bool isComment=false;
		while(true) {
			int bytesLeft = m_buffer.used-m_bufferHead;
			Assert(bytesLeft>=0);
			
			if(bytesLeft==0){
				u64 readBytes = 0;
				if(m_file)
					readBytes = FileRead(m_file,m_buffer.data,BYTES_PER_READ);
					
				if(readBytes==-1){
					// Todo: Use a different error when m_file == 0?
					m_error = EndOfFile;
					return line.length()!=0;
				}	
				m_fileHead += readBytes;
				m_buffer.used = readBytes;
				m_bufferHead = 0;
				if(readBytes==0){
					m_error = EndOfFile;
					return line.length()!=0; // no more bytes to read, end of line
				}
			}
			char chr = *((char*)m_buffer.data+m_bufferHead);
			
			if(line.empty()&&chr=='#')
				isComment=true;
			if(chr=='\n'){
				m_bufferHead++;
				if(!isComment){
					return true;
				}
				isComment=false;
				continue;
			}
			if(chr=='\r'){
				m_bufferHead++;
				continue;
			}
			if(!isComment)
				line += chr;
			m_bufferHead++;
		}
	}
	bool FileReader::readAll(std::string& lines){
		lines.clear();
		bool wasEmpty=false;
		std::string temp;
		while(true){
			temp.clear();
			bool yes = readLine(temp);
			if(!yes)
				break;
			if(wasEmpty)
				lines+="\n";
			
			wasEmpty=false;
			if(!temp.empty()){
				lines+=temp+"\n";
			}else{
				wasEmpty=true;
			}
		}
		return lines.size()!=0;
	}
	u64 FileReader::readNumbers(char* ptr, u64 count, u32 typeSize, bool isFloat) {
		Assert(ptr);
		if (binaryForm) {
			return read(ptr, typeSize * count);
		}
		
		int numberCount=0;
		std::string number;
		bool isComment=false;
		while(numberCount!=count){
			int bytesLeft = m_buffer.used-m_bufferHead;
			Assert(bytesLeft>=0);
			
			bool lastNumber=false;
			bool endNumber=false;
			if(bytesLeft==0){
				u64 readBytes = 0;
				if(m_file)
					readBytes = FileRead(m_file,m_buffer.data,BYTES_PER_READ);
					
				if(readBytes==-1){
					// Todo: Use a different error when m_file == 0?
					m_error = EndOfFile;
					return false;
				}	
				m_buffer.used = readBytes;
				m_bufferHead=0;
				m_fileHead += readBytes;
				if(readBytes==0){
					lastNumber=true;
					endNumber=true;
					m_error = EndOfFile;
				}
			}
			if(!lastNumber){
				char chr = *((char*)m_buffer.data+m_bufferHead);
				m_bufferHead++;
				if(chr=='\r'){ // just skip
					continue;
				}
				if(chr=='\n' || chr==' '){
					endNumber=true;
					if(isComment&&chr=='\n') {
						number.clear();
					}
				}else{
					number+=chr;
					if(number[0]=='#')
						isComment=true;
					continue;
				}
			}
			
			if(endNumber&&!number.empty()&&!isComment){
				numberCount++;
				double num = atof(number.c_str());
			
				if(isFloat){
					if(typeSize==sizeof(float)){*((float*)ptr) = num;ptr += typeSize;}
					else if(typeSize==sizeof(double)){*((double*)ptr) = num;ptr += typeSize;}
				}else{
					if(typeSize==sizeof(i8)){*((i8*)ptr) = num;ptr += typeSize;}
					else if(typeSize==sizeof(i16)){*((i16*)ptr) = num;ptr += typeSize;}
					else if(typeSize==sizeof(i32)){*((i32*)ptr) = num;ptr += typeSize;}
					else if(typeSize==sizeof(i64)){*((i64*)ptr) = num;ptr += typeSize;}
				}
				number.clear();
			}
			if(number.empty())
				isComment=false;
			if(lastNumber)
				break;
		}
		return numberCount;
	}
	bool FileReader::read(std::string* ptr, u64 count){
		if(binaryForm){
			u16 length;
			bool yes = read(&length);
			if(!yes){
				return false;
			}
			ptr->resize(length);
			yes = read(&(*ptr)[0], length);
			if(!yes){
				return false;
			}
		}else{
			u64 length = readLine(*ptr);
			return length!=0;
		}
		return true;
	}
		
	#define GEN_READ_TYPE(TYPE,FLOATY)\
	bool FileReader::read(TYPE* ptr, u64 count){\
		return count==readNumbers((char*)ptr,count,sizeof(TYPE),FLOATY);}\
		
	GEN_READ_TYPE(u8,false)
	GEN_READ_TYPE(i16,false)
	GEN_READ_TYPE(u16,false)
	GEN_READ_TYPE(i32,false)
	GEN_READ_TYPE(u32,false)
	GEN_READ_TYPE(i64,false)
	GEN_READ_TYPE(u64,false)
	GEN_READ_TYPE(bool,false)
	GEN_READ_TYPE(float,true)
	GEN_READ_TYPE(double,true)

	// bool FileReader::read(glm::vec3* ptr, u64 count){
	// 	return count==readNumbers((char*)ptr,count*3,sizeof(float),true);
	// }
	// bool FileReader::read(glm::mat4* ptr, u64 count){
	// 	return count==readNumbers((char*)ptr,count*16,sizeof(float),true);
	// }
}