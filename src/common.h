
struct CURTEXT{
	char* txt;
	char* basealloc;
	int allocsize;
	int numLines;
	int len;
	int cursor;
};






//--------------------------------------------------------------------------------------------------------------------
void UpdateCT();

extern POINT MousePos;
extern int FPS_Count;
extern CURTEXT ct;
