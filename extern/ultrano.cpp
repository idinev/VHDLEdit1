#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "Ultrano.h"

static const char* VKDebug_Paths[]={
	"E:\\masm32\\bin\\dbgwin.exe",
	"D:\\masm32\\bin\\dbgwin.exe",
	"\\masm32\\bin\\dbgwin.exe",
	"dbgwin.exe",
	0
};

//========[ overload allocation ]===============[

#ifndef ULTRANO_DEBUG_MEMORY

void * _cdecl operator new (size_t size){
	return xmalloc((int)size);
}
void * operator new[] (size_t size){
	return xmalloc((int)size);
}
void _cdecl operator delete (void * ptr){
	xfree(ptr);
}
void operator delete[] (void * ptr){
	xfree(ptr);
}
#else

__declspec(naked) void * _cdecl operator new (size_t size){
	__asm jmp xmalloc;
}
__declspec(naked) void * operator new[] (size_t size){
	__asm jmp xmalloc;
}
__declspec(naked) void _cdecl operator delete (void * ptr){
	__asm jmp xfree;
}
__declspec(naked) void operator delete[] (void * ptr){
	__asm jmp xfree;
}
#endif
//==============================================/




//============[ strings ]=================================================[
int ustrlen(const char* x){
	int len=0;
	if(x){	while(x[len])len++; }
	return len;
}
char* ustrcpy(char *dest,const char *src){
	if(dest && src){
		while(*dest = *src++)dest++;
	}
	return dest;
}
char* ustrchr(const char* s1,char c){
	if(s1){
		char c2;
		while(c2 = *s1){
			if(c2==c)return (char*)s1;
			s1++;
		}
	}
	return 0;
}
char* ustrchrLast(const char* s1,char c){
	char* p=0;
	if(s1){
		char c2;
		while(c2 = *s1){
			if(c2==c)p=(char*)s1;
			s1++;
		}
	}
	return p;
}

char* ustrcat(char *dest,const char *src){
	if(dest){
		while(*dest)dest++;
		if(src){
			while(*dest = *src++)dest++;
		}
	}
	return dest;
}

char* uinstring(const char* data,const char* pattern){
	if(data){
		char c;
		int i;
		char c0= *pattern;
		
		for(;c = *data;data++){
			if(c!=c0)continue;
			for(i=1;;i++){
				c = pattern[i];
				if(!c)return (char*)data;
				if(pattern[i]!=data[i])break;
			}
		}
	}
	return null;
}
int ustrRemoveChars(char* data,char c){
	if(!data)return 0;
	char *esi=data,*edi=data,t;
	int num=0;
	do{
		t = *esi++;
		num++;
		if(t!=c){ *edi++ =t; num--;}
	}while(t);
	return num;
}
int ustrCountChars(const char* data,char c){
	char t;int count=0;
	while(t = *data++){
		if(t==c)count++;
	}
	return count;
}
int ustrReplaceChars(char* data,char c,char replacement){
	char t;int count=0;
	while(t = *data){
		if(t==c){
			*data=replacement;
			count++;
		}
		data++;
	}
	return count;
}

void ustrTrimWhiteSpace(char* data){
	if(!data)return;
	char *esi;
	if(*data==32 || *data==9){
		esi=data;
		do{
			esi++;
		}while(*esi==32 || *esi==9);
		char *edi=data;
		while(*edi = *esi++)edi++;
	}
	if(data[0]==0)return;
	esi = data;
	while(esi[1])esi++;
	while(esi!=data){
		if(*esi==32 || *esi==9){
			*esi-- = 0;
			continue;
		}
		break;
	}
}

int ustrRemoveSubStrings(char* data,const char* pattern){
	if(!data || !pattern)return 0;
	char x1=pattern[0]; if(!x1)return 0;
	char x2=pattern[1];
	char c,*edi;
	if(!x2)return ustrRemoveChars(data,x1);
	edi = data;
	int numRemoved=0;

	if(pattern[2]==0){ // two-char pattern
		while(c=*data++){
			if(c==x1 && *data==x2){
				data++;
				numRemoved++;
			}else{
				*edi++ = c;
			}
		}
	}else{ // pattern of 3 or more characters
		pattern++;
		int plen=ustrlen(pattern);
		while(c= *data++){
			if(c==x1 && *data==x2){
				int i;
				for(i=0;i<plen && data[i]==pattern[i];i++){}
				if(i==plen){
					data+=plen; 
					numRemoved++;
				}else{
					*edi++ = c;
				}
			}else{
				*edi++ = c;
			}
		}
	}
	return numRemoved;
}


bool ustrEndsWith(const char* data,const char* suffix){
	if(data && suffix){
		int d1=ustrlen(data);
		int d2=ustrlen(suffix);
		if(d1<d2)return false;
		if(!d2)return false;
		return ustrcmp(data+d1-d2,suffix)==0;
	}
	return false;
}

bool ustrStartsWith(const char* data,const char* prefix){
	if(data && prefix){
		if(data[0]==0)return false;
		char c,d;
		do{
			c= *data++;
			d= *prefix++;
			if(d==0)return true;
			if(c!=d)return false;
		}while(c);
		
	}
	return false;
}

int ustrcmp(const char* s1,const char* s2){
	if(!s1 || !s2)return 1;
	const U8* e1=(const U8*)s1;
	const U8* e2=(const U8*)s2;
	
	for(;;){
		int a = *e1++;
		int b = a - *e2++;
		if(b)return b;
		if(!a)return 0;		
	}
}



int uatoi(char** ppString){
	char* esi = *ppString;
	int x=0,neg=1;
	while(*esi==32 || *esi==9)esi++;

	
	if(*esi=='-'){neg=-1;esi++;}
	while(1){
		char c=*esi - '0';
		if(c<0 || c>9)break;
		x = x*10+c;
		esi++;
	}
	if(*esi==32 || *esi==9)esi++;
	*ppString = esi;
	return x*neg;
}

int uatoh(char** ppString){
	char* esi = *ppString;
	int x=0,neg=1;
	while(*esi==32 || *esi==9)esi++;

	
	if(*esi=='-'){neg=-1;esi++;}
	while(1){
		char c=*esi;
		if(c>='0' && c<='9'){
			c-='0';
		}else if(c>='A' && c<='F'){
			c-= 'A' - 10;
		}else if(c>='a' && c<='f'){
			c-= 'a' - 10;
		}else{
			break;
		}
		x = x*16+c;
		esi++;
	}
	if(*esi==32 || *esi==9)esi++;
	*ppString = esi;
	return x*neg;
}

extern "C" double pow(double x,double y);

float uatof(char** ppString){
	double x=0; double neg=1;
	int ix=0;

	char* esi = *ppString;
	while(*esi==32 || *esi==9)esi++;

	if(*esi=='-'){neg=-1;esi++;}
	while(1){
		char c=*esi - '0';
		if(c<0 || c>9)break;
		ix = ix*10+c;
		esi++;
	}
	if(*esi=='.'){
		esi++;
		double cf,muli=0.1,muli2=0.1;
		while(1){
			char c=*esi - '0';
			if(c<0 || c>9)break;
			esi++;
			cf = c;
			x+= cf*muli;
			muli*=muli2;			
		}
	}
	if(*esi=='e' || *esi=='E'){
		char signz = esi[1];
		esi+=2;
		double powa=0;
		while(*esi>='0' && *esi<='9'){
			powa = powa*10+ (*esi++)-'0';
		}
		if(signz=='+') neg= (neg*pow(10.0f,powa));
		if(signz=='-') neg= (neg*pow(0.1f,powa));
	}

	if(*esi==32 || *esi==9)esi++;
	
	*ppString = esi;
	return (float)(neg*(x+ix));
}

char* uitoa(char* dest,int x){
	char res[10];
	int i=0;
	if(x<0){*dest++ = '-'; x= -x;}
	do{
		res[i++] = (char)((x % 10)+'0');
		x/=10;
	}while(x);
	while(i){
		*dest++ = res[--i];
	}
	*dest = 0;
	return dest;
}


char* ui64toa(char* dest,U64 x){
	char res[20];
	int i=0;
	//if(x<0){*dest++ = '-'; x= -x;}
	do{
		res[i++] = (char)((x % 10)+'0');
		x/=10;
	}while(x);
	while(i){
		*dest++ = res[--i];
	}
	*dest = 0;
	return dest;
}
extern "C" double __cdecl floor(double X);
extern "C" double __cdecl ceil(double X);

char* uftoa2(char* dest,float x){
	double y=x,z;
	if(y<0.0){y=-y;*dest++='-';}
	z = floor(y); y-=z;
	dest = uitoa(dest,(int)z);
	*dest++='.';
	int m = int((y+0.000005) *100000.0);
	
	for(int i=5;i--;){
		int t = m/10;
		dest[i] = (char)('0'+ (m-t*10));
		m=t;
	}
	dest+=5;
	*dest=0;
	return dest;
}

char* uftoa(char* dest,float x){
	dest = uftoa2(dest,x);
	while(dest[-1]=='0' && dest[-2]!='.')dest--;
	*dest=0;
	return dest;
}
char* uitoaHex(char* dest,int x){
	const static char hexi[17]="0123456789ABCDEF";
	for(int i=0;i<8;i++){
		*dest++ = hexi[(x>>28)&15];
		x<<=4;
	}
	*dest=0;
	return dest;
}


char* uitoaRank(char* dest,int x){ // 1st, 2nd, 3rd, 4th, 11th, 
	char res[10];
	int i=0;
	if(x<0){*dest++ = '-'; x= -x;}
	do{
		res[i++] = (char)((x % 10)+'0');
		x/=10;
	}while(x);
	while(i){
		*dest++ = res[--i];
	}
	if(res[1]=='1' && (res[0]=='1' || res[0]=='2' || res[0]=='3'))return ustrcpy(dest,"th"); // 11th, 12th, 13th, 111th, 212th, 213th
	if(res[0]=='1')return ustrcpy(dest,"st");
	if(res[0]=='2')return ustrcpy(dest,"nd");
	if(res[0]=='3')return ustrcpy(dest,"rd");
	return ustrcpy(dest,"th");
}

