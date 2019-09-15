#ifndef __MOUSEEVENT_H__
#define __MOUSEEVENT_H__

class CGrMainWindow;
class CGrMouseEvent
{
public:
	using ClickFunc = void (CGrMainWindow::*)();
private:
	enum {
		ID_TIMER_MOUSEBTN = 150 , // Mouse Key Repeat
		ID_TIMER_MOUSEBTN_SINGLE, // Mouse Key Single Click
	};
	HWND &m_hWnd;
	CGrMainWindow *m_pMainWindow = nullptr;
	UINT m_vk = 0;
	int m_count = 0;
	int m_clicks = 0;
	bool m_longclick = false;
	bool m_press = false;
	const UINT m_single_click_timer_id = ID_TIMER_MOUSEBTN_SINGLE;
	const UINT m_long_press_timer_id = ID_TIMER_MOUSEBTN;
	UINT m_long_press_elapse = 1000;
	ClickFunc m_pSingleClick = nullptr;
	ClickFunc m_pSingleLongPress = nullptr;
	ClickFunc m_pDoubleClick = nullptr;
	ClickFunc m_pDoubleLongPress = nullptr;
public:
	CGrMouseEvent(HWND &hWnd, CGrMainWindow *pMainWindow) : m_hWnd(hWnd), m_pMainWindow(pMainWindow)
	{
	}
	//
	bool IsNoBind()
	{
		return m_count == 0;
	}
	bool IsExistDelayEvent()
	{
		return (m_long_press_elapse > 0 && (m_pSingleLongPress || m_pDoubleLongPress));
	}
	//
	void SetFunction(ClickFunc pSingleClick, ClickFunc pSingleLongPress, ClickFunc pDoubleClick, ClickFunc pDoubleLongPress)
	{
		m_pSingleClick     = pSingleClick;
		m_pSingleLongPress = pSingleLongPress;
		m_pDoubleClick     = pDoubleClick;
		m_pDoubleLongPress = pDoubleLongPress;
	}
	void SetLongPressTime(UINT uElapse)
	{
		m_long_press_elapse = GetDoubleClickTime() > uElapse ? 0 : uElapse;
	}
	//
	void Clear()
	{
		m_count = 0;
		m_clicks = 0;
		m_longclick = false;
		m_press = false;
		StopSingleClickTimer();
		StopLongPressTimer();
	}
	// Event
	void OnButtonDown(UINT vk)
	{
		if (m_vk != vk) {
			Clear();
		}
		m_vk = vk;
		m_longclick = false;
		m_press = true;
		if (m_pDoubleClick || m_pDoubleLongPress) {
			if (m_count == 0) {
				m_clicks = 0;
				StartSingleClickTimer();
				if (m_pSingleLongPress) {
					StartLongPressTimer();
				}
			}
			else if (m_count == 1) {
				StopSingleClickTimer();
				m_clicks = 2;
				if (m_pDoubleLongPress) {
					StartLongPressTimer();
				} else if(m_pDoubleClick){
					DoubleClick();
					return;
				}
			}
		}
		else if (m_pSingleLongPress) {
			m_clicks = 1;
			StartLongPressTimer();
		}
		else {
			SingleClick();
			return;
		}
		++m_count;
	}
	void OnButtonUp(UINT vk)
	{
		m_press = false;
		if (m_vk != vk) {
			Clear();
		}
		else if (!m_longclick) {
			if (m_clicks == 1) {
				SingleClick();
			}
			else if (m_clicks == 2) {
				if(m_pDoubleClick) {
					DoubleClick();
				} else {
					SingleClick();
					SingleClick();
				}
			}
		}
	}
	void MouseTimer(UINT id)
	{
		KillTimer(m_hWnd, id);
		if (id == m_single_click_timer_id) {
			m_clicks = 1;
			if (!m_press) {
				SingleClick();
			}
		}
		else if (id == m_long_press_timer_id) {
			m_longclick = true;
			if (m_clicks == 1) {
				SingleLongPress();
			}
			else if (m_clicks == 2) {
				DoubleLongPress();
			}
		}
	}
private:
	void fireEvent(ClickFunc func)
	{
		Clear();
		if (func) {
			(m_pMainWindow->*func)();
		}
	}
	void SingleClick()
	{
		fireEvent(m_pSingleClick);
	}
	void SingleLongPress()
	{
		fireEvent(m_pSingleLongPress);
	}
	void DoubleClick()
	{
		fireEvent(m_pDoubleClick);
	}
	void DoubleLongPress()
	{
		fireEvent(m_pDoubleLongPress);
	}

	void StopSingleClickTimer()
	{
		KillTimer(m_hWnd, m_single_click_timer_id);
	}
	void StartSingleClickTimer()
	{
		SetTimer(m_hWnd, m_single_click_timer_id, GetDoubleClickTime(), nullptr);
	}
	void StopLongPressTimer()
	{
		KillTimer(m_hWnd, m_long_press_timer_id);
	}
	void StartLongPressTimer()
	{
		if (m_long_press_elapse > 0) {
			SetTimer(m_hWnd, m_long_press_timer_id, m_long_press_elapse, nullptr);
		}
	}
};

#endif // __MOUSEEVENT_H__
