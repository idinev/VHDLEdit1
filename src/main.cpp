#include <windows.h>
#include <Windowsx.h>
#include "Ultrano.h"
#include "sDraw.h"
//#pragma comment(lib,"sDraw.lib")

#include "UndoRedoMgr.h"

#include "common.h"
#include "vhdlfile.h"

HWND hwndMain;

UndoRedoMgr URM;
sSprite* bmpChars;
char* g_scratch;
static char* scratch_base;
POINT MousePos;
static bool blinkCursor = true;
int FPS_Count = 0;

int FONT_WID = 8;
int FONT_HEI = 18;
CURTEXT ct={0};

CVHDLFile* vf = null;

int SettingTokenColors[NUM_CTOK_TYPES];
bool SettingTokenBold[NUM_CTOK_TYPES];
int SettingBackgroundColor = 0x293134;
int SettingCursorColor = 0xFFFFFF;
int SettingSelectionColor = 0x404e51;

POINT g_TextEditorPos = {40,40}; // in pixels
POINT g_TextEditorOffset = {0,0}; // in characters
POINT g_TextEditorSize = {100,100}; // in characters

POINT g_CursorPos = {0,0}; // in characters
CTok* g_CursorBeforeTok = null;
CTok* g_Edittok = null;

int g_NumLines = 0, g_NumRows = 0;
CTok** g_LineStarts = null;

CTok *g_SelectPrevCursor = null;
bool bSelecting = false;




ObjVector allFiles;

void DoDraw();
int GetDistanceFromLineStart(CTok* tok);

void RecalcLines(){
	int maxx = 10;
	int y = 0;
	int x = 0;

	for(CTok* t = vf->toks.toks; t->typ!=CTOK_END; t = t->next){
		switch(t->typ){
			case CTOK_NEWLINE: y++; break;
			case CTOK_WHITE:
				loopi(t->len){
					if(t->str[i]!=9) x++;
					else x = (x + 4) & ~3;
				}
				break;
			default:	x+= t->len; break;
		}
		if(maxx<x) maxx=x;
	}
	maxx++;
	y++;
	if(g_NumLines < y){
		if(g_LineStarts) delete[] g_LineStarts;
		g_LineStarts = new CTok*[y];
	}
	g_NumRows = maxx;
	g_NumLines = y;
	y=0;
	g_LineStarts[0] = vf->toks.toks;
	for(CTok* t = g_LineStarts[0]; t->typ!=CTOK_END; t = t->next){
		if(t->typ != CTOK_NEWLINE)continue;
		y++;
		g_LineStarts[y] = t->next;
	}

}

void RecalcCursorPos(){
	int y = 0;
	g_CursorPos.x = 0;
	g_CursorPos.y = 0;
	for(CTok* t = vf->toks.toks; t->typ!=CTOK_END; t = t->next){
		if(t==g_CursorBeforeTok)break;
		if(t->typ==CTOK_NEWLINE) y++;
	}
	g_CursorPos.y = y;

	g_CursorPos.x = GetDistanceFromLineStart(g_CursorBeforeTok);
}

void ScrollToCursor(){
	int y = g_CursorPos.y;

	if(g_TextEditorOffset.y+3 > y){
		g_TextEditorOffset.y = y - 3;
	}else if(g_TextEditorOffset.y + g_TextEditorSize.y - 3 < y){
		g_TextEditorOffset.y = y + 3 - g_TextEditorSize.y;
	}
	if(g_TextEditorOffset.y + g_TextEditorSize.y > g_NumLines){
		g_TextEditorOffset.y = g_NumLines - g_TextEditorSize.y;
	}
	if(g_TextEditorOffset.y < 0) g_TextEditorOffset.y = 0;
}

static bool HitsRect(int x,int y,  int rx,int ry,int rwid,int rhei){
	if(x<rx || y<ry)return false;
	if(x>=rx+rwid || y>=ry+rhei)return false;
	return true;
}



