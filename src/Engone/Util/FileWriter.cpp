#include "Engone/Util/FileWriter.h"

namespace engone {
	FileWriter::FileWriter(const std::string& path, bool binaryForm) : binaryForm(binaryForm) {
		m_file = engone::FileOpen(path.c_str(),nullptr,engone::FILE_ALWAYS_CREATE);
		if (!m_file) {
			m_error = FileNotFound;
			return;
		}
		m_path = path;
	}
	FileWriter::~FileWriter() {
		cleanup();
	}
	void FileWriter::close() {
		engone::FileClose(m_file);
		m_file = {};
	}
	void FileWriter::cleanup() {
		m_path.clear();
		close();
		m_fileHead = 0;	
		m_error = NoError;
	}
	bool FileWriter::write(const void* ptr, u64 bytes){
		Assert(ptr);

        u64 writtenBytes = FileWrite(m_file,(char*)ptr,bytes);
        if(writtenBytes!=bytes){
            m_error = OtherError;
            printf("FileWriter::write : bad error\n");
            return false;
        }
        return true;
    }
	u64 FileWriter::writeNumbers(const char* ptr, u64 count, u32 typeSize, bool isFloat) {
		Assert(ptr);
		if (binaryForm) {
			return write(ptr, typeSize * count);
		}

        // Todo: improve the code
        char buffer[256];
        for(int i=0;i<count;i++){
            if(isFloat){
                double num = 0;
                if(typeSize==sizeof(float)){num=*((float*)ptr); ptr += typeSize;}
                else if(typeSize==sizeof(double)){num=*((double*)ptr); ptr += typeSize;}
                sprintf(buffer,"%lf ",num);
            }else{
                i64 num = 0;
                if(typeSize==sizeof(i8)){num = *((i8*)ptr); ptr += typeSize;}
                else if(typeSize==sizeof(i16)){num = *((i16*)ptr); ptr += typeSize;}
                else if(typeSize==sizeof(i32)){num = *((i32*)ptr); ptr += typeSize;}
                else if(typeSize==sizeof(i64)){num = *((i64*)ptr); ptr += typeSize;}
                sprintf(buffer,"%lld ",num);
            }
            u64 bytesWritten = FileWrite(m_file, buffer, strlen(buffer));
            if(bytesWritten!=strlen(buffer)){
                m_error = OtherError;
                printf("Some error\n");
                return i;
            }
        }
        char buf='\n';
        // Todo: deal with error?
        FileWrite(m_file, &buf, 1);
        return count;
	}
	bool FileWriter::write(const std::string* ptr, u64 count){
		if(binaryForm){
			u16 length = ptr->size();
			bool yes = write(&length);
			if(!yes){
				return false;
			}
			yes = write(ptr->data(),length);
			if(!yes){
				return false;
			}
		}else{
			bool yes = write(ptr->data(),ptr->size());
			const char temp='\n';
			yes &= write(&temp,1);

			return yes;
		}
		return true;
	}
	#define GEN_WRITE_TYPE(TYPE,FLOATY)\
	bool FileWriter::write(const TYPE* ptr, u64 count){\
		return count==writeNumbers((const char*)ptr,count,sizeof(TYPE),FLOATY);}\
		
	GEN_WRITE_TYPE(u8,false)
	GEN_WRITE_TYPE(i16,false)
	GEN_WRITE_TYPE(u16,false)
	GEN_WRITE_TYPE(i32,false)
	GEN_WRITE_TYPE(u32,false)
	GEN_WRITE_TYPE(i64,false)
	GEN_WRITE_TYPE(u64,false)
	GEN_WRITE_TYPE(bool,false)
	GEN_WRITE_TYPE(float,true)
	GEN_WRITE_TYPE(double,true)
}