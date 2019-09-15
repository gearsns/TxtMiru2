#ifndef __PROPPAGEBOOKMARK_H__
#define __PROPPAGEBOOKMARK_H__

#include "PropPageSize.h"

class CGrPropPageBookmark : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);

	virtual ~CGrPropPageBookmark();
	virtual bool Apply();
private:
	CGrPropPageBookmark(CGrTxtConfigDlg *pDlg);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // __PROPPAGEBOOKMARK_H__