static void InitTokenColors(){
	loopi(NUM_CTOK_TYPES){
		int col;
		bool bold = false;
		switch(i){
			case CTOK_COMMENT:	col = 0x6494AA; break;
			case CTOK_ASTERISK: col = 0xFF0000; break;
			case CTOK_NUMBER:	col = 0xffcd22; break;
			case CTOK_WHITE:
			case CTOK_SEMICOLON:
				col = 0x93c763; bold = true; break;
			case CTOK_COLON:
			case CTOK_ASSIGNVAR:
			case CTOK_ASSIGNRIGHT:
			case CTOK_DOT:
			case CTOK_OPERATOR1:
			case CTOK_OPERATOR2:
				col = 0xe8e2b7; break;
			default: col = 0xF0F2F4;	break;
		}
		SettingTokenColors[i] = col;
		SettingTokenBold[i] = bold;
	}
}










int sDrawText(int x,int y, const char* txt, int len, int color){
	while(len--){
		char c = *txt++;
		if(c > 32 && c < 127){
			//sBlt(x,y,FONT_WID,FONT_HEI, (c-32)*FONT_WID + 0, 0);
			sBltTint(x,y,FONT_WID,FONT_HEI, (c-32)*FONT_WID + 0, 0, color);
			x+= FONT_WID;
		}else if(c==9){
			x += FONT_WID * 4;
		}else if(c==10 || c==13){
		}else{
			x+= FONT_WID;
		}
	}
	return x;
}


U32 lastBlinkCursorTime = 0;

CTok* GetTokenAtXY(int x,int y, bool allowMishit){
	x -= g_TextEditorPos.x;	y -= g_TextEditorPos.y;
	x += g_TextEditorOffset.x * FONT_WID;
	y += g_TextEditorOffset.y * FONT_HEI;
	if(x <0 || y <0) return null;
	int subcharX = x;
	x/=FONT_WID;	y/=FONT_HEI;
	if(y >=g_NumLines)return null;


	int curLine = y;

	CTok* t = g_LineStarts[curLine];

	int xspace = 0;
	int sx = x;

	for(; t->typ != CTOK_END && t->typ != CTOK_NEWLINE; t = t->next){
		if(t->typ != CTOK_WHITE){
			sx -= t->len;
			xspace += t->len;
		}else{
			loopi(t->len){
				if(t->str[i]!=9){
					sx--;
					xspace++;
				}else{
					int next_xspace = (xspace + 4) & ~3;
					sx -= (next_xspace - xspace);
					xspace = next_xspace;
				}

			}
		}
		if(sx < 0){
			//if(t!= g_LineStarts[curLine]) t = t->prev;
			break;
		}
	}


	if(allowMishit){
		int diff1 = GetDistanceFromLineStart(t)*FONT_WID - subcharX;			if(diff1 < 0) diff1 = -diff1;
		int diff2 = GetDistanceFromLineStart(t->next)*FONT_WID - subcharX;		if(diff2 < 0) diff2 = -diff2;

		if(diff2 < diff1) t = t->next;
		return t;
	}

	if(t->typ != CTOK_END && t->typ != CTOK_NEWLINE){
		return t;
	}

	return null;
}

enum ESearchResult{
	eSRSignal,
	eSRRecord,
	eSREnum,
	eSRSubtype,
	eSRFunc
};

void OnFoundHoverResult(CTok* t, ESearchResult kind){
	while(t->prev->typ != CTOK_NEWLINE) t = t->prev;
	while(t->typ == CTOK_WHITE) t = t->next;
	CTok* line = t;
	int len = 0;
	while(t->typ != CTOK_NEWLINE && t->typ != CTOK_END){ len += t->len; t = t->next;}
	if(t->typ == CTOK_NEWLINE) t = t->prev;

	char* txtFull = new char[len + 1];
	char* s = txtFull;
	t = line;
	while(t->typ != CTOK_NEWLINE && t->typ != CTOK_END){ memcpy(s, t->origStr, t->len); s += t->len; t = t->next;}
	s[0] = 0;

	prints(txtFull);
	delete[] txtFull;

}



void FindTokenStuff(CTok* t, CVHDLPak* where, bool InCurrentArch, int* pNumFound){
	int indexInSeq = t->indexInSequence;


	if(where->processes) FindTokenStuff(t, where->processes, true, pNumFound);
	if(where->functions) FindTokenStuff(t, where->functions, true, pNumFound);

	for(CVHDLPak* arch = where; arch ; arch = (CVHDLPak*) arch->prev){
		if(InCurrentArch){
			if(arch->tokFirst->indexInSequence > indexInSeq)continue;
			if(arch->tokLast->indexInSequence < indexInSeq)continue;
		}

		// is it a signal?



	}
}

