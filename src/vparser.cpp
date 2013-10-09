
#include <windows.h>
#include "Ultrano.h"

#include "vhdlfile.h"


static const char* m_OrigESI; // original case
static const char* m_ESI; // lowercase!


const char* g_aszVHDLKeywordString[VKW_TOTAL_COUNT]={
	0,
	#define K(name) #name
	ALL_VHDL_KEYWORDS_DEFINE
	#undef K
};

static unsigned char aui8VKeywordLen[VKW_TOTAL_COUNT];
static bool bVKeywordInited = false;





void OnError(const char* desc){
	trace("error: %s",desc);
}
void OnWarning(const char* desc=null){
	if(!desc)desc = "generic";
	trace("warning: %s", desc);
}

const char* GetOriginalSourceString(const char* lowercaseVersion){
	return &m_OrigESI[lowercaseVersion - m_ESI];
}


void DebugPrint(const char* name, const CTok* t){
	trace("%s = %z",name, t->len, t->origStr);
}

static int IdentifiersDifferent(const char* var1, const char* var2, int len){
	do{
		char d = *var1 - *var2;
		if(d)return d;
		var1++; var2++;
	}while(--len);
	return 0;
}

TinyAllocator::~TinyAllocator(){
	TA_AllocPage *p = lastPage;
	while(p){
		TA_AllocPage* prev = p->prev;
		delete p;
		p = prev;
	}
}

void* TinyAllocator::slow_alloc(int alignsize){
	TA_AllocPage* p = new TA_AllocPage;
	memset(p->data, 0, sizeof(p->data));
	p->prev = lastPage;
	lastPage = p;
	remain = sizeof(p->data) - alignsize;
	dat = p->data + alignsize;
	return p->data;
}

#define reqTok(typ) 		if(!_requireTok(typ))return false
#define reqKeyword(keyw)	if(!_requireKeyword(keyw))return false
#define skipAfterSemiColon		if(!_skipAfterToken(CTOK_SEMICOLON))return false
#define skipAfterToken(typ)		if(!_skipAfterToken(typ))return false
#define skipAfterKeyword(keyw)	if(!_skipAfterKeyword(keyw))return false
#define expectCurTok(ttyp)		if(ct->typ != ttyp) do{ _onError("Current token not matching expected value"); return false; }while(0)
#define doError				do{ _onError(); return false; } while(0)
#define reqIdent 			if(!_requireTok(CTOK_IDENT))return false





void CVHDLFile::_onError(const char* desc){
	int lineNum = 1;
	loopi(toks.numToks){ if(&toks.toks[i] == ct) break; if(toks.toks[i].typ==CTOK_NEWLINE) lineNum++;}
	trace("Error On Line %d",lineNum);
	OnError(desc);
}

bool CVHDLFile::skipTillBraceR(){
	int remain = 1;
	ASSERT(ct->typ!=CTOK_BRACER);
	for(;;){
		nextTok();
		switch(ct->typ){
			case CTOK_END: _onError("Reached end while searching for )"); return false;
			case CTOK_BRACEL: remain++; break;
			case CTOK_BRACER: remain--; if(!remain){ nextTok(); return true;} break;
			default: break;
		}
	}
	doError;
	return false;
}


