#define ADV_TOKEN			ct=ct->next



enum CTOKTyp{
	// important ones first
	CTOK_END,
	CTOK_IDENT,
	CTOK_NUMBER,
	CTOK_DOT,
	CTOK_COMMA,
	CTOK_SEMICOLON,
	CTOK_COLON,
	CTOK_ASTERISK,
	CTOK_BRACEL,
	CTOK_BRACER,
	CTOK_ASSIGNVAR, // :=
	CTOK_ASSIGNRIGHT, // =>
	CTOK_OPERATOR1, // 1-char op
	CTOK_OPERATOR2, // 2-char op
	// skippable ones from now on
	CTOK_WHITE,
	CTOK_NEWLINE,
	CTOK_COMMENT,
	NUM_CTOK_TYPES
};



#define SIGFLAG_IN			1
#define SIGFLAG_OUT			2
#define SIGFLAG_BUFFER		4
#define SIGFLAG_SIGNAL		8
#define SIGFLAG_VARIABLE	16
#define SIGFLAG_GENERIC		32
#define SIGFLAG_CONST		64
#define SIGFLAG_REGISTER	128 // inferred from semantics!

#define ALL_VHDL_KEYWORDS_DEFINE K(library), K(use), K(entity), K(port), K(generic), K(in),K(out), K(inout), K(buffer), K(end), K(architecture), K(of), K(is), \
	K(signal), K(variable), K(constant), K(type), K(subtype), K(array), K(record), K(access), K(file), K(function), K(procedure), \
	K(begin), K(process), K(if), K(then), K(elsif), K(else), K(for), K(loop), K(while), K(case), K(when), K(return), K(downto), K(upto), K(wait), \
	K(work), K(package), K(body), K(component)


enum VHDLKeywords{
	VKW_NONE,
	#define K(name) VKW_##name
	ALL_VHDL_KEYWORDS_DEFINE,
	#undef K
	VKW_TOTAL_COUNT
};

struct CTok{
	const char* str;
	CTok *prev,*next;
	unsigned short len;
	unsigned char typ;
	unsigned char keyword;
	int indexInSequence; // 0 = unknown. Otherwise means Nth token from the start of the file.
	const char* origStr; // original-case string;

	bool diff(const CTok* compareTo){
		if(len != compareTo->len) return true;
		loopi(len) if(str[i] != compareTo->str[i]) return true;
		return false;
	}
};


enum eVEntityKind{
	eVEK_Invalid,
	eVEK_Signal,	// type is CSignal
	eVEK_BaseType,	// std_logic, etc. FIXME: add type
	eVEK_Subtype,	// type is CSubtype
	eVEK_Record,	// type is CTypeRecord
	eVEK_Enum,		// type is CTypeRecord
	eVEK_EnumVal,	// type is CSignal
	eVEK_Entity,	// type is CVHDLEntity
	eVEK_Architecture,	// type is CVHDLPak
	eVEK_Package,		// type is CVHDLPak
	eVEK_Function,		// type is CVHDLPak
	eVEK_Procedure,		// type is CVHDLPak
	eVEK_Process,		// type is CVHDLPak

	eVEK_UnresolvedType // probably an externally-defined type
};


class VEntity{
public:
	eVEntityKind eKind;
	CTok* name;
	VEntity* prev;
	VEntity* parent;
	CTok* tokFirst;
	CTok* tokLast;

	inline void reset(CTok* tok, eVEntityKind eKind, VEntity* parent, VEntity** linkedList){
		tokFirst = tok;
		tokLast = tok;
		name = tok;
		this->eKind = eKind;
		this->parent = parent;
		if(linkedList){
			prev = *linkedList;
			*linkedList = this;
		}else{
			prev = null;
		}
	}
};

struct TokList{
	CTok* toks;
	int numToks;
	const char* base; // lowercase
	const char* origBase; // original case
};



class CSignal : public VEntity{
	public:
	CTok* dtyp; // type of variable
	int flags; // in/out etc
};

class CSubtype : public VEntity{
	public:
	CTok* baseTyp;
	bool isArray;
};
class CTypeRecord : public VEntity{
	public:
	CSignal* members;
};
struct CStatement{
	CTok* firstTok;
	CStatement* prev;
};

class TinyAllocator{
	public:
	struct TA_AllocPage{
		TA_AllocPage* prev;
		void* padding1;
		char data[65000];
	};
	int remain;
	char* dat;

	TA_AllocPage* lastPage;