int GetDistanceFromLineStart(CTok* tok){
	int dist = 0;
	CTok* t = tok;
	while(t->indexInSequence > 1 && t->prev->typ != CTOK_NEWLINE) t = t->prev;
	while(t != tok){
		if(t->typ==CTOK_WHITE){
			loopi(t->len){
				if(t->str[i]!=9)dist++;
				else{
					dist = (dist + 4) & ~3;
				}
			}
		}else dist += t->len;
		t = t->next;
	}
	return dist;
}

void OnKeyDown(int key,int l){
	bool bRedraw = false;

	if(key==VK_ESCAPE)PostQuitMessage(0);
	else if(key==VK_DELETE){

	}else if(key=='Z'){
		URM.Undo();
	}else if(key=='Y'){
		URM.Redo();
	}


	switch(key){
		case VK_LEFT:
			if(g_CursorBeforeTok && g_CursorBeforeTok->indexInSequence > 1) {
				g_CursorBeforeTok = g_CursorBeforeTok->prev;
				if(g_CursorBeforeTok->typ ==CTOK_WHITE && g_CursorBeforeTok->prev->typ!=CTOK_NEWLINE){
					g_CursorBeforeTok = g_CursorBeforeTok->prev;
				}
				g_SelectPrevCursor = g_CursorBeforeTok;
				bRedraw = true;	blinkCursor = true;
				ASSERT(g_CursorBeforeTok);
			}
			break;
		case VK_RIGHT:
			if(g_CursorBeforeTok && g_CursorBeforeTok->typ != CTOK_END) {
				g_CursorBeforeTok = g_CursorBeforeTok->next;
				if(g_CursorBeforeTok->typ ==CTOK_WHITE){
					g_CursorBeforeTok = g_CursorBeforeTok->next;
				}
				g_SelectPrevCursor = g_CursorBeforeTok;
				bRedraw = true;	blinkCursor = true;
				ASSERT(g_CursorBeforeTok);
			}
			break;
		case VK_UP:
		case VK_DOWN:
		case VK_NEXT:
		case VK_PRIOR:
			if(g_CursorBeforeTok){
				CTok* t = g_CursorBeforeTok;
				int dist = GetDistanceFromLineStart(t);
				int iline = g_CursorPos.y;
				switch(key){
				case VK_DOWN:	iline++; break;
				case VK_UP:		iline--; break;
				case VK_NEXT:	iline+= g_TextEditorSize.y > 4 ?  g_TextEditorSize.y - 4 : 4; break;
				default:		iline-= g_TextEditorSize.y > 4 ?  g_TextEditorSize.y - 4 : 4; break;
				}
				if(iline >= g_NumLines){
					iline = g_NumLines - 1;
					t = g_LineStarts[iline];
					while(t->typ != CTOK_END) t = t->next;
				}else if(iline < 0){
					t = g_LineStarts[0];
					while(t->prev->typ != CTOK_NEWLINE) t = t->prev;
				}else{
					t = g_LineStarts[iline];
					if(t->typ != CTOK_END){
						int maxiter = dist + 10;
						while((t->typ != CTOK_NEWLINE) && (t->typ!=CTOK_END) && (maxiter > 0)){
							maxiter--;
							int d = GetDistanceFromLineStart(t->next);
							if(d > dist)break;
							t = t->next;
						}
						//if(t->typ==CTOK_WHITE) t = t->next;
					}
				}

				if(t && g_CursorBeforeTok != t){
					g_CursorBeforeTok = t;
					bRedraw = true;	blinkCursor = true;
				}
			}
			break;
		case VK_F12:
			BenchPrint();
			break;
		default: break;
	}
	if(bRedraw)	{
		g_SelectPrevCursor = g_CursorBeforeTok;
		RecalcCursorPos();
		ScrollToCursor();
		//print((int)g_CursorPos.y);
		InvalidateRect(hwndMain,0,0);
		//DoDraw();
	}
}


