#ifndef __BOOKMARKDB_H__
#define __BOOKMARKDB_H__

#include <memory>
#include <vector>
#include <list>
#include <string>
#include "stltchar.h"

#define TXTMIRU _T("TxtMiru")
#define NULL_INT_VALUE (-999)

struct Place {
	int          id             = 0;
	std::tstring url            ;
	std::tstring title          ;
	std::tstring author         ;
	int          page           = 0;
	int          read_page      = 0;
	int          visit_count    = 0;
	std::tstring insert_date    ;
	std::tstring last_visit_date;
};
struct History {
	int          id             = 0;
	int          place_id       = 0;
	std::tstring url            ;
	std::tstring title          ;
	std::tstring author         ;
	int          page           = 0;
	int          read_page      = 0;
	std::tstring insert_date    ;
	std::tstring last_visit_date;
};
struct Book {
	int          id             = 0;
	int          place_id       = 0;
	int          parent         = 0; // NULL IS TOP ITEM
	int          type           = 0; // 0:ITEM, 1:FOLDER
	int          position       = 0;
	std::tstring base_url       ;
	std::tstring title          ;
	std::tstring author         ;
	std::tstring insert_date    ;
};
struct AozoraBooks {
	int          id               = 0;
	std::tstring title            ; // 作品名
	std::tstring title_r          ; // 作品名読み
	std::tstring title_s          ; // ソート用読み
	std::tstring subtitle         ; // 副題
	std::tstring subtitle_r       ; // 副題読み
	std::tstring orgtitle         ; // 原題
	std::tstring note_f           ; // 初出
	std::tstring classification   ; // 分類番号
	std::tstring char_type        ; // 文字遣い種別
	std::tstring release_date     ; // 公開日
	std::tstring family_name      ; // 姓
	std::tstring given_name       ; // 名
	std::tstring family_name_r    ; // 姓読み
	std::tstring given_name_r     ; // 名読み
	std::tstring family_name_s    ; // 姓読みソート用
	std::tstring given_name_s     ; // 名読みソート用
	std::tstring family_name_romen; // 姓ローマ字
	std::tstring given_name_romen ; // 名ローマ字
	std::tstring role_flag        ; // 役割フラグ
	std::tstring html_url         ; // XHTML/HTMLファイルURL
	std::tstring html_update_date ; // XHTML/HTMLファイル最終更新日
	std::tstring insert_date      ;
	bool bBookmark                = false; // お気に入りに追加
};
struct AozoraFirstCharBooks {
	std::tstring first_char;
	int count = 0;
};

namespace CGrDBFunc
{
	void GetSysDate(std::tstring &out_sysdate);
	void GetSysNextDate(std::tstring &out_sysdate, int iDays);
};

namespace CGrTxtFunc
{
	void GetBookmarkFolder(std::tstring &str);
};

class CGrBookmarkDB
{
public:
	CGrBookmarkDB();
	virtual ~CGrBookmarkDB();

	bool Close();
	bool Open();
	bool Open(LPCTSTR lpFileName);
	bool OpenNew();

	int BeginSession();
	int Commit();
	int Rollback();

	bool Put(AozoraBooks &book);
	bool GetClassCount(LPCTSTR in_classification, int &out_count);
	bool GetClassList(LPCTSTR in_classification, std::vector<AozoraBooks> &book_list);
	bool GetFamilyNameSList(LPCTSTR in_family_name_s, std::vector<AozoraBooks> &book_list);
	bool GetFamilyName1List(std::vector<AozoraFirstCharBooks> &book_list);
	bool GetTitleSList(LPCTSTR in_title_s, std::vector<AozoraBooks> &book_list);
	bool GetTitleS1List(std::vector<AozoraFirstCharBooks> &book_list);
	bool GetCount(int &out_count);
	bool GetLatestCount(int &out_count);
	bool GetLatestList(std::vector<AozoraBooks> &book_list);
	bool GetSearchWord(LPCTSTR lpWords, std::vector<AozoraBooks> &book_list);
	bool DeleteAozoraBooks();
private:
	class CGrBookmarkDBImpl;
	std::shared_ptr<CGrBookmarkDBImpl> m_pimpl;
};

#endif // __BOOKMARKDB_H__
