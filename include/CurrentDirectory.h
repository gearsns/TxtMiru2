#ifndef __CURRENTDIRECTORY_H__
#define __CURRENTDIRECTORY_H__

class CGrCurrentDirectory
{
	TCHAR m_curDir[512];
	bool  m_bRestore;
public:
	CGrCurrentDirectory(bool bRestore=true);
	virtual ~CGrCurrentDirectory();
	void SetRestore(bool bRestore);
	bool GetRestore();
	LPCTSTR GetCurrentDir() const;
	LPCTSTR GetModuleDir() const;
};

#endif // __CURRENTDIRECTORY_H__