void OnLButtonDown(int x, int y,int mod){


	SetCapture(hwndMain);
	//if(!MouseFocusedMod)return;

	CTok* t = GetTokenAtXY(x,y, true);
	if(!t)return;

	{
		g_CursorBeforeTok = t;
		if(!(mod & MK_SHIFT)){
			g_SelectPrevCursor = t;
		}
		RecalcCursorPos();
		ScrollToCursor();
		blinkCursor=true;
		DoDraw();

	}
	bSelecting = true;

	//trace("Hover on %z",t->len,t->str);

	int numFound = 0;

#if 0
	for(;;){
		FindTokenStuff(t, vf->architectures, true, &numFound);	if(numFound) break;
		break;

	}
#endif

	CVHDLPak* child = vf->LocateLeafChildAtIndex(t->indexInSequence);
	for(CVHDLPak* pak = child; pak; pak = (CVHDLPak*)pak->parent){
		//trace("Searching among %z", pak->name->len, vf->GetOriginalSourceString(pak->name->str));
		#define SEARCH_AMONG(rtype, place, kind)	for(rtype* rname = place; rname; rname = (rtype*)rname->prev){ \
													if(rname->name->diff(t))continue; \
													OnFoundHoverResult(rname->name, kind);\
													numFound++;\
												}

		SEARCH_AMONG(CSignal, pak->signals, eSRSignal)
		SEARCH_AMONG(CTypeRecord, pak->records, eSRRecord)
		SEARCH_AMONG(CTypeRecord, pak->enums, eSREnum)
		SEARCH_AMONG(CSubtype, pak->subtypes, eSRSubtype)
	}

	for(CVHDLPak* pak = child; pak; pak = (CVHDLPak*)pak->parent){
		if(pak->eKind != eVEK_Architecture) continue;
		for(CVHDLEntity* ent = vf->entities; ent; ent = (CVHDLEntity*)ent->prev){
			if(pak->name->diff(ent->name))continue;
			SEARCH_AMONG(CSignal, ent->ports, eSRSignal)
			SEARCH_AMONG(CSignal, ent->generics, eSRSignal)
		}
	}



}

void OnMouseMove(int x, int y){
	if(bSelecting){
		CTok* t  = GetTokenAtXY(x,y, true);
		if(!t)return;
		if(g_CursorBeforeTok != t){
			g_CursorBeforeTok = t;
			RecalcCursorPos();
			ScrollToCursor();
			blinkCursor=true;
			InvalidateRect(hwndMain,0,0);
		}
	}
}

void OnLButtonUp(){
	ReleaseCapture();

	bSelecting = false;


}