char* usprintf(char* dest,const char* format, ...){
	va_list marker;
	va_start(marker,format);
	char c;
	while(c = *format++){
		if(c=='%'){
			c = *format++;
			if(c==0)break;
			else if(c=='%')*dest++ = c;
			else if(c=='d')dest = uitoa(dest,va_arg(marker,int));
			else if(c=='f')dest = uftoa(dest,(float)va_arg(marker,double));
			else if(c=='F')dest = uftoa2(dest,(float)va_arg(marker,double));
			else if(c=='X')dest = uitoaHex(dest,va_arg(marker,int));
			else if(c=='x')dest = uitoaHex(dest,va_arg(marker,int));
			else if(c=='L')dest = ui64toa(dest,va_arg(marker,U64));
			else if(c=='r')dest = uitoaRank(dest,va_arg(marker,int));
			else if(c=='s')dest = ustrcpy(dest,va_arg(marker,char*));
			else if(c=='z'){
				int zlen = va_arg(marker,int);
				memmove(dest,va_arg(marker,char*),zlen);
				dest += zlen;
			}
		}else{
			*dest++ = c;
		}
	}
	va_end(marker);
	*dest=0;
	return dest;
}
char* ustrclone(const char* x){ // allocates a new string!
	int len = ustrlen(x)+1;
	char* res = (char*)xmalloc(len);
	memcpy(res,x,len);
	return res;
}
char* ustrmix(const char* src1,const char* src2){ // allocates a new string!
	int len1 = ustrlen(src1);
	int len2 = ustrlen(src2);
	char* dest = (char*)xmalloc(len1+len2+1);
	ustrcpy(dest,src1);
	ustrcpy(dest+len1,src2);
	return dest;
}

char* ustrmix(const char* src1,const char* src2,const char* src3){ // allocates a new string via xmalloc() !
	int len1 = ustrlen(src1);
	int len2 = ustrlen(src2);
	int len3 = ustrlen(src3);
	char* dest = (char*)xmalloc(len1+len2+len3+1);
	ustrcpy(dest,src1);
	ustrcpy(dest+len1,src2);
	ustrcpy(dest+len1+len2,src3);
	return dest;
}
char* ustrmix(const char* src1,const char* src2,const char* src3,const char* src4){ // allocates a new string via xmalloc() !
	int len1 = ustrlen(src1);
	int len2 = ustrlen(src2);
	int len3 = ustrlen(src3);
	int len4 = ustrlen(src4);
	char* dest = (char*)xmalloc(len1+len2+len3+len4+1);
	ustrcpy(dest,src1);
	ustrcpy(dest+len1,src2);
	ustrcpy(dest+len1+len2,src3);
	ustrcpy(dest+len1+len2+len3,src4);
	return dest;
}


char* ustrSplitLines(char* data,int* pNumLines){
	ustrRemoveChars(data,13);
	*pNumLines = 1 + ustrReplaceChars(data,10,0);
	return data;
}
char** ustrSplitLinesToArray(char* data,int* pNumLines){
	ustrRemoveChars(data,13);
	int numLines= 1+ustrCountChars(data,10);
	char** lines=(char**)xmalloc(numLines*4);
	ustrReplaceChars(data,10,0);
	for(int i=0;i<numLines;i++){
		lines[i]=data;
		while(*data)data++;	data++;
	}
	*pNumLines = numLines;
	return lines;	
}


char* ustrPathFromFileName(const char* FileName,const char* AppendStuff){ 
	if(FileName){
		char* slash = ustrchrLast(FileName,'\\');
		if(!slash)slash = ustrchrLast(FileName,'/');
		if(!slash)return ustrclone(AppendStuff);
		int len1 = int(slash+1-FileName);
		int len2 = ustrlen(AppendStuff);
		char* result = (char*)xmalloc(len1+len2+1);
		memcpy(result,FileName,len1);
		memcpy(result+len1,AppendStuff,len2);
		return result;
	}else{
		if(AppendStuff)return ustrclone(AppendStuff);
	}
	return null;
}
//========================================================================/

//============[ VKDebug ]===========================================[
void DebugClear(){
	HWND hwnd;
	hwnd = FindWindowA("DbgWinClass",0);
	if(!hwnd)return;
	//SendMessageA(hwnd,WM_CLOSE,0,0);
	hwnd=FindWindowExA(hwnd,0,"Edit",0);
	SendMessage(hwnd, WM_SETTEXT, 0, 0);
}

void DebugHide(){
	HWND hwnd;
	hwnd = FindWindowA("DbgWinClass",0);
	if(!hwnd)return;
	SendMessageA(hwnd,WM_CLOSE,0,0);
}
void DebugPrint(const char* DebugData){
	HWND hwnd;
	hwnd = FindWindowA("DbgWinClass",0);
	if(!hwnd){
		HWND foreWnd = GetForegroundWindow();
		const char* vkPath=null;

		for(int i=0;;i++){
			const char* fn = VKDebug_Paths[i];
			if(fn==null)break;
			if(GetFileAttributesA(fn)==0xFFFFFFFF)continue;
			vkPath = fn;
		}
		if(!vkPath)return;
		WinExec(vkPath,SW_SHOWNORMAL);
		hwnd = FindWindowA("DbgWinClass",0);
		if(!hwnd)return;
		SetForegroundWindow(foreWnd);
	}
	hwnd=FindWindowExA(hwnd,0,"Edit",0);
	int len = (int)SendMessageA(hwnd, WM_GETTEXTLENGTH, 0, 0);
	SendMessageA(hwnd, EM_SETSEL, 0xFFFFFFFF, 0xFFFFFFFF);
	if(len){
		SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
	}
	SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM)DebugData);
	SendMessageA(hwnd, EM_SCROLLCARET, 0, 0);
}
void DebugPrint(const char* s1,const char* s2){
	char *s3 = (char*)xmalloc(ustrlen(s1)+ustrlen(s2)+4);
	char* p=ustrcpy(s3,s1);
	p=ustrcpy(p," = ");
	ustrcpy(p,s2);
	DebugPrint(s3);
	xfree(s3);
}
void DebugPrint(const char* s1,int x){
	char s3[300]; usprintf(s3,"%s = %d",s1,x);DebugPrint(s3);
}
void DebugPrint(const char* s1,U32 x){
	char s3[300]; usprintf(s3,"%s = %d",s1,x);DebugPrint(s3);
}
void DebugPrint(const char* s1,U64 x){
	char s3[300]; usprintf(s3,"%s = %L",s1,x);DebugPrint(s3);
}
void DebugPrint(const char* s1,void* ptr){
	char s3[300]; usprintf(s3,"%s = 0x%X",s1,ptr);DebugPrint(s3);
}

void DebugPrint(const char* s1,float x){
	char s3[300]; usprintf(s3,"%s = %f",s1,x);DebugPrint(s3);
}
void DebugPrint(const char* s1,double x){
	char s3[300]; usprintf(s3,"%s = %f",s1,x);DebugPrint(s3);
}
//void DebugPrint(const char* s1,vec3 v){
//	char s3[200]; sprintf(s3,"%s = <%f %f %f>",s1,v.x,v.y,v.z);DebugPrint(s3);
//}

void DebugPrintID(const char* s1,int x){
	char x2[5],*px=(char*)&x;
	x2[0]=px[3];x2[1]=px[2];x2[2]=px[1];x2[3]=px[0];x2[4]=0;
	DebugPrint(s1,x2);
}
void DebugPrintHex(const char* s1,int x){
	char s3[300]; usprintf(s3,"%s = 0x%X",s1,x);DebugPrint(s3);
}
void DebugPrintLine(){
	DebugPrint("--------------------------");
}

void trace(const char* format, ...){
	va_list marker;
	va_start(marker,format);
	char dest2[1000];
	char* dest=dest2;
	char c;
	while(c = *format++){
		if(c=='%'){
			c = *format++;
			if(c==0)break;
			else if(c=='%')*dest++ = c;
			else if(c=='d')dest = uitoa(dest,va_arg(marker,int));
			else if(c=='f')dest = uftoa(dest,(float)va_arg(marker,double));
			else if(c=='F')dest = uftoa2(dest,(float)va_arg(marker,double));
			else if(c=='X')dest = uitoaHex(dest,va_arg(marker,int));
			else if(c=='x')dest = uitoaHex(dest,va_arg(marker,int));
			else if(c=='r')dest = uitoaRank(dest,va_arg(marker,int));
			else if(c=='s')dest = ustrcpy(dest,va_arg(marker,char*));
			else if(c=='z'){
				int zlen = va_arg(marker,int);
				memmove(dest,va_arg(marker,char*),zlen);
				dest += zlen;
			}
		}else{
			*dest++ = c;
		}
	}
	va_end(marker);
	*dest=0;
	DebugPrint(dest2);
}

void uexit(const char* Error){
	if(Error)print(Error);
	ExitProcess(1);
}
void uexit(const char* format,const char* text1){
	trace(format,text1);
	ExitProcess(1);
}
//==================================================================/

//=============[ ObjVector ]========================[
ObjVector::ObjVector(){
	num=0;
	takenSlots=1;
	data= (void**)xmalloc(8);
	data[0]=0; // this is a terminating zero!
}
ObjVector::ObjVector(int initialSize){
	if(initialSize<1)initialSize=1;
	num=0;
	takenSlots=initialSize;
	data= (void**)xmalloc(takenSlots*4+4);
	//data[0]=0; // this is a terminating zero!
}
ObjVector::~ObjVector(){
	xfree(data);
}
void ObjVector::add(void* d){
	if(d){
		if(num<takenSlots){
			data[num++]=d;
			data[num]=0; // terminating value!
			return;
		}
		takenSlots<<=1;
		data = (void**)xresize(data,takenSlots*4+4);
		data[num++]=d;
		data[num]=0; // terminating value!
	}
}
void ObjVector::addAtIndex(void* d,int index){
	if(d){
		if(index>=0 && index<=num && num<takenSlots){
			num++;
			do{
				void* f1=data[index];
				data[index]=d;
				d = f1;
				index++;
			}while(index<num);
			data[num]=0;
			return;
		}
		//----- slower path:
		if(index<0)index=0;
		if(index>num)index=num;
		if(num==takenSlots){
			takenSlots<<=1;
			data = (void**)xresize(data,takenSlots*4+4);
		}
		num++;
		for(int i=index;i<num;i++){
			void* f1=data[i];
			data[i]=d;
			d = f1;
		}
		data[num]=0; // terminating value!
	}
}

