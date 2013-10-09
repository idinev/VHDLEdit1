struct sSprite{
	int		wid;
	int		hei;
	HBITMAP	hBitmap;
	int*	bits;
	int*	lastLineBits;
	int		linesize;
	int		dwFlags;
	void*	zz_pNext;
};

struct SDLOCKEDRECT{
	int		x,y,wid,hei;
	int*	lpBits;
	int 	pitch; // in DWORDs !
	int		deltaX,deltaY;
	int		deltaW,deltaH;
};

struct SDSPLINE{
	POINT p0;// starting point
	POINT p1;// control-point
	POINT p2;// control-point
	POINT p3;// ending point
};

//------[ flags of sSprite ]------[
#define SDRAWSPRITE_HASALPHA	 1
#define SDRAWSPRITE_PREMULALPHA	 2
//--------------------------------/

//---[ sdPreprocess operations ]--------[
#define SDPREPR_ALPHA_FROM_COLOR	1
#define SDPREPR_PREMULTIPLY_ALPHA	2
//--------------------------------------/

enum{
	SD_COPY=0,
	SD_XOR,
	SD_ADD,
	SD_SUB,
	SD_OR,
	SD_AND,
	SD_SHR,
	SD_MUL,
	SD_ADDSAT,
	SD_SUBSAT,
	SD_SHRSAT,
	SD_SHLSAT
};

#define mRGB(R,G,B) (((R)<<16)|((G)<<8) | (B))

#ifdef __cplusplus
extern "C" {
#endif

	extern int SD_TransparentColor;
	extern RECT SDBound; // current bounding rectangle. For experts' use
	extern POINT SDDrawOffs; // current draw-offset. For experts' use
	extern HDC SDBackDC;

	//------[ system ]----------------------------------------[
	void __stdcall InitSDraw(int wid,int hei);
	void __stdcall ResizeSDrawArea(int wid,int hei);
	void __stdcall FreeSDraw();
	//--------------------------------------------------------/


	//---[ sprite operations ]--------------------------------[
	void __stdcall sdDeleteSprite(sSprite* pSprite);
	sSprite* __stdcall sdCreateBlankSprite(int wid,int hei);
	sSprite* __stdcall sdSpriteFromHBITMAP(HBITMAP hBitmap);
	sSprite* __stdcall sdSpriteFromBitmapFile(char* lpszFileName);
	sSprite* __stdcall sdSpriteFromILBFile(char* lpszFileName);
	sSprite* __stdcall sdSpriteFromILB(void* pSourceData);
	void __stdcall sdPreprocessSprite(sSprite* pSprite,int dwOperationID,int dwColor);
	//--------------------------------------------------------/


	//-------[ drawing modifiers ]-----------------------------------[
	bool __stdcall sdStart(HWND hWnd);
	void __stdcall sdEnd();
	void __stdcall sdForceUpdate();
	void __stdcall sdSetSourceSprite(sSprite* pSprite);
	void __stdcall sdSetTargetSprite(sSprite* pSprite);
	void __stdcall sdLeaveClip();
	void __stdcall sdEnterClip(int x,int y,int wid,int hei);
	void __stdcall sdForceClip(int wid,int hei);
	bool __stdcall sdLockRect(SDLOCKEDRECT* pLR,int x,int y,int wid,int hei);

	void __stdcall sdStartDrawingOnSprite(sSprite* pSprite);
	void __stdcall sdEndDrawingOnSprite();
	//---------------------------------------------------------------/


	//-----------[ bitmap-to-bitmap blends ]----------------------------------------------------[
	void __stdcall sBlt(int x,int y,int wid,int hei,int x2,int y2);
	void __stdcall sBltTrans(int x,int y,int wid,int hei,int x2,int y2);
	void __stdcall sBltTransAlpha(int x,int y,int wid,int hei,int x2,int y2,int Alpha);
	void __stdcall sBltTile(int TileX,int TileY,int TileWidth,int TileHeight,
		int SpriteX,int SpriteY,int SpriteWid,int SpriteHei,
		int StartX,int StartY);
	void __stdcall sBltAlpha(int x,int y,int wid,int hei,int x2,int y2,int Alpha);
	void __stdcall sBltAdd(int x,int y,int wid,int hei,int x2,int y2);
	void __stdcall sBltAddFade(int x,int y,int wid,int hei,int x2,int y2,int Fader);
	void __stdcall sBltTint(int x,int y,int wid,int hei,int x2,int y2,int dwColor);

	void __stdcall sBltParam(int x,int y,int wid,int hei,int x2,int y2,sSprite* pSprite);
	void __stdcall sBltTransParam(int x,int y,int wid,int hei,int x2,int y2,sSprite* pSprite);
	void __stdcall sBltTransAlphaParam(int x,int y,int wid,int hei,int x2,int y2,int Alpha,sSprite* pSprite);
	void __stdcall sBltAlphaParam(int x,int y,int wid,int hei,int x2,int y2,int Alpha,sSprite* pSprite);
	void __stdcall sBltAddParam(int x,int y,int wid,int hei,int x2,int y2,sSprite* pSprite);
	void __stdcall sBltAddFadeParam(int x,int y,int wid,int hei,int x2,int y2,int Fader,sSprite* pSprite);
	void __stdcall sBltTintParam(int x,int y,int wid,int hei,int x2,int y2,int dwColor,sSprite* pSprite);
	//------------------------------------------------------------------------------------------/

	//-----------[ draw-to-bitmap blends ]--------------------------------------------------[
	void __stdcall sDrawRect(int x,int y,int wid,int hei,int dwColor);
	void __stdcall sDrawRectAlpha(int x,int y,int wid,int hei,int dwColor,int Alpha);
	void __stdcall sDrawRectAddFade(int x,int y,int wid,int hei,int dwColor,int Alpha);
	void __stdcall sDrawRectROP(int x,int y,int wid,int hei,int dwColor,int dwROP);
	void __stdcall sdSetPixel(int x,int y,int dwColor);
	void __stdcall sdSetPixelA(int x,int y,int dwColor,int Alpha);
	int __stdcall sdGetPixel(int x,int y);
	//--------------------------------------------------------------------------------------/

	void __stdcall sDrawLine(int x,int y,int x2,int y2,int dwColor);// anti-aliased line

	void __stdcall sDrawFastLine(int x,int y,int x2,int y2,int dwColor);//  simple line, using Bresenham's algo
	void __stdcall sDrawLineCustom(int x,int y,int x2,int y2,void (*pfCallback)(),int dwWidth,int dwColor);
	void __stdcall sDrawLineH(int x,int y,int wid,int dwColor);
	void __stdcall sDrawLineV(int x,int y,int hei,int dwColor);
	void __stdcall sDrawLineH_HalfAlpha(int x,int y,int wid,int dwColor);
	void __stdcall sDrawLineV_HalfAlpha(int x,int y,int hei,int dwColor);
	void __stdcall sDrawLineHAdd(int x,int y,int wid,int dwColor);
	void __stdcall sDrawLineVAdd(int x,int y,int hei,int dwColor);

	void __stdcall sDrawBSpline(SDSPLINE* pSpline,int numPoints,int dwColor);// ; 4-point B-Spline
	void __stdcall sDrawBSplineShade(SDSPLINE* pSpline,int numPoints,int dwColor,void (*pShadeFunc)(),int ShadeWid);

	void __stdcall sDrawRectFrame(int x,int y,int wid,int hei,int dwColor);
	void __stdcall sDrawRectFrame3D(int x,int y,int wid,int hei,int dwColor,int dwLightColor,int dwDarkColor);

#ifdef __cplusplus
}
#endif