void DoDraw(){
	if(!sdStart(hwndMain))return;	// always return if failed! sdStart fails when the window is not visible


	BenchStart();
	sdSetSourceSprite(bmpChars);
	sDrawRect(0,0,4000,4000,SettingBackgroundColor);

	sdEnterClip(0,g_TextEditorPos.y,g_TextEditorPos.x,4000);
	sDrawRect(0,0,g_TextEditorPos.x,4000, 0x3f4b4e);
	loopi(g_TextEditorSize.y){
		char str[10];
		uitoa(str,i+g_TextEditorOffset.y+1);
		if(i+g_TextEditorOffset.y == g_CursorPos.y){
			sDrawRect(0,i*FONT_HEI,4000,FONT_HEI, 0x2f393c);
		}
		sDrawText(3,i*FONT_HEI,str,strlen(str),0x81969a);
	}
	sdLeaveClip();


	sdEnterClip(g_TextEditorPos.x,g_TextEditorPos.y,4000,4000);

	sDrawRect(0,(g_CursorPos.y - g_TextEditorOffset.y) * FONT_HEI,4000,FONT_HEI,0x2f393c); // hilite line under cursor
	if(blinkCursor){
		sDrawRect(g_CursorPos.x*FONT_WID,(g_CursorPos.y - g_TextEditorOffset.y)*FONT_HEI ,2,FONT_HEI,SettingCursorColor);
	}

	int basex = - g_TextEditorOffset.x * FONT_WID;
	int x = basex;
	int y = 0;

	int xspace = 0;
	CTok* t = g_LineStarts[g_TextEditorOffset.y];
	bool bReachedEnd = false;
	bool bInSelection = false;

	if((g_CursorBeforeTok != g_SelectPrevCursor) && g_CursorBeforeTok && g_SelectPrevCursor){
		if( (g_CursorBeforeTok->indexInSequence < t->indexInSequence) ||
			(g_SelectPrevCursor->indexInSequence < t->indexInSequence)){
			if(!( (g_CursorBeforeTok->indexInSequence < t->indexInSequence) &&
				  (g_SelectPrevCursor->indexInSequence < t->indexInSequence))){
				/* If there's any selection, and only one of its ends is before this line,
				 * then we're still in selection */
				bInSelection = true;
			}
		}
	}

	for(;!bReachedEnd;t = t->next){
		if((t == g_CursorBeforeTok || t==g_SelectPrevCursor) && (g_CursorBeforeTok != g_SelectPrevCursor))   bInSelection = !bInSelection;

		switch(t->typ){
			case CTOK_END:	bReachedEnd = true; break;
			case CTOK_NEWLINE:
				y += FONT_HEI;
				x = basex;
				xspace = 0;
				break;
			case CTOK_WHITE:
			{
				int prevx = x;
				loopi(t->len){
					if(t->str[i] != 9){
						xspace++;
						x += FONT_WID;
					}else{
						int next_xspace = (xspace + 4) & ~3;
						x += FONT_WID * (next_xspace - xspace);
						xspace = next_xspace;
					}
				}
				if(bInSelection) sDrawRect(prevx,y,x-prevx,FONT_HEI,SettingSelectionColor);
				break;
			}
			default:
				if(t != g_Edittok){
					if(bInSelection) sDrawRect(x,y,t->len*FONT_WID,FONT_HEI,SettingSelectionColor);
					int col = SettingTokenColors[t->typ];
					bool bold = SettingTokenBold[t->typ];
					if(t->keyword){
						col = SettingTokenColors[CTOK_WHITE];
						bold = SettingTokenBold[CTOK_WHITE];
					}

					if(bold){ // make it bold
						sDrawText(x+1,y,t->origStr,t->len,col);
					}
					int nextx = sDrawText(x,y,t->origStr,t->len,col);
					x = nextx;
					xspace += t->len;
				}else{
					int col = SettingTokenColors[t->prev->typ];
					loopi(t->len){
						if(t->str[i]!=9){
							x = sDrawText(x,y,t->origStr + i, 1,col);
							xspace++;
						}else{
							int next_xspace = (xspace + 4) & ~3;
							x += FONT_WID * (next_xspace - xspace);
							xspace = next_xspace;
						}
					}
				}
				break;
		}

	}
	sdLeaveClip();

	sDrawRect(0,SDBound.bottom-1,4000,10,0xEEEEEE); // somehow dragging creates a smudge temporarily

	BenchEnd("DoDraw");


	BenchStart();
	sdEnd();

	BenchEnd("sDraw sdEnd()");
}



void OnDot(){
	CTok* line = g_Edittok;
	while(line->prev->typ != CTOK_NEWLINE) line= line->prev;
	CTok* t = line;
	int bytes = 0;
	while(t != g_Edittok->next){
		bytes += t->len;
		t = t->next;
	}


	char* fullStr = new char[bytes+1];
	char* s = fullStr;
	for(t=line; t != g_Edittok->next; t= t->next){
		loopi(t->len) *s++ = t->str[i];
	}
	*s=0;

	TokList tlist;
	TinyAllocator* alloc=new TinyAllocator();
	if(!Tokenize(fullStr,&tlist,alloc)){
		delete alloc;
		delete[] fullStr;
		return;
	}
	t = tlist.toks;
	while(t->next->typ != CTOK_END) t = t->next;
	while(t->prev->typ==CTOK_IDENT || t->prev->typ==CTOK_DOT){
		t = t->prev;
	}

	if(t->typ==CTOK_IDENT){
		CTok* dtype = null;
		CTypeRecord* drec = null;

		CVHDLPak* child = vf->LocateLeafChildAtIndex(g_Edittok->indexInSequence);
		for(CVHDLPak* pak = child; pak && !dtype; pak = (CVHDLPak*)pak->parent){
			for(CSignal* sig = pak->signals; sig; sig = (CSignal*) sig->prev){
				if(sig->name->diff(t))continue;
				if(!dtype) dtype = sig->dtyp;
				break;
			}
		}

		if(dtype){
			for(CVHDLPak* pak = child; pak && !drec; pak = (CVHDLPak*)pak->parent){
				for(CTypeRecord* rec = pak->records; rec; rec = (CTypeRecord*) rec->prev){
					if(rec->name->diff(dtype))continue;
					if(!drec) drec = rec;
					break;
				}
			}
			if(!drec){
				foreach(&allFiles,CVHDLFile*){
					for(CVHDLPak* p = edx->packages; p ; p = (CVHDLPak*) p->prev){
						for(CTypeRecord* r = p->records; r; r = (CTypeRecord*)r->prev){
							if(r->name->diff(dtype))continue;
							if(!drec) drec = r;
							break;
						}
					}
				}
			}
		}

		if(drec){
			for(CSignal* sig = drec->members; sig; sig = (CSignal*) sig->prev){
				trace("found %z",sig->name->len, sig->name->origStr);
			}
		}
	}

	delete alloc;
	delete[] tlist.base;
	delete[] tlist.origBase;
	delete[] fullStr;
}

