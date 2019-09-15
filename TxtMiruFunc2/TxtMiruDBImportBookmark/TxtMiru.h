#ifndef __TXTMIRU_H__
#define __TXTMIRU_H__

#include "stlutil.h"
#include "TxtParam.h"

class CGrTxtMiru
{
public:
	static CGrTxtMiru &theApp();
	static LPCTSTR ClassName(){ return _T("TxtMiru"); }
	static LPCTSTR AppName(){ return _T("TxtMiru"); }
	//
	HACCEL GetAccelerator(){ return m_haccel; }
	void SetAccelerator(HACCEL haccel){ m_haccel=haccel; }
	//
	void SetMouseCapture(HWND hwnd);
	void ReleaseMouseCapture();
	void PostMouseCaptureMessage(MSG &msg);
	//
	void SetUseAccelerator(bool bUseAcess){ m_bUseAcess = bUseAcess; }
	void SetUseDialogMessage(bool bUseDlgMes){ m_bUseDlgMes = bUseDlgMes; }
	bool IsUseAccelerator(){ return m_bUseAcess; }
	bool IsUseDialogMessage(){ return m_bUseDlgMes; }
	CGrTxtParam &Param(){ return m_param; }

	BOOL IsDialogMessage(HWND hwnd, MSG &msg);
	void SetActiveModelessDlg(HWND hDlg){ m_hModelessDlg=hDlg; }

	void SetWatting(bool bwait){ m_bWatting = bwait; }
	bool IsWaitting(){ return m_bWatting; }

	static LPCTSTR GetDataPath();
	static void MoveDataDir();
	static LPCTSTR GetWorkPath();
private:
	CGrTxtMiru(){}
	CGrTxtParam m_param;
	bool m_bUseAcess  = true;
	bool m_bUseDlgMes = false;
	bool m_bWatting   = false;
	HACCEL m_haccel   = NULL;
	HWND m_hModelessDlg     = NULL;
	HWND m_hMouseCaptureWnd = NULL;
};

#endif // __TXTMIRU_H__
