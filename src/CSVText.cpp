#include <windows.h>
#include "CSVText.h"
#include <sstream>
#include <algorithm>
#include "Shell.h"

#define IS_LF(__ch__)   ((__ch__) == _T('\n'))
#define IS_CR(__ch__)   ((__ch__) == _T('\r'))

static bool isInQuote(LPCTSTR pStr, bool bInQuote, TCHAR splitChar)
{
	if(!bInQuote && splitChar == *pStr){
		return false;
	} else if(_T('"') == *pStr){
		if(!bInQuote){
			return true;
		} else if(_T('"') != *(pStr+1)){
			return false;
		}
	}
	return bInQuote;
}

static bool isUTF16(BYTE *p, int len)
{
	while(--len > 0){
		if(*p == '\0'){
			return true;
		}
		++p;
	}
	return false;
}

// ファイル名のセット
bool CGrCSVText::setFilename(LPCTSTR fileName)
{
	if(fileName){
		m_fileName = std::tstring(fileName);
		CGrShell::ModifyBackSlash(&(m_fileName[0]));
	}
	return !m_fileName.empty();
}

// ファイルを開きます。
//   fileName  [in]: ファイル名
//   戻り値   [out]: 失敗時、FALSE
BOOL CGrCSVText::Open(const TCHAR *fileName)
{
	if(!setFilename(fileName)){ return FALSE; }
	// ファイルを開きます。
	auto hFile = ::CreateFile(m_fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return FALSE;
	}

	BOOL ret = TRUE;
	DWORD sizeHigh;
	// サイズを取得して領域を確保します。
	auto fsize = ::GetFileSize(hFile, &sizeHigh);
	if(fsize > 0){
		auto hHeap = ::HeapCreate(0, 0, 0);
		if (!hHeap) { // 2.0.39.0
			return FALSE;
		}
		auto *data = static_cast<BYTE*>(::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BYTE)*(fsize+1)));
		DWORD dw;
		do {
			if(!data){
				ret = FALSE;
				break;
			}
			if(0 == ::ReadFile(hFile, data, fsize, &dw, nullptr)){
				ret = FALSE;
				break;
			}
			auto *beginData = data;
			data[dw] = _T('\0');
#ifdef UNICODE
			if(fsize >= 2 &&
			   data[0] == 0xff && data[1] == 0xfe){
				beginData = &data[2];
			} else if(fsize >= 3 &&
			   data[0] == 0xef && data[1] == 0xbb && data[2] == 0xbf){
				int nLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(data), -1, nullptr,0);
				if(nLen != 0){
					auto sjishHeap = ::HeapCreate(0, 0, 0);
					auto *sjisdata = static_cast<BYTE*>(::HeapAlloc(sjishHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR)*(nLen+1)));
					::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(data), -1, reinterpret_cast<TCHAR*>(sjisdata), nLen);
					::HeapFree(hHeap, 0, data);
					::HeapDestroy(hHeap);
					hHeap = sjishHeap;
					data = sjisdata;
					beginData = data;
				}
			} else if(!isUTF16(data, fsize)){
				int nLen = ::MultiByteToWideChar(CP_THREAD_ACP, 0, reinterpret_cast<char*>(data), -1, nullptr,0);
				if(nLen != 0){
					auto sjishHeap = ::HeapCreate(0, 0, 0);
					if (sjishHeap) {
						auto* sjisdata = static_cast<BYTE*>(::HeapAlloc(sjishHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * (nLen + 1)));
						::MultiByteToWideChar(CP_THREAD_ACP, 0, reinterpret_cast<char*>(data), -1, reinterpret_cast<TCHAR*>(sjisdata), nLen);
						::HeapFree(hHeap, 0, data);
						::HeapDestroy(hHeap);
						hHeap = sjishHeap;
						data = sjisdata;
						beginData = data;
					}
				}
			}
#endif
			AddTailCSVData(reinterpret_cast<TCHAR*>(beginData));
		} while(0);
		::HeapFree(hHeap, 0, data);
		::HeapDestroy(hHeap);
	}
	::CloseHandle(hFile);
	return ret;
}

