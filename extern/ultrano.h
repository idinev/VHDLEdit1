#pragma once

#define ULTRANO_INCLUDED

//#define ULTRANO_DEBUG_MEMORY // decomment to debug memory


#pragma warning(disable: 4244 4305) // warning truncation  float<->double


extern void * _cdecl operator new (unsigned int size); // size_t
extern void * operator new[] (unsigned int size);
extern void _cdecl operator delete (void * ptr);
extern void operator delete[] (void * ptr);

//------[ custom types ]---------------[
typedef signed char		S8;
typedef signed short	S16;
typedef signed int		S32; // 4 bytes
typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32; // 4 bytes

#ifdef __GNUC__
#define S64 signed long long
#define U64 unsigned long long
#else
#define S64 signed __int64
#define U64 unsigned __int64
#endif

#define true 1
#define false 0
//typedef int bool;//#define bool int // <--- !! Note


#define null 0

typedef void (* PPROC)();
typedef void (* PPROC1)(int);
typedef void (* PPROC2)(int,int);
typedef void (* PPROCV2)(void*,int,int);
//-------------------------------------/


//------[ macros ]------------------------[
#define print(x) DebugPrint(#x,x)
#define prints(x) DebugPrint(x)
#define printID(x) DebugPrintID(#x,x)
#define printh(x) DebugPrintHex(#x,(int)x)
#define printi(x) DebugPrint(#x,(int)x)
#define PrintLine DebugPrintLine()
#define printOnce(x) {static bool yep=false; if(!yep){yep=true;print(x);}}
#define printArray(x,asize) {DebugPrint(#x,"{"); for(int _iii=0;_iii<asize;_iii++){trace("	[%d]",_iii);DebugPrint("	",x[_iii]);}DebugPrint("}");}

#define GET_OFFSET( struct_type, member ) ((int)&((struct_type*)0)->member) // returns offset of a member within a struct

#define resize(What,Type,OldCount,NewCount)  What=(Type*)memresize(What,sizeof(Type),OldCount,NewCount)

#define foreach(_ObjVector,_Type0)			for(_Type0* EDXPTR=(_Type0*)(_ObjVector)->data;_Type0 edx = *EDXPTR++;)

#define USES_GARBAGE Garbage MyGarbage
#define gnew(Type,Count) (Type*)MyGarbage.alloc((Count)*sizeof(Type))

#define CRTL_INITIALIZER(Target,Data,Index) static CRtlInitializer CRTL_INITIALIZER_HELPER1(unnamedcrtinit,__LINE__) =CRtlInitializer((void**)Target,sizeof(Target),(void*)&Data,Index);
#define CRTL_INITIALIZER2(Target,Data)	static CRtlInitializer CRTL_INITIALIZER_HELPER1(unnamedcrtinit,__LINE__) =CRtlInitializer((void**)Target,Target##_Count,(void*)&Data);
	#define CRTL_INITIALIZER_HELPER2(x,y) x##y // used to trick macros... (do not use!!)
	#define CRTL_INITIALIZER_HELPER1(x,y) CRTL_INITIALIZER_HELPER2(x,y) // used to trick macros... (do not use!!)

#define serializable		virtual void Serialize(int arg); public: int _serialInfo
#define init_serializable	Serialize(0)


#define loop(var,count) for(int var=0;var<count;var++)
#define loopi(count) for(int i=0;i<count;i++)
#define CLAMP(var,vmin,vmax) {if((var)<(vmin))var=(vmin); else if((var)>(vmax))var=(vmax);}

#define arraysize(array) (sizeof(array)/sizeof(array[0]))
#define CALLOC(count,type)  (type*)xmalloc((count) * sizeof(type))
#define CREALLOC(data,count,type)  (type*)xresize((data),(count) * sizeof(type))
#if !defined(RELEASE) //#if defined(_DEBUG) || defined(DEBUG)
#define ASSERT(what) if(!(what)){trace("assert failed on %s : %d :  %s",__FILE__,__LINE__,#what); BenchBreakpoint();}
#else
#define ASSERT(what)
#endif

//----------------------------------------/