void OnChar(char c,LPARAM L){
	bool updated=false;
	if(!g_CursorBeforeTok)return;

	if(c==9 || (c>= 32 && c <= 126)){
		if(!g_Edittok){
			g_Edittok = new CTok;
			g_Edittok->keyword=0;
			g_Edittok->str = null;
			g_Edittok->origStr = null;
			g_Edittok->len = 0;
			g_Edittok->typ = CTOK_IDENT;
			g_Edittok->next = g_CursorBeforeTok;
			g_Edittok->prev = g_CursorBeforeTok->prev;
			g_Edittok->prev->next = g_Edittok;
			g_Edittok->next->prev = g_Edittok;
			g_Edittok->indexInSequence = g_CursorBeforeTok->indexInSequence;
			for(CTok* t = g_CursorBeforeTok; t->typ != CTOK_END; t = t->next){
				t->indexInSequence++;
			}
			//g_CursorBeforeTok = g_Edittok;
		}

		char* s = (char*)xresize((void*)g_Edittok->str,g_Edittok->len+1);
		char* o = (char*)xresize((void*)g_Edittok->origStr,g_Edittok->len+1);
		s[g_Edittok->len] = (c>='A' && c<='Z') ? c+32 : c;
		o[g_Edittok->len] = c;
		g_Edittok->len++;
		g_Edittok->str = s;
		g_Edittok->origStr = o;
		updated = true;

		if(g_Edittok->prev->typ == CTOK_NEWLINE) RecalcLines();

		if(c=='.'){
			OnDot();
		}
	}else if(c==8){ // backspace
		if(g_Edittok){
			if(g_Edittok->len) g_Edittok->len--;
			else{
				for(CTok* t = g_Edittok; t->typ != CTOK_END;t=t->next) t->indexInSequence--;
				g_Edittok->next->prev = g_Edittok->prev;
				g_Edittok->prev->next = g_Edittok->next;

				if(g_CursorBeforeTok==g_Edittok) g_CursorBeforeTok = g_Edittok->next;
				xfree((void*)g_Edittok->str);
				xfree((void*)g_Edittok->origStr);
				delete g_Edittok;
				g_Edittok = null;
			}
		}else{
			CTok* t = g_CursorBeforeTok->prev;
			if(t->indexInSequence > 0){
				t->prev->next = t->next;
				t->next->prev = t->prev;
				// mark it as unusable, so that we crash if we ever use it
				t->prev = null;
				t->next = null;
				t->str = null;
				t->origStr = null;
				t->typ = CTOK_IDENT;
				t->len = 1;
				t->keyword = 0;
			}
		}
		RecalcLines();
		updated=true;
	}

	if(updated){
		RecalcCursorPos();
		ScrollToCursor();
		blinkCursor=true;
		DoDraw();
	}
}

void OnSize(int x,int y){
	if(x < 100) x= 100;
	if(y < 100) y= 100;
	g_TextEditorSize.x = (x - g_TextEditorPos.x) / FONT_WID;
	g_TextEditorSize.y = (y - g_TextEditorPos.y) / FONT_HEI;

	if(g_TextEditorOffset.y + g_TextEditorSize.y >= g_NumLines) g_TextEditorOffset.y -= g_TextEditorSize.y;
	if(g_TextEditorOffset.x + g_TextEditorSize.x >= g_NumRows)  g_TextEditorOffset.x -= g_TextEditorSize.x;
	if(g_TextEditorOffset.x < 0 ) g_TextEditorOffset.x = 0;
	if(g_TextEditorOffset.y < 0 ) g_TextEditorOffset.y = 0;
}