bool CVHDLFile::readVariables(CSignal** ppSigs, VEntity* parent, int baseFlags){
	CSignal* lastAdded;
	int numFound = 0;
	int myFlags = 0;

	while(ct->typ == CTOK_IDENT && ct->keyword==0){
		CSignal* sig = (CSignal*)heap.alloc(sizeof(CSignal));
		sig->reset(ct, eVEK_Signal, parent, (VEntity**)ppSigs);
		sig->dtyp = ct; // will be overwritten later
		sig->flags = baseFlags;
		numFound++;
		lastAdded = sig;

		nextTok();
		if(ct->typ==CTOK_COMMA) nextTok();
	}

	if(!numFound){
		doError;
	}

	expectCurTok(CTOK_COLON);
	reqIdent;

	for(;;){
		if(ct->keyword){
			bool bWasFlag = true;
			switch(ct->keyword){
				case VKW_in:	myFlags |= SIGFLAG_IN;	break;
				case VKW_out:	myFlags |= SIGFLAG_OUT;	break;
				case VKW_inout:	myFlags |= SIGFLAG_IN | SIGFLAG_OUT;	break;
				case VKW_buffer:myFlags |= SIGFLAG_BUFFER;	break;
				default: bWasFlag = false; break;
			}
			if(bWasFlag){
				reqIdent;
				continue;
			}
		}
		break;
	}

	CTok* foundTyp = ct;

	// set types of all signals we just added
	for(int remain = numFound; remain--;){
		lastAdded->dtyp = foundTyp;
		lastAdded->flags |= myFlags;

		//print(lastAdded->name);
		//print(lastAdded->dtyp);

		lastAdded = (CSignal*) lastAdded->prev;
	}

	nextTok();
	if(ct->typ == CTOK_BRACEL){
		if(!skipTillBraceR())return false;
	}
	if(ct->typ == CTOK_ASSIGNVAR){
		nextTok();
		if(ct->typ == CTOK_BRACEL){
			if(!skipTillBraceR())return false;
		}else{
			nextTok();
		}
	}

	// skip remainder of variable initialisation
	int numBracesRemain=0;
	for(;;){
		if(ct->typ==CTOK_SEMICOLON){
			if(!numBracesRemain){ nextTok(); break;}
		}else if(ct->typ==CTOK_BRACEL){
			numBracesRemain++;
		}else if(ct->typ==CTOK_BRACER){
			if(!numBracesRemain)break;
			numBracesRemain--;
		}
		nextTok();
	}

	return true;
}




bool CVHDLFile::BeginProcess(){
	ct = toks.toks;

	for(;;){
		while(ct->typ >= CTOK_WHITE) ADV_TOKEN;
		if(ct->typ == CTOK_END) break;
		expectCurTok(CTOK_IDENT);
		switch(ct->keyword){
			case VKW_library:	skipAfterSemiColon;	break;
			case VKW_use:
			{
				reqIdent;
				if(ct->keyword == VKW_work){
					reqTok(CTOK_DOT);
					reqIdent;
					CTok* useCustomLib = ct;

					bool AlreadyUsed=false;
					for(CStatement* lib=used_packages;lib; lib = lib->prev){
						if(useCustomLib->len != lib->firstTok->len)continue;
						if(useCustomLib->diff(lib->firstTok))continue;
						AlreadyUsed = true;
						break;
					}
					if(!AlreadyUsed){
						CStatement* lib = (CStatement*)heap.alloc(sizeof(CStatement));
						lib->firstTok=useCustomLib;
						lib->prev = used_packages;
						used_packages = lib;
					}
					//print(useCustomLib);
				}
				skipAfterSemiColon;
				break;
			}
			case VKW_entity:
			{
				if(!processKeywordEntity()) return false;
				break;
			}
			case VKW_architecture:
			{
				if(!processKeywordArchitecture())return false;
				break;
			}
			case VKW_package:
			{
				if(!processKeywordPackage())return false;
				break;
			}
			default: doError;
		}
	}

	return true;
}

