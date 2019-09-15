#ifndef __WAITCURSOR_H__
#define __WAITCURSOR_H__

class CGrWaitCursor
{
	HCURSOR m_hCursor;
public:
	CGrWaitCursor(){
		m_hCursor = SetCursor(LoadCursor(0, IDC_WAIT));
	};
	virtual ~CGrWaitCursor(){
		SetCursor(m_hCursor);
	};
};

#endif // __WAITCURSOR_H__