void ObjVector::addMultiple(void** pd,int count){
	if(!count)return;
	if(num+count<=takenSlots){
		memmoveDword(data+num,pd,count);
		num+=count;
		data[num]=0; // terminating zero
		return;
	}
	do{
		takenSlots<<=1;
	}while(num+count>takenSlots);
	
	data = (void**)xresize(data,takenSlots*4+4);
	memmoveDword(data+num,pd,count);
	num+=count;
	data[num]=0; // terminating zero
}
void ObjVector::addAllFrom(ObjVector* o){
	addMultiple(o->data,o->num);
}
void ObjVector::slimDown(){
	if(num==takenSlots || takenSlots==1)return;
	takenSlots=num;
	data = (void**)xresize(data,takenSlots*4+4);
}

void ObjVector::addToStart(void* d){
	if(d){
		if(num==takenSlots){
			takenSlots<<=1;
			data = (void**)xresize(data,takenSlots*4+4);
		}
		num++;
		for(int i=0;i<num;i++){
			void* f1=data[i];
			data[i]=d;
			d = f1;
		}
		data[num]=0; // terminating value!
	}
}
int ObjVector::remove(void *d){
	int i,n=num,res;
	void** p=data;
	for(i=0;i<n;i++){
		if(p[i]==d)break;
	}
	if(i>=n)return -1;
	res = i;
	num--;
	while(i<=num){
		data[i] = data[i+1];
		i++;
	}
	return res;
}
void ObjVector::remove(int index){
	if(index>=0 && index<num){
		num--;
		while(index<=num){
			data[index] = data[index+1];
			index++;
		}
	}
}
void ObjVector::removeQuickly(int index){
	if(index>=0 && index<num){
		num--;
		data[index] = data[num];
		data[num]=null;
	}
}
int  ObjVector::contains(void* d){
	void** p=data;
	for(int n=num;n--;){
		if(p[n]!=d)continue;
		return n+1;
	}
	return 0;
}
//==================================================/


//===========================[[ BakedVector ]]========================================================[[
BakedVector* BakedVector::FromObjVector(ObjVector* o){
	BakedVector* vec = New(o->num);
	memmoveDword(vec->data,o->data,vec->num+1);
	return vec;
}
ObjVector* BakedVector::ToObjVector(){
	ObjVector* o = new ObjVector(num);
	o->num=num;
	memmoveDword(o->data,data,num+1);
	return o;
}

BakedVector* BakedVector::add(void* d){
	if(!d)return this;
	data[num]=data;
	num++;
	BakedVector* vec = (BakedVector*)xresize(this,(num+2)*4);
	return vec;
}
// ... implement the others ...
//====================================================================================================//




static void hashtab_DefDel(void* pdata){
	xfree(pdata);
}
static U32 hashtab_getHash(const char* name){
	U32 val=7;
	while(char c= *name++){
		val+= c -'A';
	}
	return val;
}
static U32 hashtab_getHash2(const char* name){
	U32 val=7,k=0;
	char* s=(char*)&val;
	while(char c= *name++){
		val+= c;
		s[k]-=c-'a';
		k = (k+1)&3;
	}
	return val;
}

void CHashTable::add(const char* name,void* value){
	U32 hash = hashtab_getHash(name) & (tableSize-1);
	U32 hash2= hashtab_getHash2(name);
	ObjVector* vn= table[hash];
	//---[ find duplicate ]-------[
	if(vn){
		foreach(vn,U32*){
			if(edx[0]==hash2){
				if(0==ustrcmp((char*)(edx+2),name)){
					if(ValueDeleter){
						ValueDeleter((void*)edx[1]);
					}
					edx[1]=(U32)value;
					return;
				}
			}
		}
	}else{
		vn = new ObjVector();
		table[hash]=vn;
	}
	//----------------------------/

	
	int* name2 = (int*)xmalloc(8+1+ustrlen(name));
	name2[0]=hash2;
	name2[1]=(int)value;
	ustrcpy((char*)(name2+2),name);
	vn->add(name2);
	numElements++;
}
void CHashTable::remove(const char* name){
	int hash = hashtab_getHash(name) & (tableSize-1);
	int hash2= hashtab_getHash2(name);
	ObjVector* vn= table[hash];
	int idx=0;
	if(vn){
		foreach(vn,int*){
			if(edx[0]==hash2){
				if(0==ustrcmp((char*)(edx+2),name)){
					if(ValueDeleter){
						ValueDeleter((void*)edx[1]);
					}
					xfree(edx);
					vn->remove(idx);
					numElements--;
					return;
				}
			}
			idx++;
		}
	}
}
void* CHashTable::find(const char* name){
	if(!name)return null;
	U32 hash = hashtab_getHash(name) & (tableSize-1);
	U32 hash2= hashtab_getHash2(name);
	ObjVector* vn= table[hash];
	if(vn==null)return null;
	foreach(vn,U32*){
		if(edx[0]==hash2){
			if(0==ustrcmp((char*)(edx+2),name)){
				return (void*)edx[1];
			}
		}
	}
	return null;
}

CHashTable::CHashTable(int tabSizeLog2){
	numElements=0;
	tableSize=1<<tabSizeLog2;
	ValueDeleter=hashtab_DefDel;
	table = new ObjVector*[tableSize];
	for(int i=0;i<tableSize;i++)table[i]=null;
}

CHashTable::~CHashTable(){
	for(int i=0;i<tableSize;i++){
		ObjVector* v = table[i];
		if(v==null)continue;
		if(ValueDeleter!=null){
			foreach(v,void**){
				ValueDeleter(edx[1]);
				xfree((void*)edx);
			}
		}else{
			foreach(v,void**){
				xfree((void*)edx);
			}
		}
		delete v;
	}
	delete[] table;
}













//===========[ garbage collector ]=================================[
Garbage::Garbage(){
	NumObjects=0;pObjects=0;pSizes=0;
}
Garbage::~Garbage(){
	if(!NumObjects)return;
	for(int i=NumObjects;i--;){
		xfree(pObjects[i]);
	}
	xfree(pObjects);
	xfree(pSizes);
}
void  Garbage::addGarbage(void* ptr){
	if(!ptr)return;
	pObjects=(void**)xresize(pObjects,NumObjects*4+4);
	pSizes  =(int*)  xresize(pSizes,  NumObjects*4+4);
	pObjects[NumObjects]=ptr;
	pSizes[NumObjects]=0;
	NumObjects++;
}

void* Garbage::alloc(int size){
	if(size<=0)return 0;
	void* data = (void*)xmalloc(size);
	if(!data)return 0;
	pObjects=(void**)xresize(pObjects,NumObjects*4+4);
	pSizes  =(int*)  xresize(pSizes,  NumObjects*4+4);
	pObjects[NumObjects]=data;
	pSizes[NumObjects]=size;
	NumObjects++;
	return data;
}
void Garbage::free(void* ptr){
	for(int i=NumObjects;i--;){
		if(pObjects[i]!=ptr)continue;
		xfree(ptr);
		pObjects[i]=pObjects[NumObjects-1];
		pSizes[i]=pSizes[NumObjects-1];
		pObjects=(void**)xresize(pObjects,NumObjects*4-4);
		pSizes  =(int*)  xresize(pSizes,  NumObjects*4-4);
		NumObjects--;
	}
}
char* Garbage::addString(const char* str){
	if(!str)return 0;
	char* s = ustrclone(str);
	addGarbage(s);
	return s;
}

int Garbage::getSize(void* ptr){
	for(int i=NumObjects;i--;){
		if(pObjects[i]==ptr){
			return pSizes[i];
		}
	}
	return 0;
}
//=================================================================/

CRtlInitializer::CRtlInitializer(void** target,int targetsize,void* data,int index){
	if(index<0 || index>=targetsize/4){
		DebugPrint("CRtlInitializer: index out of bounds",index);
		return;
	}
	if(target[index]){
		DebugPrint("CRtlInitializer: index already taken: ",index);
		return;
	}
	target[index]=data;
	dummy=0;
}

CRtlInitializer::CRtlInitializer(void** target,int& count,void* data){
	target[count++]=data;
}



VFile::VFile(){
	size=0;
	taken=32;
	m_data=(char*)xmalloc(taken);
}
VFile::~VFile(){
	if(m_data)xfree(m_data);
}
void VFile::write(const void* data,int dataSize){
	int newsize=size+dataSize;

	if(newsize>taken){
		if(!taken)taken=16;
		do{	taken<<=1;}while(newsize>taken);
		m_data = (char*)xresize(m_data,taken);
	}
	memcpy(m_data+size,data,dataSize);
	size=newsize;
}
void VFile::writeline(const char* text){
	int len=ustrlen(text);
	int newsize=size+len+2;

	if(newsize>taken){
		if(!taken)taken=16;
		do{	taken<<=1;}while(newsize>taken);
		m_data = (char*)xresize(m_data,taken);
	}
	memcpy(m_data+size,text,len);
	m_data[size+len+0]=13;
	m_data[size+len+1]=10;

	size=newsize;
}
void* VFile::stealData(){
	if(size==0)return null;
	if(taken!=size)m_data=(char*)xresize(m_data,size);
	void* result = m_data;
	m_data=null;
	size=0;
	taken=0;
	return result;
}
char* VFile::stealString(){
	if(taken!=size+1)m_data=(char*)xresize(m_data,size+1);
	m_data[size]=0;
	char* result = m_data;
	m_data=0;
	size=0;
	taken=0;
	return result;
}


