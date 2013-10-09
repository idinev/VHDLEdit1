#include <ultrano.h>
#include <UndoRedoMgr.h>


UndoRedoMgr* UndoRedoMgr::DefURM=null;


UndoRedoMgr::UndoRedoMgr(){
}
UndoRedoMgr::~UndoRedoMgr(){
	ClearAll();
	DefURM = null;
}

void UndoRedoMgr::SetAsDefaultURM(){
	DefURM = this;
}


void UndoRedoMgr::ClearRedoChains(){
	DefURM = this;

	int NumGroups = NumRedoGroups;
	NumRedoGroups = 0;

	while(NumGroups){
		NumGroups--;

		//--------------[ delete this action chain ]-----------[
		URAction* act = RedoGroups[NumGroups];

		//------[ get first action in chain ]-------[
		while(URAction* prev = act->pPrevAction) act = prev;
		//------------------------------------------/


		//-----[ forward iterate Action chain ]-------------------------------------[
		while(act){
			act->Discard(!act->ActIsReverse);
			URAction* next = act->pNextAction;
			delete act;
			act = next;
		}
		//--------------------------------------------------------------------------/
		//-----------------------------------------------------/
	}
}

void UndoRedoMgr::ClearUndoChains(){
	DefURM = this;

	int NumGroups = NumUndoGroups;
	NumUndoGroups = 0;

	while(NumGroups){
		NumGroups--;

		URAction* act;
		act = UndoGroups[NumGroups];

		//------[ get last action in chain ]--------[
		while(URAction* next = act->pNextAction){
			act = next;
		}
		//------------------------------------------/

		//-----[ backward iterate+free Action chain ]------------------------------------[
		while(act){
			act->Discard(act->ActIsReverse);
			URAction* prev = act->pPrevAction;
			delete act;
			act = prev;
		}
		//-------------------------------------------------------------------------------/
	}

}


void UndoRedoMgr::ClearAll(){
	ClearRedoChains();
	ClearUndoChains();
}


void UndoRedoMgr::DeleteOneUndoChain(URAction *pAction){
	URAction* pActionFoundIn;

	DefURM = this;


	//-----[ iterate all UndoGroups ]-----------------------------------[
	int idx=0;
	int count = NumUndoGroups;
	
	for(idx=0;idx<count;idx++){
		//------[ try to find pAction in this chain ]-----------[
		URAction*  esi = UndoGroups[idx];
		pActionFoundIn = esi; // just pressume, it's OK
		if(esi==pAction)break;

		//----[ get first member of ESI chain ]----[
		while(esi->pPrevAction)	esi = esi->pPrevAction;
		//-----------------------------------------/

		//---[ forward iterate ESI chain to search ]--[
		while(esi){
			if(esi==pAction)break; // to _found
			esi= esi->pNextAction;
		}
		//--------------------------------------------/
		if(esi==pAction)break;
	}
	if(idx==count)return; // not found

	// Found.  EAX = index

	pAction = pActionFoundIn;//  because we might've given an unsuitable parameter


	//------[ unregister from undo chains ]----------------[
	// moves chains backwards to index 0
	count--;
	NumUndoGroups = count;
	while(idx<count){
		UndoGroups[idx]=UndoGroups[idx+1];
		idx++;
	}
	//-----------------------------------------------------/


	//-----[ backward iterate+free Action chain ]-------------------------------[
	URAction* act = pAction;
	while(act){
		act->Discard(act->ActIsReverse);
		URAction* prev = act->pPrevAction;
		delete act;
		act=prev;
	}
	//--------------------------------------------------------------------------/


}



bool UndoRedoMgr::CanUndo(){
	return NumUndoGroups;
}

bool UndoRedoMgr::CanRedo(){
	return NumRedoGroups;
}

void UndoRedoMgr::Compact(){
	if(!NumUndoGroups)return;
	
	DefURM = this;
	
	URAction* act = UndoGroups[NumUndoGroups-1]; // get last undo-chain
	while(act->pNextAction)	act = act->pNextAction;

	while(act){
		act->Compact();
		act = act->pPrevAction;
	}
}