//=======================[ SystemMisc ]=============================================================================[
//-------------------------------------[ strings ]----------------------------------[
int   ustrlen(const char* x);  // returns length of string
char* ustrcpy(char *dest,const char *src); // copies a string
char* ustrcat(char *dest,const char *src); // concatenates a string

int	  ustrcmp(const char* s1,const char* s2); // compares CASE-SENSITIVELY a string
char* ustrchr(const char* s1,char c); // finds a specific character in a string
char* ustrchrLast(const char* s1,char c); // finds the last specific character in a string
char* uinstring(const char* data,const char* pattern); // finds the first specific substring
bool  ustrStartsWith(const char* data,const char* prefix);
bool  ustrEndsWith(const char* data,const char* suffix);

int   uatoi(char** ppString); // read integer from char*, update char* to point to after this int
int   uatoh(char** ppString); // read HEX integer from char*, update char* to point to after this int
float uatof(char** ppString); // read float from char*, update char* to point to after this int

char* uitoa(char* dest,int x); // converts integer to string
char* uftoa(char* dest,float x); // converts float to string
char* uitoaHex(char* dest,int x); // converts integer to hex-string
char* uitoaRank(char* dest,int x); // 1st, 2nd, 3rd, 4th, 11th, 
char* ui64toa(char* dest,U64 x); // converts 64-bit integer to string
char* usprintf(char* dest,const char* format, ...); // tokens are: %d, %f, %F, %X, %r, %s, %%

char* ustrclone(const char* x); // allocates a new string via xmalloc() !
char* ustrmix(const char* src1,const char* src2); // allocates a new string via xmalloc() !
char* ustrmix(const char* src1,const char* src2,const char* src3); // allocates a new string via xmalloc() !
char* ustrmix(const char* src1,const char* src2,const char* src3,const char* src4); // allocates a new string via xmalloc() !

int  ustrRemoveChars(char* data,char c);
int  ustrCountChars(const char* data,char c);
int  ustrReplaceChars(char* data,char c,char replacement);
void ustrTrimWhiteSpace(char* data);
int  ustrRemoveSubStrings(char* data,const char* pattern);

char* ustrSplitLines(char* data,int* pNumLines); // overwrites "data": removes c=10, and replaces c=13 with 0. Returns ptr to first line.
char** ustrSplitLinesToArray(char* data,int* pNumLines); // allocates a char*[] array via xmalloc!! You have to xfree() that later. Overwrites "data"
char* ustrPathFromFileName(const char* FileName,const char* AppendStuff=null); // returns null ONLY if both arguments are null ! Otherwise can allocate even a 0-character string (taking-up 1 byte)
//-----------------------------------------------------------------------------------/

//------------------[ files ]-------------------------------[
char* uffetch(const char* fName,int *fSize=null);
bool  ufdump(const char* fName,const void* data,int dataSize);

bool ufopenRead(const char* FileName);
bool ufopenReadW(const wchar_t* FileName);
bool ufopenWrite(const char* FileName);
bool ufopenWriteW(const wchar_t* FileName);
void ufclose();

int  ufsize();
void ufskip(int numBytes);
int  ufseek(int absoluteAddr);
int  uftell();
bool ufIsEndOfFile();

bool ufread(void* data,int dataSize);
char  ufread1();
short ufread2();
int   ufread4();
float ufread4f();
U32   ufreadh();
void* ufreadAlloc(int dataSize); // returns null if dataSize<=0 . Otherwise allocates chunk with xmalloc()
char* ufreadstr(); // returns a xmalloc() chunk or null

bool ufwrite(const void* data,int dataSize);
void ufwrite1(char x);
void ufwrite2(short x);
void ufwrite4(int x);
void ufwrite4f(float x);
void ufwritestr(const char* string);