//================================[ MEMORY ]=========================================================================================[
static int xmalloc_NumObjects=0;
static int xmalloc_ObjectSize=0;
HANDLE HEAP1=0;




static HANDLE __MemHeap1=null;
static int __MemHeap1_numo=0,__MemHeap1_osiz=0;

void __xMEMORY_PUSHHEAP(){
	if(__MemHeap1){
		prints("Heap already pushed!");
		return;
	}
	__MemHeap1 = HEAP1;
	__MemHeap1_numo = xmalloc_NumObjects;
	__MemHeap1_osiz = xmalloc_ObjectSize;
	HEAP1 = HeapCreate(0,10000,0);
	xmalloc_NumObjects=0;
	xmalloc_ObjectSize=0;
}
void __xMEMORY_POPHEAP(){
	HeapDestroy(HEAP1);

	HEAP1=__MemHeap1;
	xmalloc_NumObjects=__MemHeap1_numo;
	xmalloc_ObjectSize=__MemHeap1_osiz;

	__MemHeap1=null;
	__MemHeap1_numo=0;
	__MemHeap1_osiz=0;
}



#ifndef ULTRANO_DEBUG_MEMORY


__declspec(noinline) void* xmalloc(int size){
	void* ptr;
	if(size>0){
		if(HEAP1){
			ptr = HeapAlloc(HEAP1,HEAP_ZERO_MEMORY,size);
			if(ptr){
				xmalloc_NumObjects++;
				return ptr;
			}
			DebugPrint("xmalloc failed at size",size);
			return 0;
		}
		HEAP1 = HeapCreate(0,10000,0);
		return xmalloc(size);
	}
	return 0;
}


__declspec(noinline) void xfree(void* what){
	if(what){
		HeapFree(HEAP1,0,what);
		xmalloc_NumObjects--;
	}
}

__declspec(noinline) void* xresize(void* ptr,int newsize){
	void* newptr;
	if(ptr){
		if(newsize){
			newptr = HeapReAlloc(HEAP1,HEAP_ZERO_MEMORY,ptr,newsize);
			return newptr;
		}
		xfree(ptr);
		return 0;
	}
	return xmalloc(newsize);
}
int xmemsize(const void* ptr){
	if(ptr){
		int size=(int)HeapSize(HEAP1,0,ptr);
		if(size>0)return size;
	}
	return 0;
}

int __xMEMORY_MARK_NON_LEAKY_XMALLOCS(bool bMarkAsNonLeaky){
	return 0;
}
void __xMEMORY_BREAK_ON_AGE(int XM_Object_Age){
}

#else // ULTRANO_DEBUG_MEMORY is defined




//--------------------[ debugged memory stuff ]---------------------------[

#include <imagehlp.h>

#pragma comment(lib,"imagehlp.lib")

static int XM_OBJECT_AGE=0;
static int  XM_OBJECT_BREAK_ON_AGE=0;
static bool XM_OBJECT_BREAK_ON_AGE_ENABLED=false;


struct XM_OBJECT{
	int magic0; // ='OcET';
	int ProcAddress; // from where it was gotten
	int size;
	int age;
	int isNonLeaky;
	int padding0;
	int magic1; // ='Ilko'
	int magic2; // ='DiNk'
	//char data[0];
	// note that there are also "magic3", "magic4" and "magic5" right after the data
}; // 32 bytes!! Keep it this big, for alignment stuff

static void* XM_OBJECT_INIT(void* ptr,int _ProcAddress,int _size,int _isNonLeaky=false){
	if(ptr==null)return null;
	XM_OBJECT* o = (XM_OBJECT*)ptr;
	o->ProcAddress = _ProcAddress;
	o->size= _size;
	o->age = ++XM_OBJECT_AGE;
	o->isNonLeaky = _isNonLeaky;
	o->magic0 = 'OcET';
	o->magic1 = 'Ilko';
	o->magic2 = 'DiNk';

	char* pcMagic3 = (char*) &o[1];  // points to start of data
	int* piMagic3 = (int*) &pcMagic3[_size];
	piMagic3[0] = 'hEiK';
	piMagic3[1] = 'On91';
	piMagic3[2] = 'GoaR';


	if(XM_OBJECT_AGE==XM_OBJECT_BREAK_ON_AGE && XM_OBJECT_BREAK_ON_AGE_ENABLED){
		__asm int 3
	}

	
	return o+1;
}
static XM_OBJECT*  XM_OBJECT_ISVALID(const void* ptr){
	if(!ptr)return null;
	XM_OBJECT* o = (XM_OBJECT*)ptr;
	if(!o)return null;
	o--;
	// check back to front!
	if(o->magic2!='DiNk')return null; 
	if(o->magic1!='Ilko')return null;
	if(o->magic0!='OcET')return null;

	char* pcMagic3 = (char*) &o[1]; // points to start of data
	int* piMagic3 = (int*) &pcMagic3[o->size];
	if(piMagic3[0] != 'hEiK')return null;
	if(piMagic3[1] != 'On91')return null;
	if(piMagic3[2] != 'GoaR')return null;
	return o;
}


static XM_OBJECT*  XM_OBJECT_IS_PARTLY_VALID(const void* ptr){
	if(!ptr)return null;
	XM_OBJECT* o = (XM_OBJECT*)ptr;
	if(!o)return null;
	o--;
	// check back to front!
	//if(o->magic2!='DiNk')return null; // skip, as is overwritable usually
	//if(o->magic1!='Ilko')return null; // skip, as is overwritable usually
	if(o->magic0!='OcET')return null;

	char* pcMagic3 = (char*) &o[1]; // points to start of data
	int* piMagic3 = (int*) &pcMagic3[o->size];
	//if(piMagic3[0] != 'hEiK')return null; // skip, as is overwritable usually
	//if(piMagic3[1] != 'On91')return null; // skip, as is overwritable usually
	if(piMagic3[2] != 'GoaR')return null;
	return o;
}



static void XM_OBJECT_CLEAR(XM_OBJECT* o){
	o->magic0=0;
	o->magic1=0;
	o->magic2=0;
}

int __cdecl XM_OBJECT_COMPARE(const XM_OBJECT* aa,const XM_OBJECT* bb){
	return aa->age - bb->age;
}


static void PrintXMallocSymbolFromAddress(int ProcAddress,XM_OBJECT* o=null){
	static HANDLE myProcess=0;
	static bool bInited=false;
	static bool bInitSuccess=false;

	ProcAddress-=5; // remove the "call" instruction


	int success,error;

	if(!bInited){
		bInited=true;
		myProcess = GetCurrentProcess();
		success = SymInitialize(myProcess,null,true);
		if(!success){
			error = GetLastError();
			trace("<Failed to SymInitialize> <Error: %d>",error);
			return;
		}
		bInitSuccess=true;
	}

	if(!bInitSuccess)return;

	
	char symdata[sizeof(IMAGEHLP_SYMBOL)+600];
	IMAGEHLP_SYMBOL * s = (IMAGEHLP_SYMBOL *)symdata;
	s->SizeOfStruct=sizeof(IMAGEHLP_SYMBOL);
	s->MaxNameLength=300;
	//s->Address = ProcAddress;
	//s->Flags=SYMF_OMAP_GENERATED;
	DWORD displace=0;
	success = SymGetSymFromAddr(myProcess,ProcAddress,&displace,s);
	if(!success){
		error = GetLastError();
		trace("<Failed to SymGetSymFromAddr> <Error: %d>",error);
		return;
	}

	prints(s->Name);
	trace("OFFSET from symbol: %d bytes",displace);

	IMAGEHLP_LINE64 line; line.SizeOfStruct = sizeof(line);	

	success=SymGetLineFromAddr64(myProcess,ProcAddress,&displace,&line);
	if(!success){
		//error = GetLastError();
		//trace("<Failed to SymGetLineFromAddr64> <Error: %d>",error);
		line.LineNumber=1;
		line.FileName="<unknown>";
	}

	int age=0; if(o)age=o->age;

	char outdebugstr[600];
	usprintf(outdebugstr,"1>%s(%d) : warning, LEAK with age=%d in : %s\n",line.FileName,line.LineNumber,age,s->Name);
	OutputDebugString(outdebugstr);

	trace("LINE:   %d",line.LineNumber);
	trace("FILE:");
	trace(line.FileName);
}

static void XM_OBJECT_PRINT(XM_OBJECT* o){
	prints(" ");
	prints("----------[ XM_OBJECT ]---------------------[");
	if(!XM_OBJECT_ISVALID(o))prints("CORRUPTED OBJECT!!!!!!");
	trace("ADDRESS: 0x%X",&o[1]);
	trace("SIZE:    %d",o->size);
	trace("AGE:     %d",o->age);
	trace("CREATOR:	0x%X",o->ProcAddress);
	prints(" ");
	prints(" ");
	PrintXMallocSymbolFromAddress(o->ProcAddress,o);



	prints("--------------------------------------------/");
	prints(" ");
}











static void* __cdecl intl_xmalloc(int size,int ProcAddress){
	//trace("xmalloc2(%d,%X)",size,ProcAddress);
	if(size>0){
		if(!HEAP1){
			HEAP1 = HeapCreate(0,10000,0);
		}
		void* ptr = HeapAlloc(HEAP1,HEAP_ZERO_MEMORY,size + sizeof(XM_OBJECT) + 12); // 12 bytes for 3 extra "magic3".."magic5" dwords
		if(ptr){
			ptr = XM_OBJECT_INIT(ptr,ProcAddress,size);
			xmalloc_ObjectSize+=size;
			xmalloc_NumObjects++;
			return ptr;
		}
		DebugPrint("xmalloc failed at size",size);
		return 0;


	}
	return 0;
}


