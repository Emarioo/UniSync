#pragma once

//#define ENUM_OP(NAME,OP) NAME operator OP(NAME a,NAME b){return (NAME)((int)a OP(int)b);}
//#define ENUM_BEG(NAME,...) enum NAME : int{ __VA_ARGS__ };ENUM_OP(NAME,|)ENUM_OP(NAME,&)ENUM_OP(NAME,^) const char* to_string(NAME value){switch(value){
//#define ENUM_CASE(x) case x: return #x;
//#define ENUM_END() }return "Unknown";}
//
//#define GenEnum(NAME,...) enum NAME : int{ __VA_ARGS__ }; static const char* strings##NAME[]{#__VA_ARGS__}; const char* to_string(NAME value){return strings##NAME[(int)value];}


#define GenEnum(NAME,...) enum NAME : int{ __VA_ARGS__ }; const char* to_string(NAME val);

#define GenEnumImpl(NAME,...)  const char* to_string(NAME val){}

//#define GenEnum(...) __VA_ARGS__

char** gen_list(const char* list, char*& sample) {
	int len = strlen(list) + 1;
	if (len < 2) return nullptr;

	sample = new char[len];
	strcpy(sample, list);

	int count = 1;
	for (int i = 0; i < len; i++) {
		if (sample[i] == ',') {
			count++;
		}
	}
	char** out = new char*[count];
	for (int i = 0; i < len; i++) {
		if (sample[i] == ',') {
			sample[i] = 0;
			count++;
		}
	}
	return out;
}

GenEnum(Test, Blue = 0, Red = 1, Green = 2)

enum a {
	rad = 28,
	klad=24,
	kra=23,
	ea,
};

void test() {
	static a ids[]{ rad = 28};
	std::unordered_map<int, const char*> map;

	const char* src = "rad=25,klad,kra=27";
	char* copy = new char[strlen(src)+1];
	strcpy(copy, src);

	int index = 0;
	int id;

	bool reachedEnd = false;
	char* name=nullptr;
	char* num=nullptr;
	while(true) {
		char chr = copy[index];
		if (chr == 0) break;

		if (chr == ',') {

		}
		if (chr == '=') {
			num = copy + index;
		}
		if(!name)
			name = copy + index;

		index++;
	}

	to_string(Blue);
}

static char* sample;
static char** list=gen_list("args,aeras",sample);

//const char* to_string(int index) {
//	return list[index];
//}


//inline constexpr const int list_length(const char* list, int index = 0) {
//	int len = 1;
//	int index = 0;
//	while (true) {
//		if (list[index] == ',') len++;
//		if (list[index] == 0) break;
//		index++;
//	}
//
//	//if (list[index] == ',') return list_length(list + 1,index+1) + 1;
//	//if (list[index] == 0) return 0;
//	//return list_length(list + 1,index+1);
//}