	void* slow_alloc(int alignsize);

	inline void* alloc(int size){
		int alignsize = (size+7) & ~7;
		void* res;
		if(alignsize <= remain){
			res = dat;
			dat += alignsize;
			remain -= alignsize;
		}else{
			res = slow_alloc(alignsize);
		}
		return res;
	}

	TinyAllocator(){
		remain = 0;	dat = null;	lastPage = null;
	}
	~TinyAllocator();
};



class CVHDLEntity : public VEntity{
	public:
	CSignal* generics;
	CSignal* ports;
};



class CVHDLPak : public VEntity{ // is reused for architecture, package, function, procedure, process
	public:
	CTok* rtl; // only if architecture

	CTok* args; // only if it's a function/procedure. Points to first arg tok

	CSubtype* subtypes;
	CTypeRecord* records;
	CTypeRecord* enums;
	CSignal* signals; // signals, variables, constants

	CStatement* statements; // concurrent ones for architectures

	CVHDLPak* processes; // only if is architecture
	CVHDLPak* functions; // only if is architecture/package
};




class CVHDLFile{
public:
	TinyAllocator heap;
	TokList toks;
	CVHDLPak* packages;
	CVHDLEntity* entities;
	CVHDLPak* architectures;
	CStatement* used_packages;

	CTok* ct; // current token



	void _onError(const char* desc);

	void _onError(){
		_onError("Syntax error");
	}

	static CVHDLFile* ProcessVHDLFile(const char* filename);
	CVHDLFile();
	~CVHDLFile();

	CVHDLPak* LocateLeafChildAtIndex(int sequenceIndex);

private:
	void nextTok() { do { ADV_TOKEN; } while(ct->typ >= CTOK_WHITE); }

	bool _requireTok(unsigned char typ){
		do { ADV_TOKEN; } while(ct->typ >= CTOK_WHITE);
		if(ct->typ==typ)return true;
		_onError("Syntax: required token missing");
		return false;
	}
	bool _requireKeyword(unsigned char keywordID){
		ADV_TOKEN;
		while(ct->typ >= CTOK_WHITE) ADV_TOKEN;
		if(ct->typ == CTOK_IDENT && ct->keyword==keywordID)return true;
		_onError("Required keyword missing");
		return false;
	}
	bool _skipAfterKeyword(unsigned char keywordID){
		int numBraces=0;
		for(;;){
			if(ct->keyword==keywordID && !numBraces){ nextTok(); return true;  }
			if(ct->typ==CTOK_BRACEL)numBraces++;
			if(ct->typ==CTOK_BRACER)numBraces--;
			if(ct->typ == CTOK_END) break;
			ADV_TOKEN;
		}
		_onError("Reached end of file while searching for semicolon ;");
		return false;
	}
	bool _skipAfterToken(unsigned char typ){
		int numBraces=0;
		for(;;){
			if(ct->typ==typ && !numBraces){ nextTok(); return true;  }
			if(ct->typ==CTOK_BRACEL)numBraces++;
			if(ct->typ==CTOK_BRACER)numBraces--;
			if(ct->typ == CTOK_END) break;
			ADV_TOKEN;
		}
		_onError("Reached end of file while searching for semicolon ;");
		return false;
	}




	bool BeginProcess();
	bool processKeywordEntity();
	bool processKeywordArchitecture();
	bool processSequentialFunc(CVHDLPak* func);
	bool processKeywordProcess(CVHDLPak* arch);
	bool processKeywordPackage();
	bool processKeywordVariable(VEntity* parent, CSignal** ppSigs, int BaseFlag);
	bool processKeywordSubtype(VEntity* parent,CSubtype** ppSubtyps);
	bool processKeywordType(VEntity* parent, CSubtype** ppSubtyps, CTypeRecord** ppEnums, CTypeRecord** ppRecords);
	bool processKeywordComponent();
	bool processKeywordFunction(CVHDLPak* arch, bool isProcedure);
	bool processArchAndPackageDeclarations(CVHDLPak* arch, bool isPackage);
	bool readVariables(CSignal** ppSigs, VEntity* parent, int baseFlags); // reads one line; multiple vars of the same type
	bool skipTillBraceR();


};


extern const char* g_aszVHDLKeywordString[VKW_TOTAL_COUNT];
bool Tokenize(const char* CodeString, TokList* pResult, TinyAllocator* pHeap);
void DebugPrint(const char* name, const CTok* t);