static void __cdecl intl_xfree(void* what,int ProcAddress){
	if(what){
		XM_OBJECT* o = XM_OBJECT_ISVALID(what);
		if(!o){
			trace("xfree() problem, trying to delete a bad object at addr 0x%X,  calling proc is at addr 0x%X",what,ProcAddress);
			PrintXMallocSymbolFromAddress(ProcAddress);
			DebugBreak();
			return;
		}
		int size=o->size;
		XM_OBJECT_CLEAR(o);
		xmalloc_ObjectSize-=size;
		xmalloc_NumObjects--;
		HeapFree(HEAP1,0,o);
	}
}
static void* __cdecl intl_xresize(void* ptr,int newsize,int ProcAddress){

	//trace("intl_xresize(%X,%d,%X)",ptr,newsize,ProcAddress);
	if(ptr){
		if(newsize){
			XM_OBJECT* o = XM_OBJECT_ISVALID(ptr);
			if(!o){
				trace("xresiz() problem, trying to resize a bad object at addr 0x%X,  calling proc is at addr 0x%X",ptr,ProcAddress);
				PrintXMallocSymbolFromAddress(ProcAddress);
				DebugBreak();
				return null;
			}
			int oldsize = o->size;
			int oldIsNonLeaky = o->isNonLeaky;
			XM_OBJECT_CLEAR(o);
			void* newptr = HeapReAlloc(HEAP1,HEAP_ZERO_MEMORY,o,newsize+sizeof(XM_OBJECT)+12); // the 12 bytes for magic3..magic5
			if(newptr==null){ // old memory is still intact!
				XM_OBJECT_INIT(ptr,ProcAddress,oldsize,oldIsNonLeaky);
				return null;
			}
			newptr = XM_OBJECT_INIT(newptr,ProcAddress,newsize,oldIsNonLeaky);

			xmalloc_ObjectSize+=newsize-oldsize;
			return newptr;
		}
		xfree(ptr);
		return null;
	}
	return xmalloc(newsize);
}

__declspec(naked) void* __cdecl xmalloc(int size){
	__asm{
		mov eax,esp;
		push dword ptr[eax];
		push dword ptr[eax+4];
		call intl_xmalloc;
		add esp,8;
		retn;
	}
}
__declspec(naked) void __cdecl xfree(void* what){
	__asm{
		mov eax,esp;
		push dword ptr[eax];
		push dword ptr[eax+4];
		call intl_xfree;
		add esp,8;
		retn;
	}
}


__declspec(naked) void* __cdecl xresize(void* ptr,int newsize){
	__asm{
		mov eax,esp;
		push dword ptr[eax];
		push dword ptr[eax+8];
		push dword ptr[eax+4];
		call intl_xresize;
		add esp,12;
		retn;
	}
}


int xmemsize(const void* ptr){
	if(ptr){
		XM_OBJECT* o = XM_OBJECT_ISVALID(ptr);
		if(!o){
			trace("xmemsize() problem, trying to lookup a bad object at addr 0x%X",ptr);
			DebugBreak();
			return 0;
		}
		return o->size;
	}
	return 0;
}



int __xMEMORY_MARK_NON_LEAKY_XMALLOCS(bool bMarkAsNonLeaky){
	if(!HEAP1)return 0;
	if(!HeapValidate(HEAP1,0,0))return 0;
	if(!xmalloc_NumObjects)return 0;
	PROCESS_HEAP_ENTRY h;
	memclear(&h,sizeof(h));

	int NumFoundObjs=0;
	while(HeapWalk(HEAP1,&h)){
		if(h.wFlags&PROCESS_HEAP_ENTRY_BUSY){
			XM_OBJECT* o = (XM_OBJECT*)(h.lpData); o++;
			o = XM_OBJECT_ISVALID(o);
			if(!o)continue;
			o->isNonLeaky = bMarkAsNonLeaky;
			NumFoundObjs++;
		}
	}
	return NumFoundObjs;
}

void __xMEMORY_BREAK_ON_AGE(int XM_Object_Age){
	XM_OBJECT_BREAK_ON_AGE=XM_Object_Age;
	XM_OBJECT_BREAK_ON_AGE_ENABLED=true;
}
//------------------------------------------------------------------------/

#endif



void memmove(void* dest,const void* src,int size){
	char *d=(char*)dest;
	char *s=(char*)src;
	if(size>0){
		while(size--)*d++ = *s++;
	}
}
void memmoveDword(void* dest,const void* src,int numDwords){
#ifndef __GNUC__
	__asm{
		push ecx
		push edi;
		push esi;
		mov ecx,numDwords;
		mov edi,dest;
		mov esi,src;
		rep movsd;
		pop esi;
		pop edi;
		pop ecx;
	}
#else
	int *d=(int*)dest;
	int *s=(int*)src;
	if(numDwords>0){
		while(numDwords--)*d++ = *s++;
	}
#endif
}

void* memclone(const void* ptr,int size){
	void* newptr;
	newptr = xmalloc(size);	
	memmove(newptr,ptr,size);
	return newptr;
}

void memclear(void* ptr,int size){
	char* d=(char*)ptr;
	while(size-- >0) *d++=0;
}

void* memAppendLg(void* ptr,int& usedSize,int& curSize,const void* dataToAppend,int dataToAppendSize){ // first the buffer is 1-byte, then 2-byte, then 4, then 8, 16, 32,.... 1024,2048,...
	if(dataToAppend && dataToAppendSize>0){
		int nextUsedSize = usedSize+dataToAppendSize;
		if(nextUsedSize<=curSize){
			memcpy(((char*)ptr)+usedSize,dataToAppend,dataToAppendSize);
			usedSize=nextUsedSize;
			return ptr;
		}
		if(curSize<=0)curSize=1;
		while(nextUsedSize>curSize)curSize+=curSize;
		ptr = xresize(ptr,curSize);
		memcpy(((char*)ptr)+usedSize,dataToAppend,dataToAppendSize);
		usedSize=nextUsedSize;
	}
	return ptr;
}


void OnExit_MemCheck(){
	if(!HEAP1)return;
	if(!HeapValidate(HEAP1,0,0))prints("warning, HEAP1 is not valid");
	if(!xmalloc_NumObjects)return;
	DebugPrint("warning, unallocated objects count",xmalloc_NumObjects);
#ifndef ULTRANO_DEBUG_MEMORY
	DebugPrint("Enable ULTRANO_DEBUG_MEMORY in Ultrano.h");
#else
		DebugPrint("size of unallocated memory",xmalloc_ObjectSize);
		PROCESS_HEAP_ENTRY h;
		memclear(&h,sizeof(h));

		XM_OBJECT** objlist;
		objlist = (XM_OBJECT**)HeapAlloc(HEAP1,HEAP_ZERO_MEMORY,xmalloc_NumObjects*4);
		int NumFoundObjs=0,NumFoundLeakyObjs=0;
		while(HeapWalk(HEAP1,&h)){
			if(h.wFlags&PROCESS_HEAP_ENTRY_BUSY){
				XM_OBJECT* o = (XM_OBJECT*)(h.lpData); o++;
				o = XM_OBJECT_IS_PARTLY_VALID(o);
				if(!o)continue;
				if(NumFoundObjs<xmalloc_NumObjects){
					NumFoundObjs++;
					if(!o->isNonLeaky || !XM_OBJECT_ISVALID(o))objlist[NumFoundLeakyObjs++]=o;
				}else{
					trace("OnExit_MemCheck() error! Found too many objects");
				}
				/*
				prints("-------------");
				trace("Chunk at 0x%X with size = %d",h.lpData,h.cbData);				
				print(h.cbOverhead);
				print(h.iRegionIndex);
				printh(h.wFlags);
				*/
			}
		}
		if(NumFoundObjs!=xmalloc_NumObjects){
			trace("OnExit_MemCheck() error! Didn't found all objects!");
		}
		//QSortA((int*)objlist,NumFoundObjs);
		if(NumFoundLeakyObjs){
			QSortB(objlist,NumFoundLeakyObjs,XM_OBJECT_COMPARE);
		}


		OutputDebugString("\n\n\n================[[ MEMORY LEAK LISTING ]]==============================[[\n\n\n\n");
		for(int i=0;i<NumFoundLeakyObjs && i<40;i++){
			XM_OBJECT_PRINT(objlist[i]);
		}
		OutputDebugString("\n\n\n=======================================================================[[\n\n\n\n");

		if(NumFoundLeakyObjs>40){
			trace("<Listed only the first 40 of %d leaks, of %d total objects>",NumFoundLeakyObjs,NumFoundObjs); 
		}
		
		HeapFree(HEAP1,0,objlist);
	#endif
}

U32 GetMemHash(const void* ptr,int size){
	U32 x=size<<20;
	const U8* p = (const U8*)ptr;
	while(size>0){
		size--;
		U32 d = p[size];
		x = (x ^ (d<<11)) + d - (d<<5);
	}
	return x;
}

//===================================================================================================================================/





//=======[[ FixedMemory ]==================================================[[

FixedAlloc::FixedAlloc(){
	z_all_free=null;
	z_NumFree=0;
}


FixedAlloc::FixedAlloc(int NumMaxElements, int ElementSize){
	init(NumMaxElements,ElementSize);
}

void FixedAlloc::init(int NumMaxElements, int ElementSize){
	z_NumFree=NumMaxElements;
	z_all_free = (void**) xmalloc(NumMaxElements* (ElementSize+4));
	
	char* d = (char*)z_all_free;
	d+= NumMaxElements*4;



	loopi(NumMaxElements){
		z_all_free[i]=d;
		d+=ElementSize;
	}
}


FixedAlloc::~FixedAlloc(){
	xfree(z_all_free);
}

void* FixedAlloc::alloc(){
	if(z_NumFree){
		z_NumFree--;
		return z_all_free[z_NumFree];
	}
	return null;
}

void FixedAlloc::free(void *ptr){
	z_all_free[z_NumFree] = ptr;
	z_NumFree++;
}
//--------------------------------------------------



FixedAlloc16::FixedAlloc16(){
	z_all_free=null;
	z_NumFree=0;
}