void UndoRedoMgr::StartBlock(char* name){
	URAction* pAction = new URAction();
	pAction->lpszDescription = name;

	//-----[ are there too many Undo's ? ]--------------------------[
	if(NumUndoGroups>=MAX_NUM_UNDOREDO_OPERATIONS){
		DeleteOneUndoChain(UndoGroups[0]); // the oldest "undo" chain
	}
	//--------------------------------------------------------------/

	//-----[ try to compact ]-------------[
	if(!NumActionsStarted){
		Compact();
	}
	//------------------------------------/

	//--------[ link in linked-list ]------------[
	if(NumActionsStarted){
		pAction->pPrevAction = pLastActionStarted;
		pLastActionStarted->pNextAction = pAction;
	}
	pLastActionStarted = pAction;
	NumActionsStarted++;
	//-------------------------------------------/

	//------[ send the "action created" msg ]---------------------[
	// this could change "NumActionsStarted"!!
	pAction->OnCreate();
	//------------------------------------------------------------/

}
void UndoRedoMgr::EndBlock(){
	NumActionsStarted--;
	if(NumActionsStarted)return; // if this action is not a group



	//--------[ register the UndoGroup[CurUndo] ]------[
	URAction* edx = pLastActionStarted;
	pLastActionStarted = 0;
	while(edx->pPrevAction)edx = edx->pPrevAction;
	
	UndoGroups[NumUndoGroups]=edx;
	NumUndoGroups++;
	//-------------------------------------------------/
	
	
	//--------[ delete all RedoGroups ]---------[
	ClearRedoChains();
	//------------------------------------------/
}


URAction* UndoRedoMgr::NewAction(URAction* pAction){

	//-----[ are there too many Undo's ? ]--------------------------[
	if(NumUndoGroups>=MAX_NUM_UNDOREDO_OPERATIONS){
		DeleteOneUndoChain(UndoGroups[0]); // the oldest "undo" chain
	}
	//--------------------------------------------------------------/

	//-----[ try to compact ]-------------[
	if(!NumActionsStarted){
		Compact();
	}
	//------------------------------------/

	//--------[ link in linked-list ]------------[
	if(NumActionsStarted){
		pAction->pPrevAction = pLastActionStarted;
		pLastActionStarted->pNextAction = pAction;
	}
	pLastActionStarted = pAction;
	NumActionsStarted++;
	//-------------------------------------------/

	//------[ send the "action created" msg ]---------------------[
	// this could change "NumActionsStarted"!!
	pAction->OnCreate();
	//------------------------------------------------------------/

	NumActionsStarted--;
	if(NumActionsStarted)return pAction; // if this action is not a group



	//--------[ register the UndoGroup[CurUndo] ]------[
	URAction* edx = pLastActionStarted;
	pLastActionStarted = 0;
	while(edx->pPrevAction)edx = edx->pPrevAction;
	
	UndoGroups[NumUndoGroups]=edx;
	NumUndoGroups++;
	//-------------------------------------------------/
	
	
	//--------[ delete all RedoGroups ]---------[
	ClearRedoChains();
	//------------------------------------------/

	return pAction;
}


void UndoRedoMgr::Undo(){
	if(!NumUndoGroups)return;
	DefURM = this;

	URAction* pAction;

	//---------[ get pAction chain end ]------------[
	NumUndoGroups--;
	URAction* act = UndoGroups[NumUndoGroups];
	pAction = act;

	while(act->pNextAction) act = act->pNextAction;
	//----------------------------------------------/

	//-----[ backward iterate Action chain ]----------------------------[
	while(act){
		act->UndoRedo(act->ActIsReverse);
		act = act->pPrevAction;
	}
	//------------------------------------------------------------------/


	//----[ move group to RedoGroups ]-----------------[
	RedoGroups[NumRedoGroups] = pAction;
	NumRedoGroups++;
	//-------------------------------------------------/	
}


void UndoRedoMgr::Redo(){
	if(!NumRedoGroups)return;
	DefURM = this;

	//---------[ get pAction chain start ]------------[
	NumRedoGroups--;
	URAction* pAction = RedoGroups[NumRedoGroups];
	URAction* act = pAction;

	while(act->pPrevAction)act=act->pPrevAction;
	//------------------------------------------------/

	//-----[ forward iterate Action chain ]----------------------------[
	while(act){
		act->UndoRedo(!act->ActIsReverse);
		act=act->pNextAction;
	}
	//-----------------------------------------------------------------/

	//----[ move group to UndoGroups ]-----------------[
	UndoGroups[NumUndoGroups] = pAction;
	NumUndoGroups++;
	//-------------------------------------------------/

}




//==========[[ XURM IMPLEMENTATIONS,  SIMPLE VARIABLES ]]===================================[[
#pragma region XURM_Simple

class _UndoRedoMgr_XURM_Int : public URAction{
public:
	int* pValue;
	int  prevVal;
	_UndoRedoMgr_XURM_Int(int* pVal){
		pValue = pVal;
		prevVal  = pVal[0];
	}

	void UndoRedo(bool bRedo){
		int tmp = pValue[0];
		pValue[0] = prevVal;
		prevVal = tmp;
	}
};

class _UndoRedoMgr_XURM_Short : public URAction{
public:
	short* pValue;
	short  prevVal;
	_UndoRedoMgr_XURM_Short(short* pVal){
		pValue = pVal;
		prevVal  = pVal[0];
	}
	void UndoRedo(bool bRedo){
		short tmp = pValue[0];
		pValue[0] = prevVal;
		prevVal = tmp;
	}
};

