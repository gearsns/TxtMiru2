#include <shlobj.h>
#include "DragDropImpl.h"
///////////////////////////////////////////////////////////////////////////////////////////////
CEnumFORMATETC::CEnumFORMATETC(){}
CEnumFORMATETC::~CEnumFORMATETC(){}
//
HRESULT __stdcall CEnumFORMATETC::QueryInterface(const IID &iid, void **ppv)
{
	if(iid == IID_IEnumFORMATETC || iid == IID_IUnknown){
		*ppv = static_cast<void*>(this);
		AddRef();
		return S_OK;
	} else {
		*ppv = nullptr;
		return E_NOINTERFACE;
	}
}
ULONG __stdcall CEnumFORMATETC::AddRef()
{
	return InterlockedIncrement(&m_cRefCount);
}
ULONG __stdcall CEnumFORMATETC::Release()
{
	if(m_cRefCount > 1){
		if(0 == InterlockedDecrement(&m_cRefCount)){
			delete this;
			return 0;
		}
	}
	return m_cRefCount;
}
//
HRESULT __stdcall CEnumFORMATETC::Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched)
{
	if(pceltFetched){
		*pceltFetched = 0;
	}
	ULONG n = celt;
	if(celt <= 0 || !rgelt || m_iCur >= m_fmtList.size()){
		return S_FALSE;
	}
	if(!pceltFetched && celt != 1){
		return S_FALSE;
	}
	auto it = m_fmtList.begin(), e = m_fmtList.end();
	for(ULONG i=0; i<m_iCur && it!=e; ++i, ++it){
		; // skip
	}
	for(; it!=e && n!=0; ++it, --n){
		*rgelt++ = *it;
		++m_iCur;
	}
	if(pceltFetched){
		*pceltFetched = celt - n;
	}
	return (n == 0) ? S_OK : S_FALSE;
}
HRESULT __stdcall CEnumFORMATETC::Skip(ULONG celt)
{
	if((m_iCur + static_cast<int>(celt)) >= m_fmtList.size()){
		return S_FALSE;
	}
	m_iCur += celt;
	return S_OK;
}
HRESULT __stdcall CEnumFORMATETC::Reset()
{
	m_iCur = 0;
	return S_OK;
}
bool CEnumFORMATETC::SetFormat(const FORMATETC &fmt)
{
	m_fmtList.push_back(fmt);
	return true;
}
HRESULT __stdcall CEnumFORMATETC::Clone(IEnumFORMATETC **ppenum)
{
	if(!ppenum){
		return E_POINTER;
	}
	auto *newEnum = new CEnumFORMATETC;
	if(!newEnum){
		return E_OUTOFMEMORY;
	}
	for(const auto &item : m_fmtList){
		newEnum->SetFormat(item);
	}
	newEnum->AddRef();
	newEnum->m_iCur = m_iCur;
	*ppenum = newEnum;
	return S_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CDropSource::CDropSource(){}
CDropSource:: ~CDropSource(){}
//
HRESULT __stdcall CDropSource::QueryInterface(const IID &iid, void **ppv)
{
	if(iid == IID_IDropSource || iid == IID_IUnknown){
		*ppv = (void*)this;
		AddRef();
		return S_OK;
	} else {
		*ppv = 0;
		return E_NOINTERFACE;
	}
}
ULONG __stdcall CDropSource::AddRef()
{
	return InterlockedIncrement(&m_cRefCount);
}
ULONG __stdcall CDropSource::Release()
{
	if(m_cRefCount > 1){
		if(0 == InterlockedDecrement(&m_cRefCount)){
			delete this;
			return 0;
		}
	}
	return m_cRefCount;
}
//
HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if(fEscapePressed || (MK_LBUTTON | MK_RBUTTON) == (grfKeyState & (MK_LBUTTON | MK_RBUTTON))){
		return DRAGDROP_S_CANCEL;
	}
	if((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0){
		m_bDropped = true;
		return DRAGDROP_S_DROP;
	}
	return S_OK;
}
HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool CDataObject::CopyMedium(STGMEDIUM *pdest, const FORMATETC *pFormatetc, const STGMEDIUM *pmedium)
{
	HANDLE hVoid = NULL;

	switch(pmedium->tymed){
	case TYMED_HGLOBAL:
		hVoid = OleDuplicateData(pmedium->hGlobal, pFormatetc->cfFormat, 0);
		pdest->hGlobal = static_cast<HGLOBAL>(hVoid);
		break;
	case TYMED_GDI:
		hVoid = OleDuplicateData(pmedium->hBitmap, pFormatetc->cfFormat, 0);
		pdest->hBitmap = static_cast<HBITMAP>(hVoid);
		break;
	case TYMED_MFPICT:
		hVoid = OleDuplicateData(pmedium->hMetaFilePict, pFormatetc->cfFormat, 0);
		pdest->hMetaFilePict = static_cast<HMETAFILEPICT>(hVoid);
		break;
	case TYMED_ENHMF:
		hVoid = OleDuplicateData(pmedium->hEnhMetaFile, pFormatetc->cfFormat, 0);
		pdest->hEnhMetaFile = static_cast<HENHMETAFILE>(hVoid);
		break;
	case TYMED_FILE:
		hVoid = OleDuplicateData(pmedium->lpszFileName, pFormatetc->cfFormat, 0);
		pdest->lpszFileName = static_cast<LPOLESTR>(hVoid);
		break;
	case TYMED_NULL:
		hVoid = reinterpret_cast<HANDLE>(1); //ƒGƒ‰[‚É‚È‚ç‚È‚¢‚æ‚¤‚É
	case TYMED_ISTREAM:
	case TYMED_ISTORAGE:
	default:
		break;
	}
	if(!hVoid){
		return false;
	}
	pdest->tymed = pmedium->tymed;
	pdest->pUnkForRelease = pmedium->pUnkForRelease;
	if(pmedium->pUnkForRelease){
		pmedium->pUnkForRelease->AddRef();
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CDataObject::CDataObject(CDropSource *pDropSource){}
CDataObject::~CDataObject(){}
//
HRESULT __stdcall CDataObject::QueryInterface(const IID &iid, void **ppv)
{
	if(iid == IID_IDataObject || iid == IID_IUnknown){
		*ppv = static_cast<void*>(this);
		AddRef();
		return S_OK;
	} else {
		*ppv = 0;
		return E_NOINTERFACE;
	}
}
ULONG __stdcall CDataObject::AddRef()
{
	return InterlockedIncrement(&m_cRefCount);
}
ULONG __stdcall CDataObject::Release()
{
	if(m_cRefCount > 1){
		if(0 == InterlockedDecrement(&m_cRefCount)){
			delete this;
			return 0;
		}
	}
	return m_cRefCount;
}
//
HRESULT __stdcall CDataObject::GetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
	if(!pFormatetc || !pmedium){
		return E_INVALIDARG;
	}
	if(!(DVASPECT_CONTENT & pFormatetc->dwAspect)){
		return DV_E_DVASPECT;
	}
	for(auto &item : m_objectList){
		if(item.fmt.tymed & pFormatetc->tymed
		   && item.fmt.dwAspect == pFormatetc->dwAspect
		   && item.fmt.cfFormat == pFormatetc->cfFormat){
			if(!CopyMedium(pmedium, &item.fmt, &item.medium)){
				return E_OUTOFMEMORY;
			}
			return S_OK;
		}
	}
	return DV_E_FORMATETC;
}
HRESULT __stdcall CDataObject::GetDataHere(FORMATETC *pFormatetc, STGMEDIUM *pmedium){ return E_NOTIMPL; }
HRESULT __stdcall CDataObject::QueryGetData(FORMATETC *pFormatetc)
{
	if(!pFormatetc){
		return E_INVALIDARG;
	}
	if(!(DVASPECT_CONTENT & pFormatetc->dwAspect)){
		return DV_E_DVASPECT;
	}
	HRESULT hr = DV_E_TYMED;
	for(auto &item : m_objectList){
		if(item.fmt.tymed & pFormatetc->tymed){
			if(item.fmt.cfFormat == pFormatetc->cfFormat){
				return S_OK;
			} else {
				hr = DV_E_CLIPFORMAT;
			}
		} else {
			hr = DV_E_TYMED;
		}
	}
	return hr;
}
HRESULT __stdcall CDataObject::GetCanonicalFormatEtc(FORMATETC *pFormatetcIn, FORMATETC *pFormatetcInOut)
{
	if(!pFormatetcInOut){
		return E_INVALIDARG;
	}
	return DATA_S_SAMEFORMATETC;
}
HRESULT __stdcall CDataObject::SetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	if(!pFormatetc || !pmedium){
		return E_INVALIDARG;
	}
	m_objectList.push_back(CObject());
	if(!m_objectList.back().Set(pFormatetc, pmedium, fRelease)){
		return E_OUTOFMEMORY;
	}
	return S_OK;
}
HRESULT __stdcall CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc)
{
	if(!ppenumFormatetc){
		return E_INVALIDARG;
	}
	CEnumFORMATETC *pfmt = nullptr;
	*ppenumFormatetc = nullptr;
	switch(dwDirection){
	case DATADIR_GET:
		ppenumFormatetc = reinterpret_cast<IEnumFORMATETC**>(new CEnumFORMATETC);
		if(!pfmt){
			return E_OUTOFMEMORY;
		}
		for(const auto &item : m_objectList){
			pfmt->SetFormat(item.fmt);
		}
		*ppenumFormatetc = pfmt;
		break;
	default:
		return E_NOTIMPL;
	}
	return S_OK;
}
HRESULT __stdcall CDataObject::DAdvise(FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection){ return OLE_E_ADVISENOTSUPPORTED; }
HRESULT __stdcall CDataObject::DUnadvise(DWORD dwConnection){ return OLE_E_ADVISENOTSUPPORTED; }
HRESULT __stdcall CDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise){ return OLE_E_ADVISENOTSUPPORTED; }
///////////////////////////////////////////////////////////////////////////////////////////////
CDropTarget::CDropTarget(HWND hTargetWnd) : m_hTargetWnd(hTargetWnd)
{
	if(FAILED(CoCreateInstance(CLSID_DragDropHelper, nullptr,CLSCTX_INPROC_SERVER,
							   IID_IDropTargetHelper, reinterpret_cast<LPVOID*>(&m_pDropTargetHelper)))){
		m_pDropTargetHelper = nullptr;
	}
}
CDropTarget::~CDropTarget()
{
	if(m_pDropTargetHelper){
		m_pDropTargetHelper->Release();
		m_pDropTargetHelper = nullptr;
	}
}
void CDropTarget::AddSuportedFormat(FORMATETC &ftetc){ m_formatetc.push_back(ftetc); }
//
HRESULT __stdcall CDropTarget::QueryInterface(const IID &iid, void **ppv)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown){
		*ppv = static_cast<void*>(this);
		AddRef();
		return S_OK;
	} else {
		*ppv = 0;
		return E_NOINTERFACE;
	}
}
ULONG __stdcall CDropTarget::AddRef()
{
	return InterlockedIncrement(&m_cRefCount);
}
ULONG __stdcall CDropTarget::Release()
{
	if(m_cRefCount > 1){
		if(0 == InterlockedDecrement(&m_cRefCount)){
			delete this;
			return 0;
		}
	}
	return m_cRefCount;
}
//
bool CDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{
	DWORD dwOKEffects = *pdwEffect;

	if(!m_bAllowDrop){
		*pdwEffect = DROPEFFECT_NONE;
		return false;
	}
	//CTRL+SHIFT  -- DROPEFFECT_LINK
	//CTRL        -- DROPEFFECT_COPY
	//SHIFT       -- DROPEFFECT_MOVE
	//no modifier -- DROPEFFECT_MOVE or whatever is allowed by src
	*pdwEffect = (grfKeyState & MK_CONTROL) ?
		( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ):
	( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 );
	if(*pdwEffect == 0){
		// No modifier keys used by user while dragging.
		if(DROPEFFECT_COPY & dwOKEffects){
			*pdwEffect = DROPEFFECT_COPY;
		} else if(DROPEFFECT_MOVE & dwOKEffects){
			*pdwEffect = DROPEFFECT_MOVE;
		} else if(DROPEFFECT_LINK & dwOKEffects){
			*pdwEffect = DROPEFFECT_LINK;
		} else {
			*pdwEffect = DROPEFFECT_NONE;
		}
	} else {
		// Check if the drag source application allows the drop effect desired by user.
		// The drag source specifies this in DoDragDrop
		if(!(*pdwEffect & dwOKEffects)){
			*pdwEffect = DROPEFFECT_NONE;
		}
	}

	return (DROPEFFECT_NONE == *pdwEffect) ? false : true;
}
HRESULT __stdcall CDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if(!pDataObject){
		return E_INVALIDARG;
	}
	if(m_pDropTargetHelper){
		m_pDropTargetHelper->DragEnter(m_hTargetWnd, pDataObject, reinterpret_cast<LPPOINT>(&ptl), *pdwEffect);
	}
	m_bAllowDrop = false;
	m_pSupportedFrmt = nullptr;
	for(auto &item : m_formatetc){
		m_bAllowDrop = (pDataObject->QueryGetData(&item) == S_OK) ? true : false;
		if(m_bAllowDrop){
			m_pSupportedFrmt = &item;
			break;
		}
	}
	QueryDrop(grfKeyState, pdwEffect);
	return S_OK;
}
HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(m_pDropTargetHelper){
		m_pDropTargetHelper->DragOver(reinterpret_cast<LPPOINT>(&pt), *pdwEffect);
	}
	QueryDrop(grfKeyState, pdwEffect);
	return S_OK;
}
HRESULT __stdcall CDropTarget::DragLeave()
{
	if(m_pDropTargetHelper){
		m_pDropTargetHelper->DragLeave();
	}
	m_bAllowDrop = false;
	m_pSupportedFrmt = nullptr;
	return S_OK;
}
#include <TCHAR.h>
HRESULT __stdcall CDropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if(!pDataObject){
		return E_INVALIDARG;
	}
	if(m_pDropTargetHelper){
		m_pDropTargetHelper->Drop(pDataObject, reinterpret_cast<LPPOINT>(&ptl), *pdwEffect);
	}
	bool bDrop = false;
	if(QueryDrop(grfKeyState, pdwEffect)){
		if(m_bAllowDrop && m_pSupportedFrmt){
			STGMEDIUM medium;
			if(pDataObject->GetData(m_pSupportedFrmt, &medium) == S_OK){
				if(OnDrop(m_pSupportedFrmt, medium, pdwEffect)){ //does derive class wants us to free medium?
					bDrop = true;
					ReleaseStgMedium(&medium);
				}
			}
		}
	}
	m_bAllowDrop = false;
	if(!bDrop){
		*pdwEffect = DROPEFFECT_NONE;
	}
	m_pSupportedFrmt = nullptr;
	return S_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////