FixedAlloc16::FixedAlloc16(int NumMaxElements, int ElementSize){
	init(NumMaxElements,ElementSize);
}

void FixedAlloc16::init(int NumMaxElements, int ElementSize){
	ElementSize = (ElementSize+15) & (~15); // round-up to 16


	z_NumFree=NumMaxElements;
	z_all_free = (void**) xmalloc(NumMaxElements* (ElementSize+4) + 15);
	
	char* d = (char*)z_all_free;
	d+= NumMaxElements*4;
	int id=(int)d; id&=15;
	if(id)d+= 16-id;



	loopi(NumMaxElements){
		z_all_free[i]=d;
		d+=ElementSize;
	}
}


FixedAlloc16::~FixedAlloc16(){
	xfree(z_all_free);
}

void* FixedAlloc16::alloc(){
	if(z_NumFree){
		z_NumFree--;
		return z_all_free[z_NumFree];
	}
	return null;
}

void FixedAlloc16::free(void *ptr){
	z_all_free[z_NumFree] = ptr;
	z_NumFree++;
}
//=========================================================================//







//====================================[ FILES ]==============================================================[
static HANDLE CurFile=INVALID_HANDLE_VALUE;

char* uffetch(const char* fName,int *fSize){
	if(fSize)*fSize=0;
	if(!ufopenRead(fName))return 0;
	int size = ufsize();
	if(size<0){ufclose();return 0;}
	char *data = (char*)xmalloc(size+4);
	if(!data){ ufclose();return 0;}
	if(!ufread(data,size)){xfree(data);ufclose();return 0;}
	ufclose();
	if(fSize)*fSize = size;
	return data;
}
bool  ufdump(const char* fName,const void* data,int dataSize){
	if(!ufopenWrite(fName))return false;
	if(!ufwrite(data,dataSize)){ufclose();return false;}
	ufclose();
	return true;
}

