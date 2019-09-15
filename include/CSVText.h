#ifndef __CSVTEXT_H__
#define __CSVTEXT_H__
#pragma warning(disable:4786)

#include "stltchar.h"
#include <vector>
#include <string>
#include <algorithm>

#ifdef _DEBUG
#define CSV_COLMN    CVC
#define CSV_ROW      CVR
#define CSV_COLMN_IT CVCIT
#define CSV_ROW_IT   CVRIT
#endif

typedef std::vector<std::tstring> CSV_COLMN;
typedef std::vector<CSV_COLMN> CSV_ROW;
typedef CSV_COLMN::iterator CSV_COLMN_IT;
typedef CSV_ROW::iterator CSV_ROW_IT;

class CGrCSVText
{
public:
	CGrCSVText() {}
	CGrCSVText(LPCTSTR csv_string){ AddTailCSVData(csv_string); }
	CGrCSVText(const std::tstring &csv_string){ AddTailCSVData(csv_string.c_str()); }

	// �t�@�C�����J���܂��B
	//   fileName  [in]: �t�@�C����
	//   �߂�l   [out]: ���s���AFALSE
	BOOL Open(LPCTSTR fileName = nullptr);
	// �t�@�C����ۑ����܂��B
	//   fileName  [in]: �t�@�C����
	//   �߂�l   [out]: ���s���AFALSE
	BOOL Save(LPCTSTR fileName = nullptr);
	// ���ǉ����܂��B
	// CSV�f�[�^��ǉ����܂��B
	//   data  [in]: �f�[�^
	void AddTailCSVData(LPCTSTR data);
	//   line [in]: �ǉ����镶����
	void AddTail(LPCTSTR line);
	//
	void AddFormatTail(LPCTSTR fmt, ...);
	//   row  [in]: �}������s
	//   line [in]: �ǉ����镶����
	void Insert(int row, LPCTSTR line);
	//
	struct SortCompare {
		virtual bool operator()(const std::vector<std::tstring> &c1, const std::vector<std::tstring> &c2) const{ /*_ftprintf(stderr,_T("NGNGNGNGN\n"));*/return true; }
	};
	void Sort(SortCompare &sc);
	//
	struct EachFunc {
		virtual bool operator()(std::vector<std::tstring> &c1){ return true; }
	};
	void ForEachAll(EachFunc &ef);
	// �s�̍폜
	//   row [in]: �s
	void Remove(int row);
	// ���������܂��B
	void Clear() {
		m_data.clear();
		m_data.shrink_to_fit();
	}
	// ������̎擾
	//   row     [in]: �s
	//   col     [in]: ��
	//   �߂�l [out]: �e�L�X�g
	LPCTSTR GetText(int row, int col);
	// ������̎擾
	//   row     [in]: �s
	//   col     [in]: ��
	//   �߂�l [out]: string
	std::tstring *GetString(int row, int col);
	// �����̎擾
	//   row     [in]: �s
	//   col     [in]: ��
	//   def     [in]: �f�[�^���Ȃ������ꍇ�̒l
	//   �߂�l [out]: ����
	int GetInteger(int row, int col, int def);

	// �w��s�𐮐��̔z��Ŏ擾
	//   row    [in]: �s
	//   array [out]: �����̔z��
	//   len    [in]: array�̐�
	bool toIntArray(int row, int array[], int len) const;
	//   row    [in]: �s
	//   col    [in]: ��
	//   array [out]: �����̔z��
	//   len    [in]: array�̐�
	bool toIntArray(int row, int col, int array[], int len) const;
	//
	// ������̐ݒ�(�񂪑��݂��Ȃ��ꍇ�͒ǉ�)
	// ���A���A�s�͒ǉ�����Ȃ��̂ŗ\�� AddTail���ōs���m�ۂ��Ă�������
	//   row [in]: �s
	//   col [in]: ��
	//   str [in]: ������
	void SetTextExAdd(int row, int col, std::tstring str);
	// �����̐ݒ�(�񂪑��݂��Ȃ��ꍇ�͒ǉ�)
	// ���A���A�s�͒ǉ�����Ȃ��̂ŗ\�� AddTail���ōs���m�ۂ��Ă�������
	//   row [in]: �s
	//   col [in]: ��
	//   val [in]: ����
	void SetIntegerExAdd(int row, int col, int val);
	// CSV�`���ŕ�������擾���܂��B
	//   line   [out]: CSV������
	//   row     [in]: �s
	//   �߂�l [out]: ���s���AFALSE
	BOOL GetCSVText(std::tstring &line, int row);
	// CSV�`���ŕ�������擾���܂��B
	//   line   [out]: CSV������
	//   row     [in]: �s
	//   �߂�l [out]: ���s���AFALSE
	BOOL SetCSVText(LPCTSTR line, int row);
	// �s�����擾���܂��B
	//   �߂�l [out]: �s��
	inline int GetRowSize() { return m_data.size(); }
	// �񐔂��擾���܂��B
	//   row     [in]: �s
	//   �߂�l [out]: ��
	inline int GetColmnSize(int row){
		auto *p_csv_colmn = GetRow(row);
		return (!p_csv_colmn) ? 0 : p_csv_colmn->size();
	}
	// �s�̎擾
	//   row     [in]: �s
	//   �߂�l [out]: CSV_COLMN
	inline CSV_COLMN *GetRow(int row){
		if((int)m_data.size() <= row){
			return nullptr;
		}
		return &(m_data[row]);
	}

	CSV_ROW &GetCSVROW() { return m_data; }

	CGrCSVText& operator=(const CGrCSVText& csvt);

	void SetSplitChar(TCHAR ch){ m_splitChar = ch; }
	const TCHAR GetSplitChar(){ return m_splitChar; }
public:
	// �������CSV�`���ŉ�͂��āA��ɕ����܂��B
	//   csv_colmn [out]: ��
	//   line       [in]: CSV������
	static void toColumns(CSV_COLMN &csv_colmn, LPCTSTR line, TCHAR splitChar);
private:
	// �������ǉ����܂��B
	//   line       [in]: ������
	//   start_pos  [in]: �J�n�ʒu
	//   last_pos   [in]: �I���ʒu
	//   �߂�l    [out]: ���J�n�ʒu
	LPCTSTR addString(std::tstring &line, LPCTSTR start_pos, LPCTSTR last_pos);
	// �w�肵������ACSV������ɕϊ�
	//   csv_colmn  [in]: ��
	//   ret_str   [out]: CSV������
	//   ["]   -> [""""]
	//   [,]   -> [","]
	//   [",a] -> [""",a"]
	void enquote(std::tstring &ret_str, const CSV_COLMN &csv_colmn);
	// �t�@�C�����̃Z�b�g
	bool setFilename(LPCTSTR fileName);

private:
	TCHAR m_splitChar = _T(',');
	CSV_ROW m_data;
	std::tstring m_fileName;
};

#endif