bool CVHDLFile::processSequentialFunc(CVHDLPak* func){

	//------[ read signals, vars etc ]-----------------[
	bool bBeginReached = false;
	while(!bBeginReached){
		switch(ct->keyword){
			case VKW_begin:	bBeginReached = true; break;
			case VKW_variable:		if(!processKeywordVariable(func, &func->signals, SIGFLAG_VARIABLE)) return false;	break;
			case VKW_file:			if(!processKeywordVariable(func, &func->signals, 0)) return false;	break;
			case VKW_subtype:		if(!processKeywordSubtype (func, &func->subtypes))return false;	break;
			case VKW_type:			if(!processKeywordType(func, &func->subtypes, &func->enums, &func->records))return false;		break;
			case VKW_use:			skipAfterSemiColon; break;
			default: doError; break;
		}
	}
	//-------------------------------------------------/

	nextTok();

	//---------[ read line statements ]---------------------------------[
	bool bEndArchReached = false;
	int numEndsNeeded = 1;
	while(!bEndArchReached){

		if(ct->typ==CTOK_IDENT && ct->keyword==0){ // try to skip labels
			CTok* cur = ct;
			nextTok();
			if(ct->typ==CTOK_COLON){ // is a label
				nextTok(); // skip label
			}else{ // not a label, but a normal combinatorial statement
				CStatement* line = (CStatement*)heap.alloc(sizeof(CStatement));
				line->firstTok = cur;
				line->prev = func->statements;
				func->statements = line;
				skipAfterSemiColon;
				continue;
			}
		}
		if(ct->keyword){
			CStatement* line = (CStatement*)heap.alloc(sizeof(CStatement));
			line->firstTok = ct;
			line->prev = func->statements;
			func->statements = line;

			switch(ct->keyword){
				case VKW_if:
				{
					numEndsNeeded++;
					skipAfterKeyword(VKW_then);
					break;
				}
				case VKW_elsif:
				{
					skipAfterKeyword(VKW_then);
					break;
				}
				case VKW_for:
				case VKW_while:
				{
					numEndsNeeded++;
					skipAfterKeyword(VKW_loop);
					break;
				}
				case VKW_case:
				{
					numEndsNeeded++;
					skipAfterKeyword(VKW_is);
					break;
				}
				case VKW_loop:
				{
					numEndsNeeded++;
					nextTok();
					break;
				}
				case VKW_end:
				{
					numEndsNeeded--;
					if(!numEndsNeeded){
						bEndArchReached = true;
					}
					skipAfterSemiColon;
					break;
				}
				case VKW_else:
				{
					nextTok();
					break;
				}
				case VKW_when:
				{
					skipAfterToken(CTOK_ASSIGNRIGHT);
					break;
				}
				case VKW_wait:
				case VKW_return:
					skipAfterSemiColon;
					break;
				default:
				{
					OnWarning();
					skipAfterSemiColon;
					break;
				}
			}
		}else if(ct->typ==CTOK_END){
			doError;
		}else{
			OnWarning();
		}
	}
	//------------------------------------------------------------------/

	func->tokLast = ct;

	return true;
}

bool CVHDLFile::processKeywordProcess(CVHDLPak* arch){
	ASSERT(ct->keyword == VKW_process);
	nextTok();
	if(ct->typ==CTOK_BRACEL){
		if(!skipTillBraceR())return false;
	}
	CVHDLPak* func = (CVHDLPak*)heap.alloc(sizeof(CVHDLPak));
	func->reset(ct, eVEK_Process, arch, (VEntity**)&arch->processes);

	if(!processSequentialFunc(func))return false;

	return true;
}