bool ufopenRead(const char* FileName){
	if(CurFile!=INVALID_HANDLE_VALUE)CloseHandle(CurFile);
	//print(FileName);
	CurFile = CreateFileA(FileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	return CurFile!=INVALID_HANDLE_VALUE;
}
bool ufopenWrite(const char* FileName){
	if(CurFile!=INVALID_HANDLE_VALUE)CloseHandle(CurFile);
	CurFile = CreateFileA(FileName,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	return CurFile!=INVALID_HANDLE_VALUE;
}


bool ufopenReadW(const wchar_t* FileName){
	if(CurFile!=INVALID_HANDLE_VALUE)CloseHandle(CurFile);
	CurFile = CreateFileW(FileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	return CurFile!=INVALID_HANDLE_VALUE;
}
bool ufopenWriteW(const wchar_t* FileName){
	if(CurFile!=INVALID_HANDLE_VALUE)CloseHandle(CurFile);
	CurFile = CreateFileW(FileName,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	return CurFile!=INVALID_HANDLE_VALUE;
}

void ufclose(){
	if(CurFile==INVALID_HANDLE_VALUE)return;
	CloseHandle(CurFile);
	CurFile=INVALID_HANDLE_VALUE;
}
bool ufIsEndOfFile(){
	int siz = GetFileSize(CurFile,NULL);
	int pos = SetFilePointer(CurFile,0,0,FILE_CURRENT);
	return siz==pos;
}

int  ufsize(){
	return GetFileSize(CurFile,NULL);
}
void ufskip(int numBytes){
	SetFilePointer(CurFile,numBytes,0,FILE_CURRENT);
}
int  ufseek(int absoluteAddr){
	return SetFilePointer(CurFile,absoluteAddr,0,FILE_BEGIN);
}
int  uftell(){
	return SetFilePointer(CurFile,0,0,FILE_CURRENT);
}

bool ufread(void* data,int dataSize){
	DWORD numDone=0;
	ReadFile(CurFile,data,dataSize,&numDone,0);
	return numDone==(DWORD)dataSize;
}
char  ufread1(){
	char x=0;	DWORD numDone=0;
	ReadFile(CurFile,&x,1,&numDone,0);
	return x;
}
short ufread2(){
	short x=0;	DWORD numDone=0;
	ReadFile(CurFile,&x,2,&numDone,0);
	return x;
}

int   ufread4(){
	int x=0;	DWORD numDone=0;
	ReadFile(CurFile,&x,4,&numDone,0);
	return x;
}
U32 ufreadh(){
	U32 x=0;	DWORD numDone=0;
	ReadFile(CurFile,&x,4,&numDone,0);
	x = ((x & 0x000000FF) << 24) |( (x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24);
	return x;
}
float ufread4f(){
	float x=0;	DWORD numDone=0;
	ReadFile(CurFile,&x,4,&numDone,0);
	return x;
}
void* ufreadAlloc(int dataSize){
	void* p= xmalloc(dataSize);
	if(dataSize)ufread(p,dataSize);
	return p;
}
char* ufreadstr(){
	int len = ufread4();
	if(len>0)return (char*)ufreadAlloc(len);
	return null;
}

bool ufwrite(const void* data,int dataSize){
	DWORD numDone=0;
	WriteFile(CurFile,data,dataSize,&numDone,0);
	return numDone==(DWORD)dataSize;
}
void ufwrite1(char x){
	DWORD numDone;
	WriteFile(CurFile,&x,1,&numDone,0);
}
void ufwrite2(short x){
	DWORD numDone;
	WriteFile(CurFile,&x,2,&numDone,0);
}
void ufwrite4(int x){
	DWORD numDone;
	WriteFile(CurFile,&x,4,&numDone,0);
}
void ufwrite4f(float x){
	DWORD numDone;
	WriteFile(CurFile,&x,4,&numDone,0);
}
void ufwritestr(const char* string){
	DWORD numDone=0;
	int len;
	if(string){
		len = 1 + ustrlen(string);
		WriteFile(CurFile,&len,4,&numDone,0);
		WriteFile(CurFile,string,len,&numDone,0);
	}else{
		len = 0;
		WriteFile(CurFile,&len,4,&numDone,0);
	}
}

U64 ufGetDate(const char* fName){
	HANDLE f = CreateFileA(fName,0,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if(f==INVALID_HANDLE_VALUE)return 0;
	FILETIME t;
	GetFileTime(f,0,0,&t);
	CloseHandle(f);

	union{
		U64 result;
		U32 words[2];
	}u;

	u.words[0] = t.dwLowDateTime;
	u.words[1] = t.dwHighDateTime;
	return u.result;
}
void ufSetDate(const char* fName,U64 date){
	FILETIME t;
	union{
		U64 dateIn;
		U32 words[2];
	}u;

	u.dateIn = date;
	t.dwLowDateTime = u.words[0];
	t.dwHighDateTime= u.words[1];
	
	HANDLE f = CreateFileA(fName,GENERIC_WRITE| GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if(f==INVALID_HANDLE_VALUE)return;
	SetFileTime(f,0,0,&t);
	CloseHandle(f);
}
//===========================================================================================================/


typedef struct{
	const char* name;
	U32 minTime;
	U32 lastTime;
	U32 maxTime;
	double avgTime;
	double avgTime2;

	bool used;
	U32 numTimesUsed;
}BenchResult;

static U32 BenchStartTick[8],BenchStartTickIDX=0;
static ObjVector* BenchObjects=null;
static U32 _RDTSC(){
	U32 x;
	#ifdef __GNUC__
		
		asm("RDTSC; "
			"movl %%eax, %0"
				: "=r"(x) /* outputs */
				:		 /* inputs */
				: "eax", "edx" /* clobbered regs */
				);
	#else

		__asm{
			RDTSC;
			mov x,eax;
		}
	#endif
	return x;
}


void BenchStart(){
	BenchStartTick[BenchStartTickIDX] = _RDTSC();
	BenchStartTickIDX++;
	BenchStartTickIDX&=7;
}
U32 BenchEnd(const char* name){
	U32 time;
	BenchStartTickIDX--;
	BenchStartTickIDX&=7;


	time = _RDTSC()-BenchStartTick[BenchStartTickIDX];
	//if(time>3000)return time;
	if(name==null)return time;
	if(!BenchObjects)BenchObjects=new ObjVector();
	foreach(BenchObjects,BenchResult*){
		if(edx->name!=name)continue;
		edx->used=true;
		edx->numTimesUsed++;
		edx->lastTime=time;
		if(edx->minTime>time)edx->minTime=time;
		if(edx->maxTime<time)edx->maxTime=time;
		edx->avgTime+= (double)time;
		edx->avgTime2 = (edx->avgTime2*0.9999) + (time*0.0001);
		return time;
	}
	BenchResult* b = new BenchResult;
	b->name = name;
	b->minTime=time;
	b->lastTime=time;
	b->maxTime=time;
	b->avgTime=(double)time;
	b->avgTime2=(double)time;

	b->used=true;
	b->numTimesUsed=1;
	BenchObjects->add(b);
	return time;
}
void BenchPrint(){
	if(BenchObjects==null)return;
	bool oki=false;
	foreach(BenchObjects,BenchResult*){
		if(!edx->used)continue;
		if(!oki){
			prints("Bench results:");
			oki=true;
		}
		edx->avgTime/= edx->numTimesUsed;

		trace("	%s:	avg=%d/%d		min=%d	last=%d	max=%d, count=%d",edx->name,(int)edx->avgTime,(int)edx->avgTime2,edx->minTime,edx->lastTime,edx->maxTime,edx->numTimesUsed);
		edx->used=false;
		edx->numTimesUsed=0;
		edx->maxTime=0;
		edx->minTime = 0x77777777;
		edx->avgTime=0;
	}

	SetThreadAffinityMask(GetCurrentThread(),2);
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
	
}
void BenchClear(){
	if(BenchObjects==null)return;
	foreach(BenchObjects,BenchResult*){
		delete edx;
	}
	delete BenchObjects;
	BenchObjects=null;
}
void BenchBreakpoint(){
#ifdef __GNUC__
	asm("int3");
#else
	__asm{
		int 3;
	}
#endif
}


void uSysExecute(const char* pExeName,const char* params){
	SECURITY_ATTRIBUTES sat = {sizeof(SECURITY_ATTRIBUTES),0,true};
	HANDLE hRead,hWrite;
	if(!CreatePipe(&hRead,&hWrite,&sat,0)){
		print("no pipe");
		return;
	}
	STARTUPINFOA info;
	info.cb=sizeof(info);
	GetStartupInfoA(&info);
	info.hStdOutput=hWrite;
	info.hStdError=hWrite;
	info.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	info.wShowWindow=SW_HIDE;
	PROCESS_INFORMATION pinfo;
	char* cmdline = ustrmix(pExeName," ",params);
	if(!CreateProcessA(0,cmdline,0,0,1,NORMAL_PRIORITY_CLASS,0,0,&info,&pinfo)){
		prints("Error executing process:");
		prints(cmdline);
		xfree(cmdline);
		return;
	}
	xfree(cmdline);
	CloseHandle(hWrite);
	
	char tmp[512];
	
	for(;;){
		DWORD numbytes;
		if(!ReadFile(hRead,tmp,sizeof(tmp),&numbytes,0))break;
	}
	CloseHandle(hRead);
	return;
}

char* uSysExecute2(const char* pExeName,const char* params){
	SECURITY_ATTRIBUTES sat = {sizeof(SECURITY_ATTRIBUTES),0,true};
	HANDLE hRead,hWrite;
	if(!CreatePipe(&hRead,&hWrite,&sat,0)){
		print("no pipe");
		return 0;
	}
	STARTUPINFOA info;
	info.cb=sizeof(info);
	GetStartupInfoA(&info);
	info.hStdOutput=hWrite;
	info.hStdError=hWrite;
	info.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	info.wShowWindow=SW_HIDE;
	PROCESS_INFORMATION pinfo;
	char* cmdline = ustrmix(pExeName," ",params);
	if(!CreateProcessA(0,cmdline,0,0,1,NORMAL_PRIORITY_CLASS,0,0,&info,&pinfo)){
		prints("Error executing process:");
		prints(cmdline);
		xfree(cmdline);
		return 0;
	}
	xfree(cmdline);
	CloseHandle(hWrite);
	

	char* result=(char*)xmalloc(1); int resultLen=0;
	char tmp[512];
	
	for(;;){
		DWORD numbytes;
		memclear(tmp,sizeof(tmp));
		if(!ReadFile(hRead,tmp,sizeof(tmp)-1,&numbytes,0))break;
		int tmplen = ustrlen(tmp);
		result = (char*)xresize(result,resultLen+tmplen+1);
		ustrcpy(result+resultLen,tmp);
		resultLen+=tmplen;
	}
	CloseHandle(hRead);
	return result;
}



static const U32 crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,  0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

U32 memCRC32(const void* ptr,int len,U32 crc){ // from info-zip
    if(ptr==null)return 0;
	const char* buf = (const char*)ptr;
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}
#undef DO1
#undef DO2
#undef DO4
#undef DO8








CUniqueArray::CUniqueArray(int ElementSize){
	data = null;
	numElements = 0;
	elementSize = ElementSize;
	bytesAllocated = 0;
	for(int i=0;i<256;i++){
		hashes[i].indices=null;
		hashes[i].crcs=null;
		hashes[i].count=0;
		hashes[i].bytesTaken=0;
	}
}
CUniqueArray::~CUniqueArray(){
	for(int i=0;i<256;i++){
		if(!hashes[i].bytesTaken)continue;
		xfree(hashes[i].indices);
		xfree(hashes[i].crcs);
	}
	if(bytesAllocated){
		xfree(data);
	}
}
int CUniqueArray::add(const void *elementData){
	U32 crc = memCRC32(elementData,elementSize);
	int hashID = crc & 0xFF;
	int* indices = hashes[hashID].indices;
	U32* crcs = hashes[hashID].crcs;
	int count = hashes[hashID].count;
	int bytesTaken = hashes[hashID].bytesTaken;

	for(int i=0;i<count;i++){
		if(crcs[i]!=crc)continue;
		if(memcmp(elementData,data+indices[i]*elementSize,elementSize)!=0)continue;
		return indices[i]; // found duplicate
	}
	// ok, duplicate not found - so, append

	int usedSize,totalSize;
	//----[ append data into large array ]------[
	usedSize = numElements*elementSize;
	data = (char*)memAppendLg(data,usedSize,bytesAllocated,elementData,elementSize);
	//------------------------------------------/
	//----[ append hash-info to hash-table ]----------[
	usedSize = count<<2; totalSize = bytesTaken;
	indices = (int*)memAppendLg(indices,usedSize,totalSize,&numElements,4);
	usedSize = count<<2; totalSize = bytesTaken;
	crcs = (U32*)memAppendLg(crcs,usedSize,totalSize,&crc,4);
	//------------------------------------------------/
	//-----[ increment counters ]-------[
	numElements++;
	hashes[hashID].count++;
	hashes[hashID].bytesTaken=totalSize;
	hashes[hashID].indices = indices;
	hashes[hashID].crcs = crcs;
	//----------------------------------/

	return numElements-1;
}

void* CUniqueArray::stealData(){
	if(!bytesAllocated)return null;
	for(int i=0;i<256;i++){
		if(!hashes[i].bytesTaken)continue;
		xfree(hashes[i].indices);
		xfree(hashes[i].crcs);
		hashes[i].indices=null;
		hashes[i].crcs=null;
		hashes[i].count=0;
		hashes[i].bytesTaken=0;
	}
	void* ptr = xresize(data,numElements*elementSize);
	data = null;
	numElements=0;
	bytesAllocated=0;
	return ptr;
}

/*
class CTextReader{
public:
	CTextReader();
	~CTextReader();

	bool LoadFile(const char* FileName);

	char ReadChar();
	bool ExpectChar(char c);
	bool StartsWith(const char* string);
	bool StartsWithI(const char* string);
	int  ReadInt();
	float ReadFloat();
	int  ReadKeyword(char* outString,int maxSize);
	int  ReadStringWord(char* outString,int maxSize);
	void SkipWhiteSpace();
	void SkipWhiteSpaceAndCRLF();
	bool SkipToAfter(char c);

private:
	char* p;
	char* OriginalData;
};
*/




static bool ctextreader_inited=false;
static char ctextreader_toLcase[256];
static bool ctextreader_keywordStart[256];
static bool ctextreader_keywordMid[256];
static void ctextreader_init(){
	ctextreader_inited=true;
	int i;
	for(i=0;i<256;i++){
		ctextreader_toLcase[i]=(char)i;
		ctextreader_keywordStart[i]=false;
		ctextreader_keywordMid[i]=false;
	}
	for(i='A';i<='Z';i++){
		int lcase = i-'A'+'a';
		ctextreader_toLcase[i]=(char)lcase;
		ctextreader_keywordStart[i]=true;
		ctextreader_keywordMid[i]=true;
		ctextreader_keywordStart[lcase]=true;
		ctextreader_keywordMid[lcase]=true;
	}
	for(i='0';i<='9';i++){
		ctextreader_keywordMid[i]=true;
	}
	const char* nam ="_@$?";
	while(*nam){
		ctextreader_keywordStart[*nam]=true;
		ctextreader_keywordMid[*nam]=true;
		nam++;
	}
}




CTextReader::CTextReader(){
	p = null;
	OriginalData=null;
	if(!ctextreader_inited)ctextreader_init();
}

CTextReader::~CTextReader(){
}

bool CTextReader::LoadFile(const char *FileName){
	OriginalData = uffetch(FileName);	if(!OriginalData)return false;
	ustrRemoveChars(OriginalData,13);
	p = OriginalData;
	return true;
}


void CTextReader::GoBack(int count){
	p-= count;
}

char CTextReader::ReadChar(){
	return *p++;
}
bool CTextReader::ExpectChar(char c){
	if(*p != c)return false;
	p++; return true;
}
bool CTextReader::ExpectString(const char* str){
	int i;
	for(i=0;str[i];i++){
		if(str[i]!=p[i])return false;
	}
	p+= i;
	return true;
}

bool CTextReader::ExpectCharAfterBlanksAndCRLF(char expected){
	char c = *p;
	while(c==32 || c==9 || c==10){
		p++;
		c = *p;
	}
	if(c != expected)return false;
	p++; return true;
}

void CTextReader::SkipWhiteSpace(){
	char c = *p;
	while(c==32 || c==9){
		p++;
		c = *p;
	}
}

void CTextReader::SkipWhiteSpaceAndCRLF(){
	char c = *p;
	while(c==32 || c==9 || c==10){
		p++;
		c = *p;
	}
}
bool CTextReader::StartsWith(const char *string){
	int i=0;
	for(;;i++){
		char c = p[i];
		char d = string[i];
		if(d==0)break;
		if(c!=d)return false;
	}
	p+=i;
	return true;
}
bool CTextReader::StartsWithI(const char *string){
	int i=0;
	for(;;i++){
		int c = p[i];
		int d = string[i];
		c = ctextreader_toLcase[c&255];
		d = ctextreader_toLcase[d&255];
		if(c!=d)return false;
		if(c==0)break;
	}
	p+=i;
	return true;
}
int CTextReader::ReadKeyword(char *outString, int bufSize){
	U8 c = *p;
	if(!ctextreader_keywordStart[c]){
		outString[0]=0;return 0;
	}
	char* out = outString;
	char* end = &outString[bufSize-1];

	*out++ = c; p++;

	for(;;){
		U8 c = *p;
		if(!ctextreader_keywordMid[c])break;
		if(out<end)*out++ = c;
		p++;
	}
	*out =0;
	return out-outString;
}

int  CTextReader::ReadInt(){
	int sign=1,x=0;
	if(*p == '-')sign = -1;
	char c = *p;
	while(c>='0' && c<='9'){
		x = x*10 + c-'0';
		p++;
		c = *p;
	}
	return x*sign;
}
char* CTextReader::ReadLine(){
	char* start = p;

	while(*p && *p!=10)p++;
	if(*p==10){ *p=0; p++;}

	return start;
}

/* // TODO: implement
int CTextReader::ReadStringWord(char *outString, int bufSize){
	U8 c = *p;
	if(c=='"' || c=='\''){
		U8 quot = c;
	}
	
}
*/




//===========================================[[ SORTING ]]======================================================================[[

#ifndef __GNUC__
void QSortA(int* Arr,int count){
	int First,Last,StackSize;
	__asm{
		mov StackSize,1

		mov esi,Arr;
		mov First,0;
		mov eax,count;
		dec eax;
		mov Last,eax;
		align 16
	outer_loop:
		mov eax,Last;
		add eax,First;
		shr eax,1;
		mov ebx,[esi+eax*4];
		mov edi,ebx;
		mov ecx,First;
		mov edx,Last;
		align 16
	inner_loop:
		cmp [esi+ecx*4],ebx;
		jge wl2;
		inc ecx;
		jmp inner_loop;
		align 16
	wl2:
		cmp [esi+edx*4],ebx;
		jle wl2out;
		dec edx;
		jmp wl2;
	wl2out:
		cmp ecx,edx;
		jg exit_innerx;
		;------[ swap elements ]-----[
			mov eax, [esi+ecx*4]
			mov ebx, [esi+edx*4]      ; swap elements
			mov [esi+ecx*4], ebx
			mov [esi+edx*4], eax
		;----------------------------/
		mov ebx,edi;
		inc ecx;
		dec edx;
		cmp ecx,edx;
		jle inner_loop;
		align 16
	exit_innerx:
			cmp ecx,Last;
			jg iNxt;
			push ecx;
			push Last;
			inc StackSize;
			align 16
	iNxt:
			mov ebx,edi;
			mov Last,edx;
			cmp edx,First;
			jg outer_loop;

			dec StackSize
			jz qsOut;
			pop Last;
			pop First;
			mov ebx,edi;
			jmp outer_loop;
	qsOut:

	}
}


static int impl_QSortB_CompareFuncB;

static void impl_QSortB(int Arr,int count){
	int FirstB,LastB,StackSizeB;
	__asm{
		push ecx;
		push esi;
		push edi;


		mov StackSizeB,1
		
		
		mov esi,Arr;
		mov FirstB,0;
		mov eax,count;
		dec eax;
		mov LastB,eax;
		align 16
	outer_loopB:
		mov eax,LastB;
		add eax,FirstB;
		shr eax,1;
		mov ebx,[esi+eax*4];
		mov edi,ebx;
		mov ecx,FirstB;
		mov edx,LastB;
		align 16
	inner_loopB:
		;-------[ compare two cells ]--------[
		push edx
		push ecx
		push ebx
		
		push ebx
		push dword ptr [esi+ecx*4]
		call dword ptr [impl_QSortB_CompareFuncB]
		add esp,8
		pop ebx
		pop ecx
		pop edx
		test eax,eax
		;------------------------------------/
		
		jge wl2B;
		inc ecx;
		jmp inner_loopB;
		align 16
	wl2B:
		;-------[ compare two cells ]--------[
		push edx
		push ecx
		push ebx
		
		push ebx
		push dword ptr [esi+edx*4]
		call dword ptr [impl_QSortB_CompareFuncB]
		add esp,8
		pop ebx
		pop ecx
		pop edx
		test eax,eax
		;------------------------------------/
		
		jle wl2outB;
		dec edx;
		jmp wl2B;
	wl2outB:
		cmp ecx,edx;
		jg exit_innerxB;
		;------[ swap elements ]-----[
		mov eax, [esi+ecx*4]
		mov ebx, [esi+edx*4]      ; swap elements
		mov [esi+ecx*4], ebx
		mov [esi+edx*4], eax
		;----------------------------/
		mov ebx,edi;
		inc ecx;
		dec edx;
		cmp ecx,edx;
		jle inner_loopB;
		align 16
	exit_innerxB:
		cmp ecx,LastB;
		jg iNxtB;
		push ecx;
		push LastB;
		inc StackSizeB;
		align 16
			
	iNxtB:
		mov ebx,edi;
		mov LastB,edx;
		cmp edx,FirstB;
		jg outer_loopB;

		dec StackSizeB
		jz qsOut;
		pop LastB;
		pop FirstB;
		mov ebx,edi;
		jmp outer_loopB;
	qsOut:
		pop edi;
		pop esi;
		pop ecx;
	}
	return;
}


void QSortB(void* Arr,int count,const void* CompareFunc){
	impl_QSortB_CompareFuncB = (int)CompareFunc;
	if(Arr!=null && count>0) impl_QSortB((int)Arr,count);
}
#endif

//==============================================================================================================================//














//====================[[ FLOATING-POINT ]]==============================================================================[


void SSE_DenormalsAreZero_FlushToZero(){
	int sse_cr;

#ifndef __GNUC__
	__asm{
		STMXCSR sse_cr
		or sse_cr,8040h
		LDMXCSR sse_cr

		xorps xmm0,xmm0
		xorps xmm1,xmm1
		xorps xmm2,xmm2
		xorps xmm3,xmm3
		xorps xmm4,xmm4
		xorps xmm5,xmm5
		xorps xmm6,xmm6
		xorps xmm7,xmm7
	};
#endif
}



float usin_fastLo(float x){
	//always wrap input angle to -PI..PI
	if (x < -3.14159265)x += 6.28318531;
	else if (x >  3.14159265)x -= 6.28318531;

	float sinx;

	//compute sine
	if (x < 0)		sinx = 1.27323954 * x + 0.405284735 * x * x;
	else			sinx = 1.27323954 * x - 0.405284735 * x * x;

	return sinx;
}

float ucos_fastLo(float x){
	//compute cosine: sin(x + PI/2) = cos(x)
	x += 1.57079632;

	if (x >  3.14159265)x -= 6.28318531;
	else if (x < -3.14159265)x += 6.28318531;

	float cosx;

	if (x < 0)	cosx = 1.27323954 * x + 0.405284735 * x * x;
	else		cosx = 1.27323954 * x - 0.405284735 * x * x;

	return cosx;
}


float usin_fastHi(float x){
	//always wrap input angle to -PI..PI
	if (x < -3.14159265)x += 6.28318531;
	else if (x >  3.14159265)x -= 6.28318531;

	float sinx;

	//compute sine
	if (x < 0)
	{
		sinx = 1.27323954 * x + .405284735 * x * x;

		if (sinx < 0)
			sinx = .225 * (sinx *-sinx - sinx) + sinx;
		else
			sinx = .225 * (sinx * sinx - sinx) + sinx;
	}
	else
	{
		sinx = 1.27323954 * x - 0.405284735 * x * x;

		if (sinx < 0)
			sinx = .225 * (sinx *-sinx - sinx) + sinx;
		else
			sinx = .225 * (sinx * sinx - sinx) + sinx;
	}

	return sinx;
}

#ifndef __GNUC__

#include <xmmintrin.h>
void __stdcall uMultMatrix4_byMatrix4(__m128* pOut,const __m128* in1,const __m128* in2){
	__asm{
		mov eax,in2;
		mov edx,in1;
		mov ecx,pOut;
		

	movaps xmm3,[eax+16*0]
	
	
	movaps xmm4,[edx+16*0]
	movaps xmm5,[edx+16*1]
	movaps xmm6,[edx+16*2]
	movaps xmm7,[edx+16*3]
	
	
	
	

	
	//--------[ row 0 ]------------------------[
	pshufd xmm0,xmm3,0*85
	  mulps xmm0,xmm4
	pshufd xmm1,xmm3,1*85
	
	pshufd xmm2,xmm3,2*85
	  mulps xmm1,xmm5
	  mulps xmm2,xmm6
	pshufd xmm3,xmm3,3*85	
	    addps xmm0,xmm1
		mulps xmm3,xmm7
			addps xmm2,xmm3
		
				movaps xmm3,[eax+16*1]
			
			addps xmm0,xmm2
	movaps [ecx+16*0],xmm0
	//-----------------------------------------/
	
	//--------[ row 1 ]------------------------[
	pshufd xmm0,xmm3,0*85
	  mulps xmm0,xmm4
	pshufd xmm1,xmm3,1*85
	
	pshufd xmm2,xmm3,2*85
	  mulps xmm1,xmm5
	  mulps xmm2,xmm6
	pshufd xmm3,xmm3,3*85
	
	  addps xmm0,xmm1
		mulps xmm3,xmm7
		  	
			addps xmm2,xmm3
		
				movaps xmm3,[eax+16*2]
			
			addps xmm0,xmm2
	movaps [ecx+16*1],xmm0
	//-----------------------------------------/
	
	//--------[ row 0 ]------------------------[
	pshufd xmm0,xmm3,0*85
	  mulps xmm0,xmm4
	pshufd xmm1,xmm3,1*85
	
	pshufd xmm2,xmm3,2*85
	  mulps xmm1,xmm5
	  mulps xmm2,xmm6
	pshufd xmm3,xmm3,3*85
	
	    addps xmm0,xmm1
		mulps xmm3,xmm7
		  
			addps xmm2,xmm3
		
				movaps xmm3,[eax+16*3]
			
			addps xmm0,xmm2
		
		
	movaps [ecx+16*2],xmm0
	//-----------------------------------------/
	
	//--------[ row 3 ]------------------------[
	pshufd xmm0,xmm3,0*85
	  mulps xmm0,xmm4
	pshufd xmm1,xmm3,1*85
	
	pshufd xmm2,xmm3,2*85
	  mulps xmm1,xmm5
	  mulps xmm2,xmm6
	pshufd xmm3,xmm3,3*85
	
	    addps xmm0,xmm1
		mulps xmm3,xmm7
		  
		
			addps xmm2,xmm3
		
			addps xmm0,xmm2
		
		
		movaps [ecx+16*3],xmm0
	//-----------------------------------------/
	};
}

//======================================================================================================================/

#endif
