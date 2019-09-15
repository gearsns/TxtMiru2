#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "CurrentDirectory.h"
#include "Shell.h"

static TCHAR l_module_path[512] = {};

CGrCurrentDirectory::CGrCurrentDirectory(bool bRestore) : m_bRestore(bRestore)
{
	::GetCurrentDirectory(sizeof(m_curDir)/sizeof(TCHAR), m_curDir);
	if(l_module_path[0] == '\0'){
		CGrShell::GetExePath(l_module_path);
	}
	::SetCurrentDirectory(l_module_path);
}

CGrCurrentDirectory::~CGrCurrentDirectory()
{
	if(m_bRestore){
		::SetCurrentDirectory(m_curDir);
	}
}

LPCTSTR CGrCurrentDirectory::GetModuleDir() const
{
	return l_module_path;
}

void CGrCurrentDirectory::SetRestore(bool bRestore)
{
	m_bRestore = bRestore;
}

bool CGrCurrentDirectory::GetRestore()
{
	return m_bRestore;
}

LPCTSTR CGrCurrentDirectory::GetCurrentDir() const
{
	return m_curDir;
}