bool CVHDLFile::processKeywordArchitecture(){
	ASSERT(ct->keyword == VKW_architecture);
	reqIdent;

	CVHDLPak* arch = (CVHDLPak*)heap.alloc(sizeof(CVHDLPak));
	arch->reset(ct, eVEK_Architecture, null, (VEntity**)&architectures);

	//-------[ read rtl and name ]------[
	arch->rtl = ct;
	reqKeyword(VKW_of);	reqIdent;
	arch->name = ct;
	reqKeyword(VKW_is);
	reqIdent;
	//----------------------------------/

	if(!processArchAndPackageDeclarations(arch, false))return false;

	//---------[ read processes, combinatorials ]---------------------------[
	bool bEndArchReached = false;
	while(!bEndArchReached){

		if(ct->typ==CTOK_IDENT && ct->keyword==0){ // try to skip labels
			CTok* cur = ct;
			nextTok();
			if(ct->typ==CTOK_COLON){
				nextTok(); // skip label
				if(ct->typ==CTOK_IDENT && ct->keyword==0){
					// a normal combi statement after label
					continue;
				}
			}else{
				// not a label, but a normal combinatorial statement
				skipAfterSemiColon;
				CStatement* line = (CStatement*)heap.alloc(sizeof(CStatement));
				line->firstTok = cur;
				line->prev = arch->statements;
				arch->statements = line;
				continue;
			}
		}
		if(ct->keyword){
			switch(ct->keyword){
				case VKW_process:
				{
					if(!processKeywordProcess(arch))return false;
					break;
				}
				case VKW_end:
				{
					bEndArchReached = true;
					break;
				}
				case VKW_entity:
				{
					skipAfterSemiColon;
					break;
				}
				case VKW_for: // generate statements
				case VKW_if:
				{
					int numEnd=1;
					while(numEnd){
						nextTok();
						if(ct->typ==CTOK_END)doError;
						switch(ct->keyword){
							case VKW_end: numEnd--; break;
							case VKW_if:
							case VKW_for: numEnd++; break;
							default: break;
						}
					}
					skipAfterSemiColon;
					break;
				}
				default: doError; break;
			}
		}else if(ct->typ==CTOK_END){
			doError;
		}else{
			doError;
		}
	}
	//----------------------------------------------------------------------/

	arch->tokLast = ct;
	skipAfterSemiColon;

	return true;
}

bool CVHDLFile::processKeywordEntity(){
	ASSERT(ct->keyword == VKW_entity);

	CVHDLEntity* entity = (CVHDLEntity*)heap.alloc(sizeof(CVHDLEntity));
	entity->reset(ct, eVEK_Entity, null, (VEntity**)&entities);

	reqIdent;

	entity->name = ct;

	reqKeyword(VKW_is);
	reqIdent;


	for(;;){
		if(ct->keyword==VKW_port){
			reqTok(CTOK_BRACEL);
			nextTok();
			for(;;){
				if(ct->typ==CTOK_BRACER) break;
				else if(ct->typ==CTOK_IDENT){
					if(!readVariables(&entity->ports, entity, 0))return false;
				}else doError;
			}
			ASSERT(ct->typ==CTOK_BRACER);
			reqTok(CTOK_SEMICOLON);
			nextTok();
		}else if(ct->keyword==VKW_generic){
			reqTok(CTOK_BRACEL);
			nextTok();
			for(;;){
				if(ct->typ==CTOK_BRACER) break;
				else if(ct->keyword==VKW_constant){
					reqIdent;
					if(!readVariables(&entity->generics, entity, SIGFLAG_GENERIC))return false;
				}else doError;
			}
			ASSERT(ct->typ==CTOK_BRACER);
			reqTok(CTOK_SEMICOLON);
			nextTok();
		}else if(ct->keyword==VKW_end){
			skipAfterSemiColon;
			break;
		}else{
			doError;
		}
	}
	return true;
}

bool CVHDLFile::processKeywordVariable(VEntity* parent, CSignal** ppSigs, int BaseFlag){
	reqIdent;
	if(!readVariables(ppSigs, parent, BaseFlag)){
		return false;
	}
	return true;
}

bool CVHDLFile::processKeywordComponent(){
	ASSERT(ct->keyword == VKW_component);
	do{ 	nextTok(); }	while(ct->keyword != VKW_end);
	skipAfterSemiColon;
	return true;
}

bool CVHDLFile::processKeywordFunction(CVHDLPak* arch, bool isProcedure){
	ASSERT(ct->keyword == VKW_function || ct->keyword == VKW_procedure);

	CVHDLPak* func = (CVHDLPak*)heap.alloc(sizeof(CVHDLPak));
	func->reset(ct, eVEK_Function, arch, (VEntity**) &arch->functions);

	reqIdent;

	func->name = ct;

	reqTok(CTOK_BRACEL);
	nextTok();
	if(ct->typ==CTOK_BRACER){
		nextTok();
	}else{
		func->args = ct;
		if(!skipTillBraceR())return false;
	}

	if(!isProcedure){
		if(ct->keyword != VKW_return) doError;
		reqIdent;
		nextTok();
		if(ct->typ==CTOK_BRACEL){
			if(!skipTillBraceR())return false;
		}
	}

	if(ct->typ == CTOK_SEMICOLON){
		nextTok();
	}else{
		if(ct->keyword != VKW_is) doError;
		nextTok();
		processSequentialFunc(func);
	}

	return true;
}

