// Shell.c
//
// �t�@�C������
//
#define STRICT // �^��錾����юg�p���ɁA��茵���Ȍ^�`�F�b�N���s���܂��B

#include <windows.h>
#include <shobjidl.h>
#include <shlguid.h>
#include "stlutil.h"
#ifdef _USRDLL
#else
#include "stdwin.h"
#endif
#include "Shell.h"
#pragma warning (push)
#pragma warning(disable:4768)
#pragma warning(disable:4091)
#include <shlobj.h>
#pragma warning (pop)

#define IS_ALPH(__ch__) (__ch__ >= _T('A') && __ch__ <= _T('Z'))
#define IS_NUM(__ch__) (__ch__ >= _T('0') && __ch__ <= _T('9'))

namespace CGrShell
{
	bool GetPathFromLink(LPCTSTR linkFilename, std::tstring &filename)
	{
		IShellLink   *psl = nullptr;
		IPersistFile *ppf = nullptr;
		bool ret = false;

		do {
			auto hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<void**>(&psl));
			if(FAILED(hres)){
				break;
			}
			hres = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));
			if(FAILED(hres)){
				break;
			}
			hres = ppf->Load(linkFilename, STGM_READ);
			if(FAILED(hres)){
				break;
			}
			TCHAR fullPath[_MAX_PATH] = {};
			hres = psl->GetPath(fullPath, _MAX_PATH, nullptr, 0);
			if(FAILED(hres)){
				break;
			}
			filename = fullPath;
			ret = true;
		} while(0);
		if(ppf){
			ppf->Release();
		}
		if(psl){
			psl->Release();
		}
		return ret;
	}
	// �t�H���_�I���_�C�A���O��Callback�֐�
	//   �����I���t�H���_���w�肷�邽�߂ɒǉ�
	//static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	// �t�@�C�����̕\�L�𐮂��܂��B /./,\ -> /
	void ToPrettyFileName(LPCTSTR fromFileName, std::tstring &toFileName)
	{
		toFileName = fromFileName;
		ToPrettyFileName(toFileName);
		if(toFileName.size() > 2 && toFileName[0] == _T('"') && toFileName[toFileName.length()-1] == _T('"')){
			toFileName = toFileName.substr(1, toFileName.length()-2);
		}
	}
	void ToPrettyFileName(std::tstring &fileName)
	{
		std::replace(fileName, _T("\n"), _T(""));
		std::replace(fileName, _T("\r"), _T(""));
		std::replace(fileName, _T("\\"), _T("/"));
		std::replace(fileName, _T("/./"), _T("/"));
	}

	// �w��t�@�C�������s�t�@�C�����ǂ������擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : ���s�t�@�C���Ȃ�TRUE
	BOOL IsExecuteFile(TCHAR *fileName)
	{
		// �g���q�̎擾
		auto *ext = GetFileExt(fileName);
		if(!ext){
			return FALSE;
		}
		if(CGrText::niCmp(_T("exe"), ext, 3) == 0 ||
		   CGrText::niCmp(_T("com"), ext, 3) == 0 ||
		   CGrText::niCmp(_T("bat"), ext, 3) == 0){
			return TRUE;
		}
		return FALSE;
	}

	// �g���q�J�n�ʒu���擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : �g���q�J�n�ʒu
	LPCTSTR GetFileExtConst(LPCTSTR fileName)
	{
		return GetFileExt(const_cast<TCHAR*>(fileName));
	}
	// �g���q�J�n�ʒu���擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : �g���q�J�n�ʒu
	TCHAR *GetFileExt(TCHAR *fileName)
	{
		const auto *lpNextSrc = fileName;
		while(*lpNextSrc){ lpNextSrc = CGrText::CharNext(lpNextSrc); }
		lpNextSrc = CharPrev(fileName, lpNextSrc);
		auto lpSrc = lpNextSrc;
		while(lpNextSrc != fileName && lpNextSrc && *lpNextSrc && !IsBackSlash(*lpNextSrc)){
			if(CGrText::niCmp(_T("."), lpNextSrc, 1) == 0){
				return const_cast<TCHAR*>(lpSrc);
			}
			lpSrc = lpNextSrc;
			lpNextSrc = CharPrev(fileName, lpNextSrc);
		}
		return nullptr;
	}

	// �t�@�C�����J�n�ʒu���擾���܂��B
	//   fileName  [in]: �t�@�C����
	//   �߂�l   [out]: �t�@�C�����J�n�ʒu
	const TCHAR *GetFileName(const TCHAR *fileName)
	{
		auto *lpNextSrc = fileName;
		while(*lpNextSrc){ lpNextSrc = CGrText::CharNext(lpNextSrc); }
		lpNextSrc = CharPrev(fileName, lpNextSrc);
		auto lpSrc = lpNextSrc;
		while(lpNextSrc != fileName && lpNextSrc && *lpNextSrc && !IsBackSlash(*lpNextSrc)){
			lpSrc = lpNextSrc;
			lpNextSrc = CharPrev(fileName, lpNextSrc);
		}
		return lpSrc;
	}

	// �p�X����t�@�C�������폜
	//   path : �p�X
	BOOL RemoveFileName(TCHAR *path)
	{
		auto *p = const_cast<TCHAR*>(GetFileName(path));
		if(p){
			if(!IsBackSlash(*p)){
				p = CharPrev(path, p);
			}
			*p = _T('\0');
			return TRUE;
		}
		return FALSE;
	}

	// �p�X��؂�L�����ǂ���
	//   c      : ����
	//   �߂�l : �p�X��؂蕶���̏ꍇ�ATRUE
	bool IsBackSlash(TCHAR c)
	{
		return (c == _T('\\') || c == _T('/'));
	}
	// �t�@�C���̏��擾
	bool getFileInfo(LPCTSTR lpFileName, WIN32_FIND_DATA &wfd)
	{
		auto hFile = ::FindFirstFile(lpFileName, &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return false;
		}
		::FindClose(hFile);
		return true;
	}

	// �f�[�^�ۑ��p�X�̎擾
	void GetDataPath(TCHAR *path, int max_path, LPCTSTR appName)
	{
		// LOCALAPPDATA�̃t�H���_�𓾂�B
		TCHAR dataPath[MAX_PATH];
		if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, dataPath))){
			_stprintf_s(path, max_path, _T("%s/%s"), dataPath, appName);
		} else {
			if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, dataPath))){
				_stprintf_s(path, max_path, _T("%s/%s"), dataPath, appName);
			} else {
				GetExePath(path);
			}
		}
	}
	// �t�@�C�����֘A�Â����s���܂��B
	//   hwnd : HWND
	//   path : �t�@�C����
	void Execute(HWND hwnd, const TCHAR *path)
	{
		TCHAR curPath[MAX_PATH];

		GetExePath(curPath);

		ShellExecute(hwnd, NULL, path, NULL, curPath, SW_SHOWDEFAULT);
	}

