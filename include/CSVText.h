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

	// ファイルを開きます。
	//   fileName  [in]: ファイル名
	//   戻り値   [out]: 失敗時、FALSE
	BOOL Open(LPCTSTR fileName = nullptr);
	// ファイルを保存します。
	//   fileName  [in]: ファイル名
	//   戻り値   [out]: 失敗時、FALSE
	BOOL Save(LPCTSTR fileName = nullptr);
	// 列を追加します。
	// CSVデータを追加します。
	//   data  [in]: データ
	void AddTailCSVData(LPCTSTR data);
	//   line [in]: 追加する文字列
	void AddTail(LPCTSTR line);
	//
	void AddFormatTail(LPCTSTR fmt, ...);
	//   row  [in]: 挿入する行
	//   line [in]: 追加する文字列
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
	// 行の削除
	//   row [in]: 行
	void Remove(int row);
	// 初期化します。
	void Clear() {
		m_data.clear();
		m_data.shrink_to_fit();
	}
	// 文字列の取得
	//   row     [in]: 行
	//   col     [in]: 列
	//   戻り値 [out]: テキスト
	LPCTSTR GetText(int row, int col);
	// 文字列の取得
	//   row     [in]: 行
	//   col     [in]: 列
	//   戻り値 [out]: string
	std::tstring *GetString(int row, int col);
	// 数字の取得
	//   row     [in]: 行
	//   col     [in]: 列
	//   def     [in]: データがなかった場合の値
	//   戻り値 [out]: 整数
	int GetInteger(int row, int col, int def);

	// 指定行を整数の配列で取得
	//   row    [in]: 行
	//   array [out]: 整数の配列
	//   len    [in]: arrayの数
	bool toIntArray(int row, int array[], int len) const;
	//   row    [in]: 行
	//   col    [in]: 列
	//   array [out]: 整数の配列
	//   len    [in]: arrayの数
	bool toIntArray(int row, int col, int array[], int len) const;
	//
	// 文字列の設定(列が存在しない場合は追加)
	// ※但し、行は追加されないので予め AddTail等で行を確保しておくこと
	//   row [in]: 行
	//   col [in]: 列
	//   str [in]: 文字列
	void SetTextExAdd(int row, int col, std::tstring str);
	// 整数の設定(列が存在しない場合は追加)
	// ※但し、行は追加されないので予め AddTail等で行を確保しておくこと
	//   row [in]: 行
	//   col [in]: 列
	//   val [in]: 整数
	void SetIntegerExAdd(int row, int col, int val);
	// CSV形式で文字列を取得します。
	//   line   [out]: CSV文字列
	//   row     [in]: 行
	//   戻り値 [out]: 失敗時、FALSE
	BOOL GetCSVText(std::tstring &line, int row);
	// CSV形式で文字列を取得します。
	//   line   [out]: CSV文字列
	//   row     [in]: 行
	//   戻り値 [out]: 失敗時、FALSE
	BOOL SetCSVText(LPCTSTR line, int row);
	// 行数を取得します。
	//   戻り値 [out]: 行数
	inline int GetRowSize() { return m_data.size(); }
	// 列数を取得します。
	//   row     [in]: 行
	//   戻り値 [out]: 列数
	inline int GetColmnSize(int row){
		auto *p_csv_colmn = GetRow(row);
		return (!p_csv_colmn) ? 0 : p_csv_colmn->size();
	}
	// 行の取得
	//   row     [in]: 行
	//   戻り値 [out]: CSV_COLMN
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
	// 文字列をCSV形式で解析して、列に分けます。
	//   csv_colmn [out]: 列
	//   line       [in]: CSV文字列
	static void toColumns(CSV_COLMN &csv_colmn, LPCTSTR line, TCHAR splitChar);
private:
	// 文字列を追加します。
	//   line       [in]: 文字列
	//   start_pos  [in]: 開始位置
	//   last_pos   [in]: 終了位置
	//   戻り値    [out]: 次開始位置
	LPCTSTR addString(std::tstring &line, LPCTSTR start_pos, LPCTSTR last_pos);
	// 指定した列を、CSV文字列に変換
	//   csv_colmn  [in]: 列
	//   ret_str   [out]: CSV文字列
	//   ["]   -> [""""]
	//   [,]   -> [","]
	//   [",a] -> [""",a"]
	void enquote(std::tstring &ret_str, const CSV_COLMN &csv_colmn);
	// ファイル名のセット
	bool setFilename(LPCTSTR fileName);

private:
	TCHAR m_splitChar = _T(',');
	CSV_ROW m_data;
	std::tstring m_fileName;
};

#endif
