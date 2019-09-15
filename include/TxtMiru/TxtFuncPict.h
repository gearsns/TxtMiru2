// 以下の ifdef ブロックは DLL からのエクスポートを容易にするマクロを作成するための 
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された TXTFUNCPICT_EXPORTS
// シンボルでコンパイルされます。このシンボルは、この DLL を使うプロジェクトで定義することはできません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、 
// TXTFUNCPICT_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef TXTFUNCPICT_EXPORTS
#define TXTFUNCPICT_API __declspec(dllexport)
#else
#define TXTFUNCPICT_API __declspec(dllimport)
#endif
#include "Bitmap.h"
class cdecl CGrTxtFuncICanvas
{
public:
	CGrTxtFuncICanvas(){}
	virtual ~CGrTxtFuncICanvas(){};

	virtual bool GetBitmap(int page, int width, int height, CGrBitmap **pBmp) = 0;
};
extern "C" TXTFUNCPICT_API bool cdecl TxtFuncSavePicture(HWND hWnd, LPCTSTR ini_filename, LPCTSTR filename, int total_page, SIZE paper_size, CGrTxtFuncICanvas *pCanvas);
