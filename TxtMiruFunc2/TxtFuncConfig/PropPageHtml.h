#ifndef __PROPPAGEHTML_H__
#define __PROPPAGEHTML_H__

#include "PropPageSize.h"

class CGrPropPageHtml : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);
	virtual ~CGrPropPageHtml();
	virtual bool Apply();
private:
	CGrPropPageHtml(CGrTxtConfigDlg *pDlg);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	// �I�[�o�[���C�h�\�ȃE�B���h�E(�_�C�A���O)�v���V�[�W��
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // __PROPPAGEHTML_H__