U64  ufGetDate(const char* fName);
void ufSetDate(const char* fName,U64 date);
//----------------------------------------------------------/
//----------[ memory ]------------------------------------------------------------------------------------[
void* xmalloc(int size);
void  xfree(void* what);
void* xresize(void* ptr,int newsize);
int   xmemsize(const void* ptr);
void  memmove(void* dest,const void* src,int size);
void  memmoveDword(void* dest,const void* src,int numDwords);
void* memclone(const void* ptr,int size);
void  memclear(void* ptr,int size);
U32   memCRC32(const void* ptr,int len,U32 crc=0);
void* memAppendLg(void* ptr,int& usedSize,int& curSize,const void* dataToAppend,int dataToAppendSize);
void  OnExit_MemCheck();
void __xMEMORY_PUSHHEAP();
void __xMEMORY_POPHEAP();
int  __xMEMORY_MARK_NON_LEAKY_XMALLOCS(bool bMarkAsNonLeaky=true); // returns count of marked objects
void __xMEMORY_BREAK_ON_AGE(int XM_Object_Age);
//--------------------------------------------------------------------------------------------------------/
//---------[ debug ]---------------------------[
void DebugClear();
void DebugHide();
void DebugPrint(const char* DebugData);
void DebugPrint(const char* s1,const char* s2);
void DebugPrint(const char* s1,int x);
void DebugPrint(const char* s1,U32 x);
void DebugPrint(const char* s1,U64 x);
void DebugPrint(const char* s1,void* ptr);
void DebugPrint(const char* s1,float x);
void DebugPrint(const char* s1,double x);
void DebugPrintID(const char* s1,int x);
void DebugPrintHex(const char* s1,int x);
void DebugPrintLine();
void trace(const char* format, ...);
//---------------------------------------------/

//-----[ benchmark ]------------[
void BenchStart();
U32  BenchEnd(const char* name);
void BenchPrint();
void BenchClear();
void BenchBreakpoint();
//------------------------------/
//-----[ sys-exec ]---------------------------------------------[
void uSysExecute(const char* pExeName,const char* params=null);
char* uSysExecute2(const char* pExeName,const char* params=null); // you must xfree() the result!
void uexit(const char* Error=null);
void uexit(const char* format,const char* text1);
//--------------------------------------------------------------/

//-----[ sorting ]-------------------------------------[
void QSortA(int* Arr,int count);
void QSortB(void* Arr,int count,const void* CompareFunc); // sort a void*[count] array.
//-----------------------------------------------------/



//-------[ floating-point ]---------------------------------[
void SSE_DenormalsAreZero_FlushToZero();

float usin_fastLo(float x);
float ucos_fastLo(float x);
float usin_fastHi(float x);
//void __stdcall uMultMatrix4_byMatrix4(__m128* pOut,const __m128* in1,const __m128* in2);
//----------------------------------------------------------/
//==================================================================================================================/







//==============================================[ structures ]======================================================[
typedef struct{
	int int0;
	PPROC proc;
}CallBackInt;


class ObjVector{
public:
	void** data; // Array of pointers to each inserted element. Zero-terminated
	int num; // Count of inserted elements

	ObjVector();
	ObjVector(int initialSize); // Creates a vector with given initial capacity (in number of elements). Use powers of two if possible. 
	~ObjVector(); // Deletes the data[] array, but leaves all elements intact. As we don't know whether they're ints, structs, HWND handles or whatever
	void add(void* d); // Inserts 
	void addAtIndex(void* d,int index);
	void addToStart(void* d); // Inserts at beginning of array
	void addMultiple(void** pd,int count);
	void addAllFrom(ObjVector* o);
	int  remove(void* d); // Compares by address or value of element
	void remove(int index); // Removes element at given index. It's OK to give aray-out-of-range indices.
	void removeQuickly(int index); // swaps the last element with this index
	int  contains(void* d); // returns 0 if not contained here, or 1+index otherwise
	void sort(void* CompareFunc){QSortB(data,num,CompareFunc);}
	void slimDown();
private:
	int takenSlots; // = size of data[], in dwords. Counts the zero-terminating dword
};



class BakedVector{
public:
	int num;
	void* data[1]; // zero-terminated!!
	// ------> end of data <-------

	inline int getTotalSize(){
		return (num+2)*4;
	}

	static BakedVector* New(int size){
		BakedVector* vec = (BakedVector*)xmalloc((size+2)*4);
		vec->num = size;
		return vec;
	}
	static void Delete(BakedVector* vec){
		xfree(vec);
	}


