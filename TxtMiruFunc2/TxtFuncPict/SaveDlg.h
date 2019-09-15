#ifndef __SAVEDLG_H__
#define __SAVEDLG_H__
#include "WindowCtrl.h"
#include "ComonCtrl.h"

class CCriticalSection
{
private:
	CRITICAL_SECTION m_cs;
public:
	CCriticalSection(){
		InitializeCriticalSection(&m_cs);
	}
	~CCriticalSection(){
		DeleteCriticalSection(&m_cs);
	}
private:
	void Enter(){ EnterCriticalSection(&m_cs); }
	void Leave(){ LeaveCriticalSection(&m_cs); }
public:
	struct CEnterCriticalCtrl {
	private:
		CCriticalSection &m_cs;
	public:
		CEnterCriticalCtrl(CCriticalSection &cs) : m_cs(cs){ m_cs.Enter(); }
		~CEnterCriticalCtrl(){ m_cs.Leave(); }
	};
};

class CGrTxtFuncICanvas;
class CGrSaveDlg : public CGrComonCtrl
{
private:
	std::tstring m_orgFilename;
	std::tstring m_baseFilename;
	CCriticalSection m_cs;
	bool m_bCancel;
	int m_total_page;
	SIZE m_paperSize;
	CGrTxtFuncICanvas *m_pCanvas;
	HANDLE m_hHandle;
public:
	CGrSaveDlg(LPCTSTR filename, int total_page, SIZE paper_size, CGrTxtFuncICanvas *pCanvas);
	virtual ~CGrSaveDlg();
	bool Save(HWND hWnd);
	SIZE PaperSize(){ return m_paperSize; }
	int TotalPage(){ return m_total_page; }
private:
	int DoModal(HWND hWnd);
	bool isCanceled();
	void setCancel();
	static unsigned int __stdcall save_png_file(void *arg);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};

#endif // __SAVEDLG_H__