bool CVHDLFile::processKeywordSubtype(VEntity* parent, CSubtype** ppSubtyps){
	CTok* firstTok = ct;
	reqIdent;
	CSubtype* stype = (CSubtype*)heap.alloc(sizeof(CSubtype));
	stype->reset(firstTok, eVEK_Subtype, parent, (VEntity**)ppSubtyps); // FIXME add parent
	stype->name = ct;
	stype->baseTyp = ct;
	stype->isArray = false;
	stype->prev = *ppSubtyps;
	*ppSubtyps = stype;
	reqKeyword(VKW_is);
	reqIdent;
	stype->baseTyp = ct;
	skipAfterSemiColon;
	return true;
}

bool CVHDLFile::processKeywordType(VEntity* parent, CSubtype** ppSubtyps, CTypeRecord** ppEnums, CTypeRecord** ppRecords){
	CTok* firstTok = ct;
	VEntity* vtype = null;

	reqIdent;
	CTok* name = ct;
	reqKeyword(VKW_is);
	nextTok();
	if(ct->keyword==VKW_record){
		CTypeRecord* rec = (CTypeRecord*)heap.alloc(sizeof(CTypeRecord));
		rec->reset(firstTok, eVEK_Record, parent, (VEntity**) ppRecords);		vtype = (VEntity*) rec;
		rec->name = name;

		reqIdent;
		for(;;){
			if(ct->keyword==VKW_end) break;
			else if(ct->typ==CTOK_IDENT){
				if(!readVariables(&rec->members, parent, 0))return false;
			}else doError;
		}
		reqKeyword(VKW_record);
	}else if(ct->keyword==VKW_array){
		reqTok(CTOK_BRACEL);
		if(!skipTillBraceR())return false;
		if(ct->keyword != VKW_of) doError;
		reqIdent;

		CSubtype* stype = (CSubtype*)heap.alloc(sizeof(CSubtype));
		stype->reset(firstTok, eVEK_Subtype, parent, (VEntity**)ppSubtyps);	vtype = (VEntity*) stype;
		stype->name = name;
		stype->baseTyp = ct;
		stype->isArray = true;
	}else if(ct->keyword==VKW_access){
		reqIdent;

		CSubtype* stype = (CSubtype*)heap.alloc(sizeof(CSubtype));
		stype->reset(firstTok, eVEK_Subtype, parent, (VEntity**)ppSubtyps);	vtype = (VEntity*)stype;
		stype->name = name;
		stype->baseTyp = ct;
		stype->isArray = false;
	}else if(ct->keyword==VKW_file){
		reqKeyword(VKW_of);
		reqIdent;

		CSubtype* stype = (CSubtype*)heap.alloc(sizeof(CSubtype));
		stype->reset(firstTok, eVEK_Subtype, parent, (VEntity**)ppSubtyps);	vtype = (VEntity*) stype;
		stype->name = name;
		stype->baseTyp = ct;
		stype->isArray = false;
	}else if(ct->typ==CTOK_BRACEL){ // enum
		CTypeRecord* rec = (CTypeRecord*)heap.alloc(sizeof(CTypeRecord));
		rec->reset(firstTok, eVEK_Enum, parent,(VEntity**)ppEnums);			vtype = (VEntity*) rec;
		rec->name = name;
		rec->members = null;

		for(;;){
			nextTok();
			if(ct->typ==CTOK_IDENT){
				CSignal* enumVal = (CSignal*)heap.alloc(sizeof(CSignal));
				enumVal->reset(ct, eVEK_EnumVal, rec, (VEntity**)&rec->members);
				enumVal->name = ct;
				enumVal->dtyp = ct;
				enumVal->flags = SIGFLAG_CONST;
			}else if(ct->typ==CTOK_BRACER){
				nextTok();
				break;
			}else if(ct->typ==CTOK_COMMA){
			}else{
				doError;
			}
		}

	}else doError;
	skipAfterSemiColon;
	vtype->tokLast = ct;
	return true;
}

