#ifndef __IMAGEFILE_H__
#define __IMAGEFILE_H__

#include "TxtRenderer.h"

class CGrImageFile
{
public:
	enum class FOLDER_TYPE { DATA, EXEPATH, NONE };
	enum class IMAGE_TYPE { EMF, OLE, NONE };

	CGrImageFile(LPCTSTR lpBasePath);
	virtual ~CGrImageFile();
	bool Draw(CGrBitmap &bmp, int icon_num, int width, int height);
	bool Draw(HDC hdc, int icon_num, int x, int y, int width, int height);
private:
	std::tstring base_path;
	int         ext_no = -1;
	FOLDER_TYPE ft = FOLDER_TYPE::NONE;
	IMAGE_TYPE  im = IMAGE_TYPE::NONE;
	CGrPictEmfRenderer pict_emf;
	CGrPictOleRenderer pict_ole;
	bool getImagePath(LPCTSTR lpDir, int icon_no, LPCTSTR lpExt, std::tstring &outFileName);
	IMAGE_TYPE getImagePath(int icon_no, std::tstring &outFileName);
};

#endif // __IMAGEFILE_H__
