// Text.h
//
// 2�o�C�g�����񑀍�
//
#ifndef __TEXT_H__
#define __TEXT_H__

#include "stltchar.h"

namespace CGrText
{
#ifdef UNICODE
#define GRCHAR TCHAR
#else
#define GRCHAR unsigned int
#endif
	// ���[�h�o�C�g���ǂ����𔻒�
	//   (�����J�n����)
	//   ch     : ����
	//   �߂�l : ���[�h�o�C�g�̏ꍇ TRUE
	BOOL isLead(GRCHAR ch);
	// �g���[�����O�o�C�g���ǂ����𔻒�
	//  �i�����j
	//   ch     : ����
	//   �߂�l : �g���[�����O�o�C�g�̏ꍇ TRUE
	BOOL isTrail(GRCHAR ch);
	// ���������擾���܂��B
	//   string : ������
	//   �߂�l : �Q�o�C�g������1�����Ƃ�����������Ԃ��܂��B
	int len(LPCTSTR string);
	// ���������ǂ�������
	//   c      : ����
	//   �߂�l : �������̏ꍇ TRUE
	BOOL isHira(GRCHAR c);
	// �V���{�����ǂ�������
	//   c      : ����
	//   �߂�l : �V���{���̏ꍇ TRUE
	BOOL isSymbl(GRCHAR c);
	// �V���{�����ǂ�������
	//   ������ �R �S �T �U �V �W �X  �̂��Âꂩ�ǂ���
	//   c      : ����
	//   �߂�l : �V���{���̏ꍇ TRUE
	BOOL isSymblKanji(GRCHAR c);
	// �Љ������ǂ�������
	//   c      : ����
	//   �߂�l : �Љ����̏ꍇ TRUE
	BOOL isKata(GRCHAR c);
	// �������ǂ������� : IsCharAlphaNumeric
	// �󔒂��ǂ�������
	//   c      : ����
	//   �߂�l : �󔒂̏ꍇ TRUE
	BOOL isSpace(GRCHAR c);
	// �J���}��؂蕶����ɕϊ����܂��B
	//   numStr    [out]: �������󂯎��string
	//   num        [in]: ����
	//   bZeroDisp  [in]: �[���̎��A�\��(=TRUE)
	void numCommaStr(std::tstring &numStr, int num, BOOL bZeroDisp);
	void FormatMessage(std::tstring &str, LPCTSTR lpszFormat, va_list *pargList);
	void FormatMessage(std::tstring &str, LPCTSTR lpszFormat, ...);
	void FormatMessage(std::tstring &str, UINT nFormatID, ...);
	void FormatMessage(HINSTANCE hInst, std::tstring &str, UINT nFormatID, ...);
#ifdef TXTMIRUFUNCLIB_DLL
	HINSTANCE GetDllModuleHandle();
	void SetDllModuleHandle(HINSTANCE hInst);
	void LoadString(UINT nFormatID, TCHAR *strFormat, int size);
	void LoadString(UINT nFormatID, std::tstring &str);
#else
	void LoadString(UINT nFormatID, TCHAR *strFormat, int size);
	void LoadString(UINT nFormatID, std::tstring &str);
#endif
	void LoadString(HINSTANCE hInst, UINT nFormatID, std::tstring &str);

	LPCTSTR CharNext(LPCTSTR lpStr);
	LPCTSTR CharPrev(LPCTSTR,LPCTSTR lpStr);
	LPCTSTR CharNextN(LPCTSTR str, int n);
	LPCTSTR CharPrevN(LPCTSTR begin, LPCTSTR str, int n);
	WORD GetStringTypeEx(LPCTSTR str, int len);
	int CharLen(LPCTSTR pSrc);
	// ������v�m�F
	//   pSrc    [in]: ������
	//   pDst    [in]: ��������(�ő� pDst�̕������܂Ō���)
	//   �߂�l [out]: ��v�Atrue
	bool isMatchChar(LPCTSTR pSrc, LPCTSTR pDst);
	//
	bool isMatchLast(LPCTSTR pSrc, LPCTSTR pDst);
	bool isMatchILast(LPCTSTR pSrc, LPCTSTR pDst);
	LPCTSTR Find(LPCTSTR lpSrc, LPCTSTR lpDst);
	LPCTSTR RFind(LPCTSTR lpBegin, LPCTSTR lpSrc, LPCTSTR lpDst);
	// �ŒZ��v������̎擾
	//   outstr [out]: ��v����������
	//   lpSrc   [in]: ����������
	//   lpStart [in]: �J�n����
	//   lpEnd   [in]: �I������
	//   �߂�l [out]: �ŏI���������ʒu
	LPCTSTR GetShortMatchStr(std::tstring &outstr, LPCTSTR lpSrc, LPCTSTR lpStart, LPCTSTR lpEnd);
	// �ŒZ��v������ʒu�̎擾
	//   lpSrc   [in]: ����������
	//   lpStart [in]: �J�n����
	//   lpEnd   [in]: �I������
	//   �߂�l [out]: �ŏI���������ʒu
	LPCTSTR GetFirstShortMatchPos(LPCTSTR lpSrc, LPCTSTR lpStart, LPCTSTR lpEnd);
	// �啶������������ʂ��āA��������r���܂��B
	//   string1 : ������
	//   string2 : ������
	//   coount  : ������
	//   �߂�l  : �� string1 �̕���������� string2 �̕����������菬����
	//              0 string1 �̕���������� string2 �̕���������Ɠ�����
	//             �� string1 �̕���������� string2 �̕�����������傫��
	int nCmp(LPCTSTR string1, LPCTSTR string2, size_t count);
	// �啶������������ʂ����ɁA��������r���܂��B
	//   string1 : ������
	//   string2 : ������
	//   coount  : ������
	//   �߂�l  : �� string1 �̕���������� string2 �̕����������菬����
	//              0 string1 �̕���������� string2 �̕���������Ɠ�����
	//             �� string1 �̕���������� string2 �̕�����������傫��
	int niCmp(LPCTSTR string1, LPCTSTR string2, size_t count);
	// �������ɕϊ����܂��B
	//   ch     : ����
	//   �߂�l : ������
	TCHAR toLower(TCHAR ch);
	// ������̒��ɕ��������邩�ǂ���
	//   striing : ������
	//   ch      : ����
	//   �߂�l  : �݂����ꍇ�� ���������ʒu
	LPCTSTR strChar(LPCTSTR string, LPCTSTR ch);

	int toInt(const std::tstring &str);
	void itoa(int num, std::tstring &str);
	bool MultiByteToTString(UINT CodePage, LPCSTR mb, const DWORD len, std::tstring& text);
	bool isUTF16(BYTE *p, int len);
};

#endif