bool CVHDLFile::processArchAndPackageDeclarations(CVHDLPak* arch, bool isPackage){

	//------[ read signals, vars etc ]-----------------[
	bool bBeginReached = false;
	while(!bBeginReached){
		switch(ct->keyword){
			case VKW_begin:	bBeginReached = true;	if(isPackage)  doError; break;
			case VKW_end: 	bBeginReached = true; 	if(!isPackage) doError; break;

			case VKW_constant:		if(!processKeywordVariable(arch, &arch->signals, SIGFLAG_CONST))    return false; 	break;
			case VKW_signal:		if(!processKeywordVariable(arch, &arch->signals, SIGFLAG_SIGNAL))   return false; 	if(isPackage)doError;	break;
			case VKW_variable:		if(!processKeywordVariable(arch, &arch->signals, SIGFLAG_VARIABLE)) return false; 	if(isPackage)doError;	break;
			case VKW_file:			if(!processKeywordVariable(arch, &arch->signals, 0)) return false; 	if(isPackage)doError;	break;
			case VKW_subtype:		if(!processKeywordSubtype (arch, &arch->subtypes))return false;	break;
			case VKW_type:			if(!processKeywordType(arch, &arch->subtypes, &arch->enums, &arch->records))return false;		break;
			case VKW_component:		if(!processKeywordComponent())return false;		break;
			case VKW_function:		if(!processKeywordFunction(arch,false))return false;	break;
			case VKW_procedure:		if(!processKeywordFunction(arch,true))return false;		break;

			//case VKW_function: break;
			//case VKW_procedure: break;
			default: doError; break;
		}
	}
	//-------------------------------------------------/
	if(isPackage){
		skipAfterSemiColon;
	}else{
		if(ct->keyword != VKW_begin) doError;
		nextTok();
	}
	return true;
}

bool CVHDLFile::processKeywordPackage(){
	ASSERT(ct->keyword == VKW_package);
	reqIdent;
	if(ct->keyword==VKW_body){
		reqIdent;
	}

	//------[ read name ]---------------------------[
	CVHDLPak* pack = (CVHDLPak*)heap.alloc(sizeof(CVHDLPak));
	pack->reset(ct, eVEK_Package, null, (VEntity**)&packages);

	reqKeyword(VKW_is);
	reqIdent;
	//-----------------------------------------------/

	if(!processArchAndPackageDeclarations(pack, true))return false;

	return true;

}