// ファイルを保存します。
//   fileName  [in]: ファイル名
//   戻り値   [out]: 失敗時、FALSE
BOOL CGrCSVText::Save(const TCHAR *fileName)
{
	if(!setFilename(fileName)){ return FALSE; }
	auto hFile = ::CreateFile(m_fileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		TCHAR folder[MAX_PATH];
		_tcscpy_s(folder, fileName);
		CGrShell::ModifyBackSlash(folder);
		if(CGrShell::RemoveFileName(folder)){
			CGrShell::CreateFolder(folder);
			hFile = ::CreateFile(m_fileName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
		}
		if(hFile == INVALID_HANDLE_VALUE){
			return FALSE;
		}
	}
	DWORD dw;
#ifdef UNICODE
	BYTE header[] = {0xff, 0xfe};
	::WriteFile(hFile, header, sizeof(header), &dw, NULL);
#endif
	std::tstring line;
	for(const auto &row : m_data){
		enquote(line, row);
		line += _T("\r\n");
		::WriteFile(hFile, line.c_str(), _tcslen(line.c_str())*sizeof(TCHAR), &dw, NULL);
	}
	::CloseHandle(hFile);
	return TRUE;
}

// 文字列の取得
//   row     [in]: 行
//   col     [in]: 列
//   戻り値 [out]: テキスト
const TCHAR *CGrCSVText::GetText(int row, int col)
{
	if(static_cast<int>(m_data.size()) <= row){
		return nullptr;
	}
	const auto &csv_colmn = m_data[row];
	if(static_cast<int>(csv_colmn.size()) <= col){
		return nullptr;
	}
	return csv_colmn[col].c_str();
}

// 文字列の取得
//   row     [in]: 行
//   col     [in]: 列
//   戻り値 [out]: string
std::tstring *CGrCSVText::GetString(int row, int col)
{
	if(row < 0 || static_cast<int>(m_data.size()) <= row){
		return nullptr;
	}
	auto &csv_colmn = m_data[row];
	if(col < 0 || static_cast<int>(csv_colmn.size()) <= col){
		return nullptr;
	}
	return &(csv_colmn[col]);
}

// 数字の取得
//   row     [in]: 行
//   col     [in]: 列
//   def     [in]: データがなかった場合の値
//   戻り値 [out]: 整数
int CGrCSVText::GetInteger(int row, int col, int def)
{
	const auto *pstr = GetString(row, col);
	if(!pstr){
		return def;
	}
	return _ttoi(pstr->c_str());
}

// 文字列の設定(列が存在しない場合は追加)
//   row [in]: 行
//   col [in]: 列
//   str [in]: 文字列
void CGrCSVText::SetTextExAdd(int row, int col, std::tstring str)
{
	if(static_cast<int>(m_data.size()) <= row){
		m_data.resize(row+1);
	}
	auto &csv_colmn = m_data[row];
	for(int i=static_cast<int>(csv_colmn.size()); i<=col; ++i){
		csv_colmn.push_back(_T(""));
	}
	csv_colmn[col] = str;
}

// 整数の設定(列が存在しない場合は追加)
//   row [in]: 行
//   col [in]: 列
//   val [in]: 整数
void CGrCSVText::SetIntegerExAdd(int row, int col, int val)
{
	std::tstring str;
	CGrText::itoa(val, str);
	SetTextExAdd(row, col, str);
}

// 列を追加します。
//   line [in]: 追加する文字列
void CGrCSVText::AddTail(LPCTSTR line)
{
	CSV_COLMN csv_colmn;

	toColumns(csv_colmn, line, m_splitChar);
	m_data.push_back(csv_colmn);
}

void CGrCSVText::AddFormatTail(LPCTSTR fmt, ...)
{
	CSV_COLMN csv_colmn;
	va_list argptr;
	va_start(argptr, fmt);
	for(;*fmt; ++fmt){
		switch(*fmt){
		case 's': csv_colmn.push_back(std::tstring(va_arg(argptr, LPCTSTR))); break;
		case 'd':
			{
				std::tstring str;
				CGrText::itoa(va_arg(argptr, int), str);
				csv_colmn.push_back(str);
			}
			break;
		}
	}
	va_end(argptr);
	m_data.push_back(csv_colmn);
}

//   row  [in]: 挿入する行
//   line [in]: 追加する文字列
void CGrCSVText::Insert(int row, LPCTSTR line)
{
	if(row < static_cast<int>(m_data.size())){
		CSV_COLMN csv_colmn;
		toColumns(csv_colmn, line, m_splitChar);
		auto it_row=m_data.begin()+row;
		m_data.insert(it_row, csv_colmn);
	} else {
		AddTail(line);
	}
}

// CSV形式で文字列を設定します。
//   line   [out]: CSV文字列
//   row     [in]: 行
//   戻り値 [out]: 失敗時、FALSE
BOOL CGrCSVText::SetCSVText(LPCTSTR line, int row)
{
	if(static_cast<int>(m_data.size()) <= row){
		return FALSE;
	}
	m_data[row].clear();
	m_data[row].shrink_to_fit();
	toColumns(m_data[row], line, m_splitChar);
	return TRUE;
}

// CSV形式で文字列を取得します。
//   line   [out]: CSV文字列
//   row     [in]: 行
//   戻り値 [out]: 失敗時、FALSE
BOOL CGrCSVText::GetCSVText(std::tstring &line, int row)
{
	if(static_cast<int>(m_data.size()) <= row){
		return FALSE;
	}
	enquote(line, m_data[row]);
	return TRUE;
}

// ,が次に現れる位置を返します。
//   str     [in]: 文字列
//   戻り値 [out]:  ,が次に現れる位置
static LPCTSTR nextTokenPos(LPCTSTR str, TCHAR splitChar)
{
	if(!str){
		return nullptr;
	}
	bool bBeginDobuleQuotation = (*str == _T('"'));
	bool bInQuote = false;
	for(;*str; ++str){
		if(bBeginDobuleQuotation && *str == _T('"') && *(str+1) == _T('"')){
			++str;
			continue;
		}
		bInQuote = isInQuote(str, bInQuote, splitChar);
		if(!bInQuote && splitChar == *str){
			break;
		}
	}
	return str;
}

// 文字列をCSV形式で解析して、列に分けます。
//   csv_colmn [out]: 列
//   line       [in]: CSV文字列
void CGrCSVText::toColumns(CSV_COLMN &csv_colmn, LPCTSTR line, TCHAR splitChar)
{
	if(!line || _tcslen(line) <= 0){
		return;
	}

	auto pStPos = line, pNxPos = line;

	while(pNxPos && *pNxPos){
		std::tstring col_str;

		pNxPos = nextTokenPos(pStPos, splitChar);
		if(*pStPos == _T('"')){ ++pStPos; }
		for(;pStPos < pNxPos; ++pStPos){
			// "の場合は、連続で""となっている筈
			//   なってなければ付け加えない
			if(*pStPos == _T('"')){
				// 次の文字を見てみて"なら"を追加
				if((pStPos+1) < pNxPos && *(pStPos+1) == _T('"')){
					col_str += *pStPos;
					++pStPos;
				}
			} else {
				col_str += *pStPos;
			}
		}
		pStPos = pNxPos+1;
		csv_colmn.push_back(col_str);
	}
}

// 指定した列を、CSV文字列に変換
//   csv_colmn  [in]: 列
//   ret_str   [out]: CSV文字列
//   ["]   -> [""""]
//   [,]   -> [","]
//   [",a] -> [""",a"]
void CGrCSVText::enquote(std::tstring &ret_str, const CSV_COLMN &csv_colmn)
{
	auto col_num = csv_colmn.size();
	ret_str.erase();

	for(size_t col=0; col<col_num; ++col){
		const auto &col_str = csv_colmn[col];
		if(col != 0){
			ret_str += m_splitChar;
		}
		// '"'や',','\r','\n'が無ければ""でくくる必要は無い
		if(col_str.find_first_of(_T("\",\n\r")) == std::tstring::npos){
			ret_str += col_str;
			continue;
		}
		ret_str += _T('"');
		auto len = col_str.size();
		for(size_t idx=0; idx<len; ++idx){
			// "は ""に変換
			if(col_str[idx] == _T('"')){
				ret_str += col_str[idx];
			}
			ret_str += col_str[idx];
		}
		ret_str += _T('"');
	}
}

// 文字列を追加します。
//   line       [in]: 文字列
//   start_pos  [in]: 開始位置
//   last_pos   [in]: 終了位置
//   戻り値    [out]: 次開始位置
LPCTSTR CGrCSVText::addString(std::tstring &line, LPCTSTR start_pos, LPCTSTR last_pos)
{
	if(*last_pos == _T('\0')){
		return nullptr;
	}
	auto lpEnd = last_pos;
	if((last_pos-1) >= start_pos && IS_CR(*(last_pos-1))){
		--lpEnd;
	}
	line.append(start_pos, (lpEnd-start_pos));
	AddTail(line.c_str());
	line.erase();

	return last_pos + 1;
}

// 行の削除
//   row [in]: 行
void CGrCSVText::Remove(int row)
{
	if(row < 0 || row >= static_cast<int>(m_data.size())){
		return;
	}
	m_data.erase(m_data.begin() + row);
}

// CSVデータを追加します。
//   data  [in]: データ
void CGrCSVText::AddTailCSVData(const TCHAR *data)
{
	std::tstring line;
	bool bInQuote = false;
	bool cr = false;

	auto start_pos = data, last_pos = data;
	for(;*last_pos; ++last_pos){
		bInQuote = isInQuote(last_pos, bInQuote, m_splitChar);
		if(!bInQuote){
			if(IS_LF(*last_pos) || cr){
				start_pos = addString(line, start_pos, last_pos);
			}
			cr = IS_CR(*last_pos);
		} else {
			cr = FALSE;
		}
	}
	if(start_pos && *start_pos){
		line += start_pos;
	}
	if(!line.empty()){
		AddTail(line.c_str());
	}
}

CGrCSVText& CGrCSVText::operator=(const CGrCSVText& csvt)
{
	m_data.resize(csvt.m_data.size());
	const auto *pFromCol = &(csvt.m_data[0]);
	auto *pToCol = &(m_data[0]);
	for(int row=csvt.m_data.size(); row>0; --row, ++pFromCol, ++pToCol){
		pToCol->resize(pFromCol->size());
		const auto *pFromStr = &((*pFromCol)[0]);
		auto *pToStr = &((*pToCol)[0]);
		for(int col=pFromCol->size(); col>0; --col, ++pFromStr, ++pToStr){
			*pToStr = *pFromStr;
		}
	}
	return *this;
}

// 指定行を整数の配列で取得
//   row    [in]: 行
//   array [out]: 整数の配列
//   len    [in]: arrayの数
bool CGrCSVText::toIntArray(int row, int array[], int len) const
{
	return toIntArray(row, 0, array, len);
}

//   row    [in]: 行
//   col    [in]: 列
//   array [out]: 整数の配列
//   len    [in]: arrayの数
bool CGrCSVText::toIntArray(int row, int col, int array[], int len) const
{
	if(row < 0 || static_cast<int>(m_data.size()) <= row){
		return false;
	}
	const auto &csv_colmn = m_data[row];
	if(col < 0 || static_cast<int>(csv_colmn.size()) < col + len){
		return false;
	}
	int idx=0;
	for(; idx<len; ++idx, ++col){
		array[idx] = CGrText::toInt(csv_colmn[col]);
	}
	return true;
}

void CGrCSVText::Sort(SortCompare &sc)
{
	if(m_data.size() > 1){
		std::sort(std::begin(m_data), std::end(m_data), [&](const std::vector<std::tstring> &c1, const std::vector<std::tstring> &c2){
			return sc(c1, c2);
		});
	}
}

void CGrCSVText::ForEachAll(EachFunc &ef)
{
	for(auto &&item : m_data){
		if(!ef(item)){
			return;
		}
	}
}
