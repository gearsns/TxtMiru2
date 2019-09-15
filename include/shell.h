// Shell.h
//
// �t�@�C������
//
#ifndef __SHELL_H__
#define __SHELL_H__

#include "Text.h"
#include <vector>

namespace CGrShell
{
	// �����N��t�@�C�����̎擾
	bool GetPathFromLink(LPCTSTR linkFilename, std::tstring &filename);
	// �t�@�C�����̕\�L�𐮂��܂��B /./,\ -> /
	void ToPrettyFileName(LPCTSTR fromFileName, std::tstring &toFileName);
	void ToPrettyFileName(std::tstring &fileName);
	// �p�X��؂�L�����ǂ���
	//   c      : ����
	//   �߂�l : �p�X��؂蕶���̏ꍇ�ATRUE
	bool IsBackSlash(TCHAR c);
	// �w��t�@�C�������s�t�@�C�����ǂ������擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : ���s�t�@�C���Ȃ�TRUE
	BOOL IsExecuteFile(TCHAR *fileName);
	// �t�@�C�����J�n�ʒu���擾���܂��B
	//   fileName  [in]: �t�@�C����
	//   �߂�l   [out]: �t�@�C�����J�n�ʒu
	const TCHAR *GetFileName(const TCHAR *fileName);
	// �g���q�J�n�ʒu���擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : �g���q�J�n�ʒu
	TCHAR *GetFileExt(TCHAR *fileName);
	// �g���q�J�n�ʒu���擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : �g���q�J�n�ʒu
	LPCTSTR GetFileExtConst(LPCTSTR fileName);
	// �p�X����t�@�C�������폜
	//   path : �p�X
	BOOL RemoveFileName(TCHAR *path);
	// �t�@�C�����֘A�Â����s���܂��B
	//   hwnd : HWND
	//   path : �t�@�C����
	void Execute(HWND hwnd, const TCHAR *path);
	// �������g�̃t���p�X��
	void GetOwnFileName(TCHAR *path, int len);
	// �t�@�C���̏��擾
	bool getFileInfo(LPCTSTR lpFileName, WIN32_FIND_DATA &wfd);
	// �f�[�^�ۑ��p�X�̎擾
	void GetDataPath(TCHAR *path, int max_path, LPCTSTR appName);
	// ���s�t�@�C���̃p�X���擾���܂��B
	//   path : �p�X���擾����A�h���X
	void GetExePath(TCHAR *path);
	// �p�X�̍Ō��\��ǉ����܂��B
	//   path : �p�X
	void AddBackSlash(TCHAR *path);
	// �������t�@�C�����̈ꕔ�Ƃ��đ����������ǂ���
	//   ch     : ����
	//   �߂�l : ����������� TRUE
	BOOL IsAppropriateFileName(TCHAR ch);
	// double null-terminated�Ŋi�[����Ă��镶����̎��̕�������擾���܂��B
	//   strings : ������
	//   �߂�l  : ���̕�����̊J�n�ʒu
	TCHAR *GetNextString(TCHAR *strings);
	// double null-terminated�Ŋi�[����Ă��镶����̕����񐔂�Ԃ��B
	//   strings : ������
	//   �߂�l  : ������
	int GetStringCount(TCHAR *strings);
	// double null-terminated�Ŋi�[����Ă��镶����ɕ������ǉ����܂��B
	//   str    : ������
	//   addStr : �ǉ�������
	void AddString(TCHAR *str, int maxlen, TCHAR *addStr);
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
	LPCTSTR RemovePath(LPCTSTR fileName, LPCTSTR path);
	// �t�@�C�����ōŏ��Ɍ����p�X��؂蕶�����擾���܂��B
	//   fileName : �t�@�C����
	//   �߂�l   : �������A��؂蕶���ʒu
	//              ���s���ANULL
	TCHAR *GetFirstBackSlash(TCHAR *fileName);
	// �p�X��؂蕶���ŏI����Ă��邩�ǂ���
	//   fileName : �t�@�C����
	BOOL EndBackSlash(TCHAR *fileName);
	// �p�X��؂蕶���� "/" ���� "\\" �ɕύX���܂��B
	//   fileName : �t�@�C����
	void ModifyBackSlash(TCHAR *fileName);
	//
	void GetFileList(std::vector<WIN32_FIND_DATA> &flist, LPCTSTR path);
	// �t�H���_(�T�u�t�H���_���܂�)�ȉ��̃t�@�C��������
	// flist      [out]: std::vector<tstring>
	// path        [in]: �����t�H���_
	void GetFolderList(std::vector<std::tstring> &flist, LPCTSTR path);
	// �t�H���_(�T�u�t�H���_���܂�)�ȉ��̃t�@�C��������
	// flist      [out]: std::vector<tstring>
	// path        [in]: �����t�H���_
	// filter      [in]: �����p�^�[��
	// bNeedFolder [in]: �t�H���_��flist�ɉ����邩?
	void GetFullPathList(std::vector<std::tstring> &flist, LPCTSTR path, LPCTSTR filter, bool bNeedFolder=false);
	// �w��t�@�C���̗L�����擾���܂��B
	//   path   : �t�@�C����
	//   �߂�l : ���݂����ꍇ�́ATRUE
	BOOL GetSearchDir(LPCTSTR path);
	// �t�H���_���쐬���܂��B
	//   path   : �t�H���_��
	//   �߂�l : �쐬�����ꍇ�ͤTRUE
	BOOL CreateFolder(LPCTSTR path);
	// �e�f�B���N�g�����擾���܂��B
	//   fileName : �t���p�X
	//   parent   : �e�f�B���N�g�����󂯎��A�h���X
	//   �߂�l   : �e�f�B���N�g�����擾�ł��Ȃ��ꍇ�ͤNULL
	//              �擾�����ꍇ�ͤ�q�f�B���N�g���̃A�h���X
	//   ��parent == fileName ���� panret == NULL �̏ꍇ�ͤfileName�ɏ㏑��
	TCHAR *GetParentDir(TCHAR *fileName, TCHAR *parent);
	// URI���ǂ����𔻒f�Fhttp://, https://, file:///, ftp://
	bool IsURI(LPCTSTR lpFileName);
	bool GetPrevFile(std::tstring &outFileNae, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum);
	bool GetNextFile(std::tstring &outFileNae, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum);
	//
	bool MakeFullPath(std::tstring &outpath, LPCTSTR lpParent, LPCTSTR lpFolder);
};
#endif