	static BakedVector* FromObjVector(ObjVector* o);
	ObjVector* ToObjVector();


	BakedVector* add(void* d); // returns a new pointer to this baked vector !!
	BakedVector* remove(void* d);
	BakedVector* remove(int index);
	int  contains(void* d);

private:
	BakedVector(){	prints("uhoh, BakedVector constructor must never run!");	}
	~BakedVector(){	prints("uhoh, BakedVector destructor must never run!");		}
};

class Garbage{
public:
	Garbage();
	~Garbage();
	void* alloc(int size); // Allocated data is zeroed
	void  free(void* ptr); 
	int   getSize(void* ptr); // Returns valid data only on alloc()-returned vars. Pointers, added via addGarbage() return 0 here. 
	void  addGarbage(void* ptr); // Registers an xmalloc()-allocated pointer to be destroyed together with this garbage-bin.
	char* addString(const char* str); // Clones this string!
private:
	int    NumObjects;
	void **pObjects;
	int   *pSizes;
};
class CRtlInitializer{
public:
	CRtlInitializer(void** target,int targetsize,void* data,int index);
	CRtlInitializer(void** target,int& count,void* data);
private:
	char dummy;
};

class VFile{
public:
	VFile();
	~VFile();
	
	char*  m_data;
	int size;
	void  write(const void* data,int dataSize);
	void  writeline(const char* text);
	void* stealData(); // Compacts memory, and provides it as a result. Resets the VFile completely, you are now to xfree() this result!
	char* stealString();// as above, but puts a terminating zero at end.
private:
	
	int taken;	
};


class FixedAlloc{
public:
	FixedAlloc();
	FixedAlloc(int NumMaxElements,int ElementSize);
	void  init(int NumMaxElements,int ElementSize);
	~FixedAlloc();
	
	void* alloc(); // can return 0
	void  free(void* ptr);

private:
	void** z_all_free;
	int z_NumFree;
};

class FixedAlloc16{ // all allocations are 16-byte aligned
public:
	FixedAlloc16();
	FixedAlloc16(int NumMaxElements,int ElementSize);
	void  init(int NumMaxElements,int ElementSize);
	~FixedAlloc16();
	
	void* alloc(); // can return 0
	void  free(void* ptr);

private:
	void** z_all_free;
	int z_NumFree;
};


class CHashTable{
public:
	int numElements;
	int tableSize; //=64 by default
	void (*ValueDeleter)(void* pdata); // callback to delete the value. If it's not set, then xfree() is used on it!

	ObjVector **table; // array of ObjVector[tableSize]
	
	void add(const char* name,void* value);
	void remove(const char* name);
	void* find(const char* name);

	CHashTable(int tabSizeLog2=6);
	~CHashTable();
};

class CUniqueArray{ // each element is the same size
public:
	CUniqueArray(int ElementSize); // size of each element
	~CUniqueArray();

	int add(const void* elementData); // returns the unique index
	void* stealData();

	

	char* data;
	int numElements;
private:
	int elementSize;
	int bytesAllocated;
	
	struct{
		int* indices;
		U32* crcs;
		int count;
		int bytesTaken;
	}hashes[256];
};


class CTextReader{
public:
	CTextReader();
	~CTextReader();

	bool LoadFile(const char* FileName);

	char ReadChar();
	void GoBack(int count = 1);
	bool ExpectChar(char c);
	bool ExpectString(const char* str);
	bool ExpectCharAfterBlanksAndCRLF(char c);
	bool StartsWith(const char* string);
	bool StartsWithI(const char* string);
	int  ReadInt();
	float ReadFloat();
	int  ReadKeyword(char* outString,int bufSize);
	int  ReadStringWord(char* outString,int bufSize);
	void SkipWhiteSpace();
	void SkipWhiteSpaceAndCRLF();
	bool SkipToAfter(char c);

	char* ReadLine(); // returns current position,  while advances p to next line, and overwrites the newline with a 0

	char* p;
	char* OriginalData;
};
//==================================================================================================================/

