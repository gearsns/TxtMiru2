#ifndef __TXTLAYOUTMGR_H__
#define __TXTLAYOUTMGR_H__

#include "TxtLayout.h"

class CGrTxtLayoutMgr
{
public:
	CGrTxtLayoutMgr();
	~CGrTxtLayoutMgr();

	bool Open(LPCTSTR lpFileName = NULL);
	bool Save(LPCTSTR lpFileName = NULL);

	void SetLayout(CGrTxtLayout *playout);
	const CGrTxtLayout &GetConstTxtLayout() const;
	CGrTxtLayout &GeTxtLayout();
	//
	int ConfigurationDlg(HWND hWnd, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName);
	struct LaytoutInfo {
		std::tstring type;
		std::tstring name;
		std::tstring filename;
		static bool LessType(const LaytoutInfo &a1, const LaytoutInfo &a2) {
			if(a1.type == a2.type){
				if(a1.name == a2.name){
					return a1.filename < a2.filename;
				} else {
					return a1.name < a2.name;
				}
			} else {
				return a1.type < a2.type;
			}
		}
	};
	void GetLayoutList(std::vector<LaytoutInfo> &lil);
	CGrTxtLayout *GetTxtLayout(const LaytoutInfo &li);
private:
	CGrTxtLayout *m_pDefTxtLayout;
	CGrTxtLayout *m_pTxtLayout;
};

#endif // __TXTLAYOUTMGR_H__
