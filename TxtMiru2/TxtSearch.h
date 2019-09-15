#ifndef __TXTSEARCH_H__
#define __TXTSEARCH_H__

class CGrTxtDocument;
class CGrTxtSearch
{
private:
	std::tstring m_str;
	TxtMiru::TextPoint m_pos;
	WORD m_searchCharType = 0;
	int m_len = 0;
	bool m_bLoop = true;
	bool m_bUseRegExp = false;
	bool m_bDown = true;
	const CGrTxtDocument *m_pDoc = nullptr;
	bool upSearch(TxtMiru::TextPoint &pos);
	bool downSearch(TxtMiru::TextPoint &pos);
	bool next(TxtMiru::TextPoint &pos, bool bDown);
public:
	CGrTxtSearch();
	virtual ~CGrTxtSearch();
	void Attach(const CGrTxtDocument *pDoc){ m_pDoc = pDoc; }
	void Set(const TxtMiru::TextPoint &pos, LPCTSTR lpSrc, bool bLoop, bool bUseRegExp, bool bDown);
	bool Search(TxtMiru::TextPoint &pos, LPCTSTR lpSrc, bool bLoop, bool bUseRegExp, bool bDown);
	int StringLength() const { return m_len; }
	void Clear();
	bool Next(TxtMiru::TextPoint &pos);
	bool Prev(TxtMiru::TextPoint &pos);
};

#endif // __TXTSEARCH_H__
