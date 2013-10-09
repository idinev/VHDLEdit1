
#define MAX_NUM_UNDOREDO_OPERATIONS 300



class URAction{
public:
	URAction* pNextAction; // = null
	URAction* pPrevAction; // = null
	char* lpszDescription; // = null
	bool ActIsReverse; // = false

	virtual ~URAction(){}

	virtual void OnCreate(){} // if the object is complex/composite, it must do the XURM:: calls here!


	virtual void UndoRedo(bool bRedo){}
	virtual void Discard(bool bFromRedo){} // remember this:
						// discarding from "bFromRedo=true"  :   completely DESTROY object/action
						// discarding from "bFromRedo=false" : stop managing this object/action
						// if ActIsReverse=true, do vice-versa

	virtual void Compact(){} // try to compress the data, comparing with the current data

	virtual int  GetMemoryFootprint(){ return 32; }// some rough size needs to be returned
};




class UndoRedoMgr{
public:
	UndoRedoMgr();
	~UndoRedoMgr();

	void SetAsDefaultURM();

	bool CanUndo();
	bool CanRedo();

	void Undo();
	void Redo();

	URAction* NewAction(URAction* pAction);

	


	void StartBlock(char* name);
	void EndBlock();

	void ClearAll();


	//void SetupMenuItems(...); // TODO: complete
	

	static UndoRedoMgr* DefURM;

	static void AddAction(URAction* pAction){
		DefURM->NewAction(pAction);
	}

private:

	int	NumUndoGroups;
	int	NumRedoGroups;
	
	
	int	NumActionsStarted;
	URAction* pLastActionStarted;
	
	
	URAction* UndoGroups[MAX_NUM_UNDOREDO_OPERATIONS];
	URAction* RedoGroups[MAX_NUM_UNDOREDO_OPERATIONS];

	void ClearRedoChains();
	void ClearUndoChains();
	
	void DeleteOneUndoChain(URAction* pAction);	

	void Compact();
};

class XURM{
public:
	//----[ simple values ]----------------[
	static void Char(char* pChar);
	static void Short(short* pShort);
	static void Int(int* pInt);
	static void Float(float* pFloat);
	static void Double(double* pDouble);
	//-------------------------------------/
	static void Void(void* pData){ XURM::Int((int*)pData);}

	static void FixedSmallChunk(void* pChunk,int ChunkSize); // i.e ArraySize<10kB
	static void DynSmallChunk(void** ppvArray); // pointer to pointer.  This array MUST be allocated with HEAP1 


	
	static void ObjVector_AddRemove(ObjVector* pObjVector,void* ObjPtr,bool bRemove,void (*destructor)(void* pObjPtr));// this preserves the order
};

//===================================================================================================