void OnMouseWheel(int dy){
	int y = g_TextEditorOffset.y;
	int scrollSpeed = 5;

	dy = dy > 0 ? scrollSpeed : -scrollSpeed;
	y -= dy;
	if(y +g_TextEditorSize.y > g_NumLines) y = g_NumLines - g_TextEditorSize.y;
	if(y < 0) y = 0;
	g_TextEditorOffset.y = y;
	InvalidateRect(hwndMain,0,0);
}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l){

	switch(msg){
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:

		DoDraw();	// draw the window
		ValidateRect(hwnd,0); // Note, we should validate.
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(l), GET_Y_LPARAM(l));
		//DoDraw(); // let's redraw the window
		break;
	case WM_TIMER:
		DoDraw();
		blinkCursor = !blinkCursor;
		break;
	case WM_KEYDOWN:
		OnKeyDown(w,l);
		break;
	case WM_CHAR:
		OnChar((char)w,l);
		//DoDraw();
		break;
	case WM_SIZE:
		OnSize(LOWORD(l),HIWORD(l));
		break;
	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(l), GET_Y_LPARAM(l),w);
		DoDraw();
		break;
	case WM_MOUSEWHEEL:
		OnMouseWheel(((int)w) >> 16);
		break;
	case WM_LBUTTONUP:
		OnLButtonUp();
		DoDraw();
		break;
	default:
		return DefWindowProc(hwnd,msg,w,l);
	}
	return 0;
}


void MakeMainWindow(long wid,long hei,char* lpszTitle,DWORD dwStyle){
	WNDCLASSEX wc;
	memclear(&wc,sizeof(wc));
	wc.cbSize = sizeof (WNDCLASSEX);
	wc.hInstance = GetModuleHandle(0);
	wc.lpszClassName = "GabEdit_MainWndCls";
	wc.lpfnWndProc = MainWndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	RegisterClassEx(&wc);

	
	hwndMain = CreateWindowEx(0,wc.lpszClassName,lpszTitle,dwStyle,CW_USEDEFAULT,CW_USEDEFAULT,wid,hei,0,NULL,wc.hInstance,NULL);
}


void UpdateCT(){
	ct.len = ustrlen(ct.txt);
	if(ct.cursor < 0) ct.cursor = 0;
	else if(ct.cursor > ct.len) ct.cursor = ct.len;
}

int WINAPI WinMain (HINSTANCE hThisInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,	int nFunsterStil){
	DebugClear();

	URM.SetAsDefaultURM();
	InitSDraw(0,0);
	//------[ load some images ]-----------------------[
	//pBack = sdSpriteFromILBFile("../Media/back.ilb");
	//pBall = sdSpriteFromILBFile("../Media/ball.ilb");
	//-------------------------------------------------/
	bmpChars = sdSpriteFromBitmapFile("src/alphas.bmp");
	sdPreprocessSprite(bmpChars,SDPREPR_ALPHA_FROM_COLOR,-1);
	sdSetSourceSprite(bmpChars);

	scratch_base = new char[10000000];
	memclear(scratch_base,4096);
	g_scratch = &scratch_base[1000];
	ct.allocsize = 16384;
	ct.basealloc = new char[ct.allocsize];
	ct.txt = ct.basealloc + 1024;
	ct.numLines = 2;

	strcpy(ct.txt,"hello world\n	two");
	UpdateCT();
	InitTokenColors();





	MakeMainWindow(1400,800,"VEdit",(WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_VISIBLE | WS_SYSMENU));



	//char asci[256]={0};	for(int i=32;i<127;i++)asci[i-32] = i;	prints(asci);




		BenchStart();


	vf = CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/spi.vhd");
	if(!vf){
		return -1;
	}
	g_CursorBeforeTok = vf->toks.toks;
	g_SelectPrevCursor = g_CursorBeforeTok;
	RecalcLines();
	RecalcCursorPos();

	allFiles.add(vf);
	{
		CVHDLFile* common = CVHDLFile::ProcessVHDLFile("D:/ULTRANO/_TOYS/RandomFpga/src/common.vhd");
		if(common) allFiles.add(common);
		else prints("ERROR loading common");
	}



	BenchPrint();


	
	SetTimer(hwndMain, 1,200,0);
	
	MSG messages;
	while(GetMessage(&messages,0,0,0)){
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
	DestroyWindow(hwndMain);
	FreeSDraw();
	return 0;
}



