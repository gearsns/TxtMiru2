#include <windows.h>
#include "ImageFile.h"
#include "Image.h"
#include "Shell.h"
#include "TxtMiru.h"

CGrImageFile::CGrImageFile(LPCTSTR lpBasePath) : base_path(lpBasePath)
{
}

CGrImageFile::~CGrImageFile()
{
}

bool CGrImageFile::getImagePath(LPCTSTR lpDir, int icon_no, LPCTSTR lpExt, std::tstring &outFileName)
{
	WIN32_FIND_DATA wfd = {0};
	TCHAR path[MAX_PATH];
	_stprintf_s(path, _T("%s\\%s%d.%s"), lpDir, base_path.c_str(), icon_no, lpExt);
	if(CGrShell::getFileInfo(path, wfd)){
		outFileName = path;
		return true;
	}
	return false;
}
CGrImageFile::IMAGE_TYPE CGrImageFile::getImagePath(int icon_no, std::tstring &outFileName)
{
	static LPCTSTR l_check_ext[] = { _T("emf"), _T("png"), _T("jpg"), _T("jpeg") };
	auto lpDir = CGrTxtMiru::GetDataPath();
	int i=0;
	if(ft == FOLDER_TYPE::NONE || im == IMAGE_TYPE::NONE || ext_no == -1){
		for(i=0; i<sizeof(l_check_ext)/sizeof(LPCTSTR); ++i){
			if(getImagePath(lpDir, icon_no, l_check_ext[i], outFileName)){
				ext_no = i;
				ft = FOLDER_TYPE::DATA;
				im = i == 0 ? IMAGE_TYPE::EMF : IMAGE_TYPE::OLE;
				return im;
			}
		}
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		lpDir = curPath;
		for(i=0; i<sizeof(l_check_ext)/sizeof(LPCTSTR); ++i){
			if(getImagePath(lpDir, icon_no, l_check_ext[i], outFileName)){
				ext_no = i;
				ft = FOLDER_TYPE::EXEPATH;
				im = i == 0 ? IMAGE_TYPE::EMF : IMAGE_TYPE::OLE;
				return im;
			}
		}
	} else {
		if(ft == FOLDER_TYPE::EXEPATH){
			TCHAR curPath[MAX_PATH];
			CGrShell::GetExePath(curPath);
			lpDir = curPath;
		}
		if(getImagePath(lpDir, icon_no, l_check_ext[ext_no], outFileName)){
			return im;
		}
	}
	ext_no = -1;
	ft = FOLDER_TYPE::NONE;
	im = IMAGE_TYPE::NONE;
	return IMAGE_TYPE::NONE;
}

bool CGrImageFile::Draw(CGrBitmap &bmp, int icon_num, int width, int height)
{
	std::tstring image_filename;
	switch(getImagePath(icon_num, image_filename)){
	case IMAGE_TYPE::EMF: pict_emf.Draw(bmp, 0, 0, width, height, image_filename.c_str()); break;
	case IMAGE_TYPE::OLE: pict_ole.Draw(bmp, 0, 0, width, height, image_filename.c_str()); break;
	default:
		return false;
	}
	return true;
}

bool CGrImageFile::Draw(HDC hdc, int icon_num, int x, int y, int width, int height)
{
	std::tstring image_filename;
	switch(getImagePath(icon_num, image_filename)){
	case IMAGE_TYPE::EMF: pict_emf.Draw(hdc, x, y, width, height, image_filename.c_str()); break;
	case IMAGE_TYPE::OLE: pict_ole.Draw(hdc, x, y, width, height, image_filename.c_str()); break;
	default:
		return false;
	}
	return true;
}
