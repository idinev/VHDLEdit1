#include "keywords.h"

#define	TT_END		0
#define	TT_IDENT	'edi'  // identifier, ie "float" or "baka"
#define	TT_DOTIDENT	'tod' // struct-member, i.e ".xyz" or ".myValue"
#define	TT_NUM		'mun' // 10.1f or 17 or .13





struct TOK
{
	union {
		double	dValue;
		char*	pszValue;
	} u;
	unsigned int	eType; // can be TT_IDENT, TT_END, TT_DOTIDENT, TT_NUM, or stuff like ';' and '>>='
	GLSLKeywordID	eKeywordID; // glsl keyword if it's TT_IDENT
	unsigned char	uLen; // length of string if it's a TT_IDENT, or if it's a TT_NUM  0: integer, 1: float

	char debugType[4];
};



enum GLSLKeywordClass
{
	GLSLKEYWORD_CLASS_NOT_KEYWORD,
	GLSLKEYWORD_CLASS_NONE,
	GLSLKEYWORD_CLASS_DATATYPE,
	GLSLKEYWORD_CLASS_FUNCTION,

	GLSLKEYWORD_CLASS_SWIZZLE,

	GLSLKEYWORD_CLASS_LANG,

	GLSLKEYWORD_CLASSES_COUNT
};


struct GLSLKeywordInfo
{
	const char*			pszName;
	GLSLKeywordClass	eClass;
	int					uSubType;
	const void*			pvExtraInfo;

	int	iMinVersion;
	int	iMaxVersion;
	int iDeprVersion;
};



void InitTokenizer_OnStartup(); // call this only once per application run

TOK* Tokenize(char* pszText);
void FreeTokens(TOK* psTokens);

char* GetTokZString(const TOK* psTok);
int   GetTokZStringLen(const char* pszTok);

extern TOK* gpsCurToken;


GLSLKeywordID GetGLSLKeywordFromIdentifier(const char* pszName, int uNameLen);

//bool IsGLSLDataTypeToken(const TOK* psToken, GLSLDataType* psResult);