bool Tokenize(const char* CodeString, TokList* pResult, TinyAllocator* pHeap){
	TokList res;
	int flen = strlen(CodeString);

	char* ori = new char[flen + 128]; memset(ori,0,flen + 128);
	char* low = new char[flen + 128]; memset(low,0,flen + 128);

	{ // remove char 13 (CR from CRLF) and create low/original version
		int j=0;

		loopi(flen){
			char c = CodeString[i];
			if(c!=13){
				ori[j + 64] = c;
				if(c >='A' && c <='Z') c += 32;
				low[j + 64] = c;
				j++;
			}
		}
	}

	res.numToks = 0;
	res.base = low;
	res.origBase = ori;

	m_ESI = low;
	m_OrigESI = ori;

	const char* str = low + 64;

	// setup guard-tokens. 2 "end" and 3 "newline" tokens
	CTok* prevTok = null;

	loopi(5){
		CTok* tok = (CTok*)pHeap->alloc(sizeof(CTok));
		if(!tok) return false;
		tok->typ = CTOK_NEWLINE; tok->len=1; tok->str = str; tok->origStr=str; tok->keyword = 0; tok->indexInSequence = 0;
		if(i<2)tok->typ = CTOK_END;
		if(prevTok) prevTok->next = tok;
		tok->prev = prevTok;
		prevTok = tok;
	}

	res.toks = prevTok;

	//CTok* tok = res.toks;
	int bDone = false;

	if(!bVKeywordInited){
		bVKeywordInited = true;
		for(int i=1;i<VKW_TOTAL_COUNT;i++){
			aui8VKeywordLen[i] = (unsigned char)strlen(g_aszVHDLKeywordString[i]);
		}
	}



	while(!bDone){
		const char* strBegin = str;
		CTok* t = (CTok*)pHeap->alloc(sizeof(CTok));
		if(!t){
			delete[] res.base;
			delete[] res.origBase;
			return false;
		}
		t->prev = prevTok;	prevTok->next = t;	prevTok = t; // add to linked list

		t->str = str;
		t->origStr = &ori[str - low];
		t->typ = CTOK_END;
		t->len = 1;
		t->keyword = 0;
		t->indexInSequence = t->prev->indexInSequence + 1;
		res.numToks++;

		char c = *str++;
		switch(c){
			case 0:
			{
				bDone = true;
				t->typ = CTOK_END;
				break;
			}
			case 10:
			{
				t->typ = CTOK_NEWLINE;
				break;
			}
			case 32:
			case 9:
			{
				t->typ = CTOK_WHITE;
				c = *str;
				while(c==9 || c==32){
					str++;
					c = *str;
				}
				break;
			}
			case '"':
			{
				t->typ = CTOK_NUMBER;
				c = *str;
				while(c && c!='"'){		str++; c = *str;	}
				if(c=='"')str++;
				break;
			}
			case '\'':
			{
				c = str[0];
				if(c=='0' || c=='1' || c=='z' || c=='u'){
					t->typ = CTOK_NUMBER;
					c = *str;
					while(c && c!='\'' && c!=10){	str++; c = *str;	}
					if(c=='\'')str++;
				}else{
					t->typ = CTOK_ASTERISK;
				}
				break;
			}
			case '.':
			{
				t->typ = CTOK_DOT;
				break;
			}
			case ';':
			{
				t->typ = CTOK_SEMICOLON;
				break;
			}
			case ',':
			{
				t->typ = CTOK_COMMA;
				break;
			}
			case ':':
			{
				if(str[0]=='='){
					t->typ = CTOK_ASSIGNVAR;
					str++;
				}else{
					t->typ = CTOK_COLON;
				}
				break;
			}
			case '(':
			{
				t->typ = CTOK_BRACEL;
				break;
			}
			case ')':
			{
				t->typ = CTOK_BRACER;
				break;
			}
			case '+':
			case '&':
			{
				t->typ = CTOK_OPERATOR1;
				break;
			}
			case '=':
			{
				if(str[0]=='>'){
					t->typ = CTOK_ASSIGNRIGHT;
					str++;
				}else{
					t->typ = CTOK_OPERATOR1;
				}
				break;
			}
			case '-':
			{
				if(str[0]=='-'){
					t->typ = CTOK_COMMENT;
					c = *str;
					while(c && c!=10){
						str++;
						c = *str;
					}
				}else{
					t->typ = CTOK_OPERATOR1;
				}
				break;
			}
			default:
			{
				if(c=='x' && str[0]=='"'){ //  X"7F"  hexadecimal
					t->typ = CTOK_NUMBER;
					str++;
					c = *str;
					while(c && c!=10 && c!='"'){
						str++;
						c = *str;
					}
					if(c=='"') str++;
				}else if(c>='0' && c<='9'){
					t->typ = CTOK_NUMBER;
					c = *str;
					while(c>='0' && c<='9'){
						str++; c = *str;
					}
				}else if(c>='a' && c<='z'){
					t->typ = CTOK_IDENT;
					c = *str;
					while((c>='a' && c<='z') || (c>='0' && c<='9') || c=='_'){
						str++; c = *str;
					}
					t->len = str-strBegin;
					for(int i=1;i<VKW_TOTAL_COUNT;i++){
						if(aui8VKeywordLen[i] != t->len) continue;
						if(IdentifiersDifferent(strBegin, g_aszVHDLKeywordString[i], t->len)) continue;
						t->keyword = (unsigned char)i;
						break;
					}
				}else if((c=='/' && str[0]=='=') || (c=='<' && str[0]=='=') || (c=='>' && str[0]=='=') || (c=='*' && str[0]=='*')){
					t->typ = CTOK_OPERATOR2;
					str++;
				}else{
					t->typ = CTOK_OPERATOR1;
				}
				break;
			}
		}
		int tlen = (str - strBegin);
		if(tlen < 65536){
			t->len = (unsigned short)tlen;
		}else{
			OnError("Token too long");
			delete[] res.base;
			delete[] res.origBase;
			return false;
		}
	}

	loopi(5){ // append 5 CTOK_END tokens
		CTok* t = (CTok*)pHeap->alloc(sizeof(CTok));
		t->typ = CTOK_END; t->len=1; t->str = str; t->origStr=str; t->keyword = 0; t->indexInSequence = 0;
		t->prev = prevTok;	prevTok->next = t;	prevTok = t;
	}

	res.toks = res.toks->next;

	*pResult = res;
	return true;
}

