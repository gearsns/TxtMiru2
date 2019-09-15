#ifndef __DRAGDROPIMPL_H__
#define __DRAGDROPIMPL_H__

#include <ShlObj.h>
#include <list>
class CEnumFORMATETC : public IEnumFORMATETC
{
public:
	using formatetc_list = std::list<FORMATETC>;
private:
	ULONG m_cRefCount = 1;
	ULONG m_iCur      = 0;
	formatetc_list m_fmtList;

public:
	CEnumFORMATETC();
	virtual ~CEnumFORMATETC();
	//
	bool SetFormat(const FORMATETC &fmt);
	//
	virtual HRESULT __stdcall QueryInterface(const IID &iid, void **ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();
	//
	virtual HRESULT __stdcall Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched);
	virtual HRESULT __stdcall Skip(ULONG celt);
	virtual HRESULT __stdcall Reset();
	virtual HRESULT __stdcall Clone(IEnumFORMATETC **ppenum);
};

///////////////////////////////////////////////////////////////////////////////////////////////
class CDropSource : public IDropSource
{
private:
	ULONG m_cRefCount = 1;
	bool  m_bDropped  = false;
public:
	CDropSource();
	virtual ~CDropSource();
	//
	virtual HRESULT __stdcall QueryInterface(const IID &iid, void **ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();
	//
	virtual HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	virtual HRESULT __stdcall GiveFeedback(DWORD dwEffect);
};
///////////////////////////////////////////////////////////////////////////////////////////////
class CDataObject : public IDataObject
{
private:
	CDropSource *m_pDropSource = nullptr;
	ULONG m_cRefCount = 1;
	static bool CopyMedium(STGMEDIUM *pdest, const FORMATETC *pFormatetc, const STGMEDIUM *pmedium);
	struct CObject {
	public:
		FORMATETC fmt = {};
		STGMEDIUM medium;
	public:
		CObject(){ medium.tymed = TYMED_NULL; }
		~CObject(){
			if(medium.tymed != TYMED_NULL){
				::ReleaseStgMedium(&medium);
			}
		}
		bool Set(FORMATETC *pf, STGMEDIUM *pm, BOOL fRelease){
			fmt = *pf;
			if(fRelease){
				medium = *pm;
				return TRUE;
			} else{
				return CopyMedium(&medium, pf, pm);
			}
		}
	};
	using CObjectList = std::list<CObject>;
	CObjectList m_objectList;
public:
	CDataObject(CDropSource *pDropSource);
	virtual ~CDataObject();
	//
	virtual HRESULT __stdcall QueryInterface(const IID &iid, void **ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();
	//
	virtual HRESULT __stdcall GetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium);
	virtual HRESULT __stdcall GetDataHere(FORMATETC *pFormatetc, STGMEDIUM *pmedium);
	virtual HRESULT __stdcall QueryGetData(FORMATETC *pFormatetc);
	virtual HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatetcIn, FORMATETC *pFormatetcInOut);
	virtual HRESULT __stdcall SetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease);
	virtual HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc);
	virtual HRESULT __stdcall DAdvise(FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
	virtual HRESULT __stdcall DUnadvise(DWORD dwConnection);
	virtual HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppenumAdvise);
};
///////////////////////////////////////////////////////////////////////////////////////////////
class CDropTarget : public IDropTarget
{
public:
	using formatetc_list = std::list<FORMATETC>;
private:
	long m_cRefCount = 1;
	struct IDropTargetHelper *m_pDropTargetHelper = nullptr;
protected:
	bool m_bAllowDrop = false;
	formatetc_list m_formatetc;
	FORMATETC *m_pSupportedFrmt = nullptr;
protected:
	HWND m_hTargetWnd = NULL;
public:

	CDropTarget(HWND hTargetWnd);
	virtual ~CDropTarget();
	void AddSuportedFormat(FORMATETC &ftetc);
	//
	virtual bool OnDrop(FORMATETC *pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect) = 0;
	//
	virtual HRESULT __stdcall QueryInterface(const IID &iid, void **ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();
	//
	bool QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect);
	virtual HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	virtual HRESULT __stdcall DragLeave();
	virtual HRESULT __stdcall Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
};
///////////////////////////////////////////////////////////////////////////////////////////////
class CDragSourceHelper
{
private:
	IDragSourceHelper *m_pDragSourceHelper = nullptr;
public:
	CDragSourceHelper();
	virtual ~CDragSourceHelper();
	//
	HRESULT InitializeFromBitmap(HBITMAP hBitmap, POINT &pt, RECT &rc, IDataObject *pDataObject, COLORREF crColorKey=GetSysColor(COLOR_WINDOW));
	HRESULT InitializeFromWindow(HWND hwnd, POINT &pt,IDataObject *pDataObject);
};
#endif //__DRAGDROPIMPL_H__