class _UndoRedoMgr_XURM_Char : public URAction{
public:
	char* pValue;
	char  prevVal;
	_UndoRedoMgr_XURM_Char(char* pVal){
		pValue = pVal;
		prevVal  = pVal[0];
	}
	void UndoRedo(bool bRedo){
		char tmp = pValue[0];
		pValue[0] = prevVal;
		prevVal = tmp;
	}

};

class _UndoRedoMgr_XURM_Double : public URAction{
public:
	double* pValue;
	double  prevVal;
	_UndoRedoMgr_XURM_Double(double* pVal){
		pValue = pVal;
		prevVal  = pVal[0];
	}
	void UndoRedo(bool bRedo){
		double tmp = pValue[0];
		pValue[0] = prevVal;
		prevVal = tmp;
	}
};


void XURM::Int(int* pVar){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_Int(pVar));
}
void XURM::Float(float* pVar){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_Int((int*)pVar));
}
void XURM::Short(short* pVar){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_Short(pVar));
}
void XURM::Char(char* pVar){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_Char(pVar));
}

void XURM::Double(double* pVar){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_Double(pVar));
}
#pragma endregion
//==========================================================================================//

//==========[[ XURM IMPLEMENTATIONS, FixedSmallArray ]]==============================[[
#pragma region XURM::FixedSmallArray
static void memxchg(void* arr1,void* arr2, int size){
	char* c1 = (char*)arr1;
	char* c2 = (char*)arr2;
	for(int i=0;i<size;i++){
		char t1 = c1[i];
		char t2 = c2[i];
		c2[i]=t1;
		c1[i]=t2;
	}
}



class _UndoRedoMgr_XURM_FixedSmallChunk : public URAction{
public:
	void* Array;
	int   Size;
	void* Prev;
	
	_UndoRedoMgr_XURM_FixedSmallChunk(void* pArray,int ArraySize){
		Array = pArray;
		Size =  ArraySize;
		Prev = memclone(pArray,ArraySize);
	}

	void UndoRedo(bool bRedo){
		// exchange memory
		memxchg(Array,Prev,Size);
	}
	
	int GetMemoryFootprint(){ 
		return 32 + Size;
	}

};


void XURM::FixedSmallChunk(void* pChunk,int ChunkSize){ // i.e ArraySize<10kB
	UndoRedoMgr::DefURM->NewAction(new _UndoRedoMgr_XURM_FixedSmallChunk(pChunk,ChunkSize));
}
#pragma endregion
//===================================================================================//




class _UndoRedoMgr_XURM_DynSmallChunk : public URAction{
public:
	void** ppvArray;
	void* Prev;
	int size; // size of Prev

	
	_UndoRedoMgr_XURM_DynSmallChunk(void** _ppvArray){
		ppvArray = _ppvArray;

		if(void *ptr = _ppvArray[0]){
			size = xmemsize(ptr);
			Prev = memclone(ptr,size);
		}
	}

	void UndoRedo(bool bRedo){
		void* tmp = ppvArray[0];
		ppvArray[0] = Prev;
		Prev = tmp;
		size = xmemsize(Prev);
	}
	
	int GetMemoryFootprint(){
		return 32 + size;
	}
	void Discard(bool bFromRedo){
		xfree(Prev);
	}
};

void XURM::DynSmallChunk(void** ppvArray){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_DynSmallChunk(ppvArray));
}

//=============================================================================================



class _UndoRedoMgr_XURM_ObjVector : public URAction{
public:

	ObjVector* pObjVector;
	void* ObjPtr;
	void (*destructor)(void* pObjPtr);
	int IndexInVector;

	_UndoRedoMgr_XURM_ObjVector(ObjVector* pObjVector,void* ObjPtr,bool bRemove,void (*destructor)(void* pObjPtr)){
		this->pObjVector = pObjVector;
		this->ObjPtr = ObjPtr;
		this->destructor = destructor;

		IndexInVector = pObjVector->num;

		ActIsReverse = bRemove;
	}

	void OnCreate(){
		UndoRedo(!ActIsReverse);
	}

	void UndoRedo(bool bRedo){
		if(bRedo){
			pObjVector->addAtIndex(ObjPtr,IndexInVector);
		}else{
			if(IndexInVector < pObjVector->num && pObjVector->data[IndexInVector]==ObjPtr){
				pObjVector->remove(IndexInVector);
			}else{
				IndexInVector = pObjVector->remove(ObjPtr);
			}
		}
	}
	void Discard(bool bFromRedo){
		if(bFromRedo){
			if(destructor) destructor(ObjPtr);
		}else{
		}
	}
};


void XURM::ObjVector_AddRemove(ObjVector* pObjVector,void* ObjPtr,bool bRemove,void (*destructor)(void* pObjPtr)){
	UndoRedoMgr::AddAction(new _UndoRedoMgr_XURM_ObjVector(pObjVector,ObjPtr,bRemove,destructor));
}