CVHDLFile::CVHDLFile(){
	ct = null;
	architectures = null; packages = null;
	used_packages = null; entities = null;
}

CVHDLFile::~CVHDLFile(){
	delete[] toks.base;
	delete[] toks.origBase;
}




CVHDLFile* CVHDLFile::ProcessVHDLFile(const char* filename){
	int flen=0;
	char* str = uffetch(filename,&flen);	if(!str)return null;


	CVHDLFile* vf = new CVHDLFile();

	if(Tokenize(str, &vf->toks, &vf->heap)){
		if(vf->BeginProcess()){
			//prints("Success. All done.");
			xfree(str);
			return vf;
		}
	}
	prints("Failed.");
	xfree(str);
	delete vf;
	return null;
}



CVHDLPak* CVHDLFile::LocateLeafChildAtIndex(int sequenceIndex){
	CVHDLPak* pak = null;
	for(CVHDLPak* p = architectures; p; p = (CVHDLPak*)p->prev){
		if(p->name->indexInSequence > sequenceIndex)continue;
		if(p->tokLast->indexInSequence < sequenceIndex)continue;
		pak = p;
		break;
	}
	if(!pak){
		for(CVHDLPak* p = packages; p; p = (CVHDLPak*)p->prev){
			if(p->name->indexInSequence > sequenceIndex)continue;
			if(p->tokLast->indexInSequence < sequenceIndex)continue;
			pak = p;
			break;
		}
	}
	if(!pak) return null; // we're outside of an architecture/package

	for(CVHDLPak* p = pak->processes; p; p = (CVHDLPak*)p->prev){
		if(p->name->indexInSequence > sequenceIndex)continue;
		if(p->tokLast->indexInSequence < sequenceIndex)continue;
		return p;
	}
	for(CVHDLPak* p = pak->functions; p; p = (CVHDLPak*)p->prev){
		if(p->name->indexInSequence > sequenceIndex)continue;
		if(p->tokLast->indexInSequence < sequenceIndex)continue;
		return p;
	}

	return pak; // not in any specific child, but in an architecture/package

}

int WINAPI WinMain3 (HINSTANCE hThisInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,	int nFunsterStil){


	CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/spi.vhd");
	//CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/common.vhd");
	//CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/pshader.vhd");
	//CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/FMAD.vhd");
	//CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/uram256.vhd");

	return 0;
}


