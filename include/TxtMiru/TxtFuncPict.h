// �ȉ��� ifdef �u���b�N�� DLL ����̃G�N�X�|�[�g��e�Ղɂ���}�N�����쐬���邽�߂� 
// ��ʓI�ȕ��@�ł��B���� DLL ���̂��ׂẴt�@�C���́A�R�}���h ���C���Œ�`���ꂽ TXTFUNCPICT_EXPORTS
// �V���{���ŃR���p�C������܂��B���̃V���{���́A���� DLL ���g���v���W�F�N�g�Œ�`���邱�Ƃ͂ł��܂���B
// �\�[�X�t�@�C�������̃t�@�C�����܂�ł��鑼�̃v���W�F�N�g�́A 
// TXTFUNCPICT_API �֐��� DLL ����C���|�[�g���ꂽ�ƌ��Ȃ��̂ɑ΂��A���� DLL �́A���̃}�N���Œ�`���ꂽ
// �V���{�����G�N�X�|�[�g���ꂽ�ƌ��Ȃ��܂��B
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