#ifdef _USRDLL
	// �������g�̃t���p�X��
	void GetOwnFileName(TCHAR *path, int len)
	{
		::GetModuleFileName(NULL, path, len);
	}
	// ���s�t�@�C���̃p�X���擾���܂��B
	//   path : �p�X���擾����A�h���X
	void GetExePath(TCHAR *path)
	{
		::GetModuleFileName(NULL, path, MAX_PATH-1);
		RemoveFileName(path);
	}
#else
	// �������g�̃t���p�X��
	void GetOwnFileName(TCHAR *path, int len)
	{
		::GetModuleFileName(CGrStdWin::GetInst(), path, len);
	}
	// ���s�t�@�C���̃p�X���擾���܂��B
	//   path : �p�X���擾����A�h���X
	void GetExePath(TCHAR *path)
	{
		::GetModuleFileName(CGrStdWin::GetInst(), path, MAX_PATH-1);
		RemoveFileName(path);
	}
#endif

	// �p�X�̍Ō��\��ǉ����܂��B
	//   path : �p�X
	void AddBackSlash(TCHAR *path)
	{
		int last = _tcslen(path)-1;
		if(last < 0){
			return;
		}
		if(!IsBackSlash(path[last])){
			path[last+1] = _T('\\');
			path[last+2] = _T('\0');
		}
	}

	// �������t�@�C�����̈ꕔ�Ƃ��đ����������ǂ���
	//   ch     : ����
	//   �߂�l : ����������� TRUE
	BOOL IsAppropriateFileName(TCHAR ch)
	{
		ch = toupper(ch);
		return (IS_ALPH(ch) || IS_NUM(ch) || ch == _T('@'));
	}

	// double null-terminated�Ŋi�[����Ă��镶����̎��̕�������擾���܂��B
	//   strings : ������
	//   �߂�l  : ���̕�����̊J�n�ʒu
	TCHAR *GetNextString(TCHAR *strings)
	{
		if(!strings || *strings == _T('\0')){
			return nullptr;
		}
		return strings + _tcslen(strings) + 1;
	}

	// double null-terminated�Ŋi�[����Ă��镶����̕����񐔂�Ԃ��B
	//   strings : ������
	//   �߂�l  : ������
	int GetStringCount(TCHAR *strings)
	{
		int  i = 0;

		auto p = strings;
		while(*p != _T('\0')){
			i++;
			p += _tcslen(strings)+1;
		}
		return i;
	}

	// double null-terminated�Ŋi�[����Ă��镶����ɕ������ǉ����܂��B
	//   str    : ������
	//   addStr : �ǉ�������
	void AddString(TCHAR *str, int maxlen, TCHAR *addStr)
	{
		auto *p = str;
		while(*p != _T('\0')){
			p += _tcslen(p)+1;
		}

		//lstrcat(p, addStr);
		_tcscat_s(p, maxlen, addStr);
		p += _tcslen(p)+1;
		*p = _T('\0');
	}

	// fileName����path���폜���܂��B
	//   fileName : �t�@�C�����̃t���p�X
	//   path     : �폜����p�X
	//   �߂�l   : fileName��path���܂܂�Ă��Ȃ��ꍇ�ANULL
	//              �������ͤfileName����path���폜�����ʒu
	//   ex.
	//       path     = "path/path/path"
	//       fileName = "path/path/path/fileName"
	//     ->return   = "fileName"
	//       path     = "path2/path2/path2"
	//       fileName = "path2/path2/path1/fileName"
	//     ->return   = NULL
	LPCTSTR RemovePath(LPCTSTR fileName, LPCTSTR path)
	{
		if(!fileName || !path){
			return nullptr;
		}

		auto name = fileName;
		while(*path != _T('\0')){
			if(IsBackSlash(*path)){
				if(!IsBackSlash(*name)){
					return nullptr;
				}
			} else if(*path != *name){
				return nullptr;
			}
			path = CharNext(path);
			name = CharNext(name);
		}

		return name;
	}

	// �t�@�C�����ōŏ��Ɍ����p�X��؂蕶�����擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : �������A��؂蕶���ʒu
	//              ���s���ANULL
	TCHAR *GetFirstBackSlash(TCHAR *fileName)
	{
		if(!fileName){
			return nullptr;
		}

		while(*fileName != _T('\0')){
			if(IsBackSlash(*fileName)){
				return fileName;
			}
			fileName = CharNext(fileName);
		}
		return nullptr;
	}

	// �p�X��؂蕶���ŏI����Ă��邩�ǂ���
	//   fileName : �t�@�C����
	BOOL EndBackSlash(TCHAR *fileName)
	{
		if(!fileName){
			return FALSE;
		}

		auto len = _tcslen(fileName);
		if(len <= 0){
			return FALSE;
		}
		return IsBackSlash(fileName[len-1]);
	}

	// �p�X��؂蕶���� "/" ���� "\\" �ɕύX���܂��B
	//   fileName : �t�@�C����
	void ModifyBackSlash(TCHAR *fileName)
	{
		TCHAR *p;
		TCHAR c = _T('/');

		while((p = const_cast<TCHAR*>(CGrText::strChar(fileName, &c))) != nullptr){
			*p = _T('\\');
		}
	}

	void GetFileList(std::vector<WIN32_FIND_DATA> &flist, LPCTSTR path)
	{
		WIN32_FIND_DATA wfd = {};
		std::tstring buf(path);
		if(buf.empty()){
			return;
		}
		if(IsBackSlash(buf[buf.size()-1])){
			buf.resize(buf.size()-1);
		}
		path = buf.c_str();
		std::tstring find_path;
		// �f�B���N�g���ꗗ�擾
		CGrText::FormatMessage(find_path, _T("%1!s!\\*.*"), path);
		//
		auto hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(_tcscmp(wfd.cFileName, _T(".")) == 0 ||
				   _tcscmp(wfd.cFileName, _T("..")) == 0){
					continue;
				}
			}
			flist.push_back(wfd);
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
	}
	// �t�H���_(�T�u�t�H���_���܂�)�ȉ��̃t�@�C��������
	// flist      [out]: std::vector<tstring>
	// path        [in]: �����t�H���_
	void GetFolderList(std::vector<std::tstring> &flist, LPCTSTR path)
	{
		WIN32_FIND_DATA wfd = {};
		std::tstring buf(path);
		if(buf.empty()){
			return;
		}
		if(IsBackSlash(buf[buf.size()-1])){
			buf.resize(buf.size()-1);
		}
		path = buf.c_str();
		std::tstring find_path;
		// �f�B���N�g���ꗗ�擾
		CGrText::FormatMessage(find_path, _T("%1!s!\\*.*"), path);
		//
		auto hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(_tcscmp(wfd.cFileName, _T(".")) == 0 ||
				   _tcscmp(wfd.cFileName, _T("..")) == 0){
					continue;
				}
				std::tstring next_path;
				CGrText::FormatMessage(next_path, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				flist.emplace_back(next_path);
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
	}
	// �t�H���_(�T�u�t�H���_���܂�)�ȉ��̃t�@�C��������
	// flist      [out]: std::vector<tstring>
	// path        [in]: �����t�H���_
	// filter      [in]: �����p�^�[��
	// bNeedFolder [in]: �t�H���_��flist�ɉ����邩?
	void GetFullPathList(std::vector<std::tstring> &flist, LPCTSTR path, LPCTSTR filter, bool bNeedFolder)
	{
		WIN32_FIND_DATA wfd = {};
		std::tstring buf(path);
		if(buf.empty()){
			return;
		}
		if(IsBackSlash(buf[buf.size()-1])){
			buf.resize(buf.size()-1);
		}
		path = buf.c_str();
		std::tstring find_path;
		// �f�B���N�g���ꗗ�擾
		CGrText::FormatMessage(find_path, _T("%1!s!\\*.*"), path);
		//
		auto hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(_tcscmp(wfd.cFileName, _T(".")) == 0 ||
				   _tcscmp(wfd.cFileName, _T("..")) == 0){
					continue;
				}
				std::tstring next_path;
				CGrText::FormatMessage(next_path, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				GetFullPathList(flist, next_path.c_str(), filter, bNeedFolder);
				if(bNeedFolder){
					flist.push_back(next_path);
				}
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
		// �t�@�C���ꗗ�擾
		CGrText::FormatMessage(find_path, _T("%1!s!\\%2!s!"), path, filter);
		hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				;
			} else {
				std::tstring next_path;
				CGrText::FormatMessage(next_path, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				flist.push_back(next_path);
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
	}

	// �w��t�@�C���̗L�����擾���܂��B
	//   path   : �t�@�C����
	//   �߂�l : ���݂����ꍇ�́ATRUE
	BOOL GetSearchDir(LPCTSTR path)
	{
		WIN32_FIND_DATA wfd;
		auto hFile = FindFirstFile(path, &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			// ������܂���ł����B
			return FALSE;
		}
		FindClose(hFile);
		return TRUE;
	}

	// �t�H���_���쐬���܂��B
	//   path   : �t�H���_��
	//   �߂�l : �쐬�����ꍇ�ͤTRUE
	BOOL CreateFolder(LPCTSTR path)
	{
		if(!GetSearchDir(path)){
			if(_tcslen(path) <= 2 && path[1] == _T(':')){
				// �h���C�u��
				return FALSE;
			}
			if(!CreateDirectory(path, NULL)){
				TCHAR folder[MAX_PATH];
				_tcscpy_s(folder, path);
				ModifyBackSlash(folder);
				if(RemoveFileName(folder)){
					CreateFolder(folder);
				}
				auto bOK = CreateDirectory(path, NULL);
				return bOK;
			} else {
				return TRUE;
			}
		}
		return FALSE;
	}

	// �e�f�B���N�g�����擾���܂��B
	//   fileName : �t���p�X
	//   parent   : �e�f�B���N�g�����󂯎��A�h���X
	//   �߂�l   : �e�f�B���N�g�����擾�ł��Ȃ��ꍇ�ͤNULL
	//              �擾�����ꍇ�ͤ�q�f�B���N�g���̃A�h���X
	//   ��parent == fileName ���� panret == NULL �̏ꍇ�ͤfileName�ɏ㏑��
	TCHAR *GetParentDir(TCHAR *fileName, TCHAR *parent)
	{
		TCHAR *path, *prePath = nullptr;
		if(!parent){
			parent = fileName;
		} else if(parent != fileName){
#ifdef _DEBUG
			_tcscpy_s(parent, 128, fileName);
#else
			_tcscpy_s(parent, 512, fileName);
#endif // DEBUG
		}
		path = GetFirstBackSlash(parent);
		if(!path){
			return nullptr;
		}
		while(path){
			path = const_cast<TCHAR*>(CGrText::CharNext(path));
			if(*path == _T('\0')){
				break;
			}
			prePath = path;
			path = GetFirstBackSlash(path);
		}
		if(prePath){
			*prePath = _T('\0');
			return fileName + _tcslen(parent);
		}
		return nullptr;
	}
	bool IsURI(LPCTSTR lpFileName)
	{
		auto lpFind = CGrText::Find(lpFileName, _T(":"));
		return (lpFind && lpFind > (lpFileName+1));
	}
#define IS_DIRECTORY(_pwfd_) ((_pwfd_)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	struct SortWFDCompare {
		SortWFDCompare(){}
		bool operator()(const WIN32_FIND_DATA &wfd1, const WIN32_FIND_DATA &wfd2) const {
			int f1 = 0;
			int f2 = 0;
			std::tstring name1;
			std::tstring name2;
			std::tstring ext1;
			std::tstring ext2;
			if(IS_DIRECTORY(&wfd1)){
				f1 = 1;
				name1 = wfd1.cFileName;
			} else {
				auto lpExt = GetFileExtConst(wfd1.cFileName);
				if(lpExt){
					name1 = std::tstring(wfd1.cFileName, lpExt);
					ext1  = lpExt + 1;
				} else {
					name1 = wfd1.cFileName;
				}
			}
			if(IS_DIRECTORY(&wfd2)){
				f2 = 1;
				name2 = wfd2.cFileName;
			} else {
				auto lpExt = GetFileExtConst(wfd2.cFileName);
				if(lpExt){
					name2 = std::tstring(wfd2.cFileName, lpExt);
					ext2  = lpExt + 1;
				} else {
					name2 = wfd2.cFileName;
				}
			}
			int ret = f1 - f2;
			if(ret == 0){
				ret = _tcsicmp(name1.c_str(), name2.c_str());
				if(ret == 0){
					ret = _tcsicmp(ext1.c_str(), ext2.c_str());
				}
			}
			return ret < 0;
		}
	};
	struct FileData {
		LPCTSTR lpFileName;
	};
	static int FileExtCompare(const void *p1, const void *p2){
		auto *lpFile1 = static_cast<const FileData*>(p1);
		auto *lpFile2 = static_cast<const FileData*>(p2);
		return _tcsicmp(lpFile1->lpFileName, lpFile2->lpFileName);
	}
	static bool GetPrevFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpFolderName, LPCTSTR lpFullPath, FileData lpExtList[], int extListNum, int type = 1)
	{
		bool bFile   = false;
		bool bFolder = false;
		bool bParent = false;
		std::vector<WIN32_FIND_DATA> flist;
		GetFileList(flist, lpFullPath);
		std::sort(flist.begin(), flist.end(), SortWFDCompare());
		int len = flist.size();
		if(lpFileName){
			// ��O�̃t�@�C�������邩�m�F
			bFile   = true;
			bParent = true;
		} else if(lpFolderName){
			// �t�H���_�ɑΏۃt�@�C��������t�H���_�����邩
			bFolder = true;
			bParent = true;
		} else {
			bFolder = true;
		}
		if(bFolder){
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[len-1];
				for(; len>0; --len, --pwfd){
					if(IS_DIRECTORY(pwfd)){
						if(lpFolderName && _tcsicmp(pwfd->cFileName, lpFolderName) >= 0){
							continue;
						}
						std::tstring new_foldername;
						CGrText::FormatMessage(new_foldername, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
						if(GetPrevFile(outFileName, nullptr, nullptr, new_foldername.c_str(), lpExtList, extListNum)){
							return true;
						}
					}
				}
				bFile = true; // �Ώۃt�@�C�������邩
			}
		}
		if(bFile){
			// �Ώۃt�@�C�������邩
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[len-1];
				for(; len>0; --len, --pwfd){
					if(pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
						continue;
					}
					auto lpFileExt = GetFileExt(pwfd->cFileName);
					if(!lpFileExt){ // 2.0.11.0
						continue;
					}
					FileData fd = {lpFileExt};
					if(!bsearch(&fd, lpExtList, extListNum, sizeof(FileData), FileExtCompare)){
						continue;
					}
					if(lpFileName && _tcsicmp(pwfd->cFileName, lpFileName) >= 0){
						continue;
					}
					CGrText::FormatMessage(outFileName, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
					return true;
				}
			}
		}
		if(bParent){
			// �e�t�H���_���`�F�b�N
			std::tstring parent_foldername(lpFullPath);
			int pos = parent_foldername.find_last_of(_T("\\/"));
			if(pos != std::string::npos){
				auto lpTartgetFolder = lpFullPath + (pos+1);
				parent_foldername[pos] = '\0';
				return GetPrevFile(outFileName, nullptr, lpTartgetFolder, parent_foldername.c_str(), lpExtList, extListNum);
			}
		}
		return false;
	}
	bool GetPrevFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum)
	{
		if(extListNum <= 0){
			return false;
		}
		LPCTSTR lpTartget = nullptr;
		std::tstring find_path(lpFileName);
		int pos = find_path.find_last_of(_T("\\/"));
		if(pos != std::string::npos){
			find_path[pos] = '\0';
			lpTartget = lpFileName + (pos+1);
		}
		auto *fdlist = new FileData[extListNum];
		for(int i=0; i<extListNum; ++i){
			fdlist[i].lpFileName = lpExtList[i];
		}
		qsort(fdlist, extListNum, sizeof(FileData), FileExtCompare);
		auto ret = GetPrevFile(outFileName, lpTartget, nullptr, find_path.c_str(), fdlist, extListNum);
		delete [] fdlist;
		return ret;
		return false;
	}
	static bool GetNextFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpFolderName, LPCTSTR lpFullPath, FileData lpExtList[], int extListNum)
	{
		bool bFile   = false;
		bool bFolder = false;
		bool bParent = false;
		std::vector<WIN32_FIND_DATA> flist;
		GetFileList(flist, lpFullPath);
		std::sort(flist.begin(), flist.end(), SortWFDCompare());
		int len = 0;
		if(lpFileName){
			// ���̃t�@�C�������邩�m�F
			bFile   = true;
			bParent = true;
		} else if(lpFolderName){
			// �t�H���_�ɑΏۃt�@�C��������t�H���_�����邩
			bFolder = true;
			bParent = true;
		} else {
			// �Ώۃt�@�C�������邩
			bFile   = true;
		}
		if(bFile){
			// �Ώۃt�@�C�������邩
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[0];
				for(; len>0; --len, ++pwfd){
					if(IS_DIRECTORY(pwfd)){
						continue;
					}
					auto lpFileExt = GetFileExt(pwfd->cFileName);
					if(!lpFileExt){
						continue;
					}
					FileData fd = {lpFileExt};
					if(!bsearch(&fd, lpExtList, extListNum, sizeof(FileData), FileExtCompare)){
						continue;
					}
					if(lpFileName && _tcsicmp(pwfd->cFileName, lpFileName) <= 0){
						continue;
					}
					CGrText::FormatMessage(outFileName, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
					return true;
				}
				bFolder = true; // �Ώۃt�@�C��������t�H���_�����邩
			}
		}
		if(bFolder){
			// �Ώۃt�@�C��������t�H���_�����邩
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[0];
				for(; len>0; --len, ++pwfd){
					if(IS_DIRECTORY(pwfd)){
						if(lpFolderName && _tcsicmp(pwfd->cFileName, lpFolderName) <= 0){
							continue;
						}
						std::tstring new_foldername;
						CGrText::FormatMessage(new_foldername, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
						if(GetNextFile(outFileName, nullptr, nullptr, new_foldername.c_str(), lpExtList, extListNum)){
							return true;
						}
					}
				}
			}
		}
		if(bParent){
			// �e�t�H���_���`�F�b�N
			std::tstring parent_foldername(lpFullPath);
			auto pos = parent_foldername.find_last_of(_T("\\/"));
			if(pos != std::string::npos){
				auto lpTartgetFolder = lpFullPath + (pos+1);
				parent_foldername[pos] = '\0';
				return GetNextFile(outFileName, nullptr, lpTartgetFolder, parent_foldername.c_str(), lpExtList, extListNum);
			}
		}
		return false;
	}
	bool GetNextFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum)
	{
		if(extListNum <= 0){
			return false;
		}
		LPCTSTR lpTartget = nullptr;
		std::tstring find_path(lpFileName);
		auto pos = find_path.find_last_of(_T("\\/"));
		if(pos != std::string::npos){
			find_path[pos] = '\0';
			lpTartget = lpFileName + (pos+1);
		}
		auto *fdlist = new FileData[extListNum];
		for(int i=0; i<extListNum; ++i){
			fdlist[i].lpFileName = lpExtList[i];
		}
		qsort(fdlist, extListNum, sizeof(FileData), FileExtCompare);
		auto ret = GetNextFile(outFileName, lpTartget, nullptr, find_path.c_str(), fdlist, extListNum);
		delete [] fdlist;
		return ret;
	}
	bool MakeFullPath(std::tstring &outpath, LPCTSTR lpParent, LPCTSTR lpFolder)
	{
		TCHAR *lpfilepart;
		TCHAR fullpath[MAX_PATH+1];
		TCHAR exepath[MAX_PATH] = {};
		if(!lpParent){
			GetExePath(exepath);
			if(exepath[0] == _T('\0')){
				return false;
			}
			lpParent = exepath;
		}
		::GetFullPathName(lpParent, sizeof(fullpath)/sizeof(TCHAR), fullpath, &lpfilepart);
		CGrShell::AddBackSlash(fullpath);
		CGrText::FormatMessage(outpath, _T("%1!s!%2!s!\\"), fullpath, lpFolder);
		return true;
	};
};