CDragSourceHelper::CDragSourceHelper()
{
	if(FAILED(CoCreateInstance(CLSID_DragDropHelper,
							   nullptr,
							   CLSCTX_INPROC_SERVER,
							   IID_IDragSourceHelper,
							   reinterpret_cast<void**>(&m_pDragSourceHelper)))){
		m_pDragSourceHelper = nullptr;
	}
}
CDragSourceHelper::~CDragSourceHelper()
{
	if(m_pDragSourceHelper){
		m_pDragSourceHelper->Release();
		m_pDragSourceHelper = nullptr;
	}
}
//
HRESULT CDragSourceHelper::InitializeFromBitmap(HBITMAP hBitmap, POINT &pt, RECT &rc, IDataObject *pDataObject, COLORREF crColorKey)
{
	if(!m_pDragSourceHelper){
		return E_FAIL;
	}
	SHDRAGIMAGE di = {};
	BITMAP      bm = {};
	GetObject(hBitmap, sizeof(bm), &bm);
	di.sizeDragImage.cx = bm.bmWidth;
	di.sizeDragImage.cy = bm.bmHeight;
	di.hbmpDragImage = hBitmap;
	di.crColorKey = crColorKey;
	di.ptOffset.x = pt.x - rc.left;
	di.ptOffset.y = pt.y - rc.top;
	return m_pDragSourceHelper->InitializeFromBitmap(&di, pDataObject);
}
HRESULT CDragSourceHelper::InitializeFromWindow(HWND hwnd, POINT &pt,IDataObject *pDataObject)
{
	if(!m_pDragSourceHelper){
		return E_FAIL;
	}
	return m_pDragSourceHelper->InitializeFromWindow(hwnd, &pt, pDataObject);
}
