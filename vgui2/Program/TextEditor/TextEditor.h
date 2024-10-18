#pragma once
/* [=============================== Includes ===============================] */
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <array>
#include <execution>
#include <algorithm>
#include <mutex>
#include <cctype>

#include "../../Framework/Framework.h"
#include "../ProgramSlider.h"

/* [=============================== TextEditor ===============================] */

class TextEditor : 
	wxPanel
{ public: 
	/* [=============================== Core ===============================] */
	struct _CoreList
	{
		wxPanel* _Parent;
		Framework::Debug* _Debug;

		wxPoint _Position;
		wxSize _Size;

		wxColor _LayoutBackgroundColor;
		wxColor _LayoutLineIndexingBackgroundRegularColor;
		wxColor _LayoutLineIndexingFontColorDeActivated;
		wxColor _LayoutLineIndexingFontColorActivated;
		wxColor _LayoutLineIndexingBackgroundSelectionColor;
		wxColor _LayoutScrollbarsBackgroundColor;
		wxColor _LayoutScrollbarsElementColorDeActivated;
		wxColor _LayoutBufferBackgroundColor;
		wxColor _LayoutBufferFontColor;
		wxColor _LayoutBufferActivatedLineColor;
		wxColor _LayoutCursorColor;
		wxColor _LayoutCursorOutlineColor;

		std::vector<std::string> _Buffer; 

		std::vector<size_t> _LineIndexingActivatedLines;
		bool _LineIndexingActivatedLinesIncludeBG = false;
		bool _CursorActive = true;

		size_t _BufferActivatedLine = 0; 

		int _LayoutLineIndexingSpacingXStart = 10;
		int _LayoutLineIndexingSpacingXEnd = 18;
		int _LayoutScrollbarsAxisLength = 10; 
		int _LayoutLineIndexingSpacingY = 8;
		int _ScrollbarsCheckTimerMs = 5; 
		int _SmartRefreshUpdateTimerMs = 5; 
		int _LayoutBufferSpacingY = 8;
		int _MouseWheelGlobalActionsVerticalScrollLinesPerWheelTick = 2;
		int _LayoutCursorOutlineSize = 1; 
		int _LayoutCursorSizeX = 2; 
		int _LayoutCursorTabLength = 2; 
		
		std::array<int, 2> _BufferScroll = { 0, 0 };
		std::array<size_t, 2> _CursorAt = { 0, 0 };
		
		wxFont _LayoutLinesIndexingFont;
		wxFont _LayoutBufferFont; 
	};

	/* [=============================== Initializer ===============================] */
		TextEditor(
			_CoreList& cr_List
		);
	
	_CoreList cr_List;
private:
	/* [=============================== _LayoutPositions ===============================] */
		struct _LayoutPositions
		{ 
			std::array<int, 4> _LineIndexingBB;
			std::array<int, 4> _BufferBB;
			std::array<int, 4> _VerticalScrollbarBB;
			std::array<int, 4> _HorizontalScrollbarBB;

			Framework::Geometry::BoundingBox g_BBFromPackedArray(
				std::array<int, 4>& _Arr
			) const
			{
				return { wxPoint(_Arr[0], _Arr[1]), wxPoint(_Arr[2], _Arr[3]) };
			}
		};

		_LayoutPositions g_LayoutPositions();
	/* [=============================== _RenderHook ===============================] */
		void init_hk_Render();

		void hk_Render(
			wxPaintEvent& _event
		);
	/* [=============================== _Background ===============================] */
		void hk_BackgroundRender(wxAutoBufferedPaintDC& _Canvas);
	/* [=============================== _LineIndexing ===============================] */
		void hk_LineIndexingRender(wxAutoBufferedPaintDC& _Canvas);

		void hk_LineIndexingRenderBackground(wxAutoBufferedPaintDC& _Canvas);
		void hk_LineIndexingRenderNumbers(wxAutoBufferedPaintDC& _Canvas);

		Framework::Geometry::BoundingBox g_LineIndexingGivenAtBB(
			const size_t& _Where
		);

		const wxPoint g_LineIndexingNumberCenteredPosition(
			const wxPoint& _BBFinalPosition, const std::string& _Number
		) const;

		const std::array<size_t, 2> g_LineIndexingRenderingRange();
	/* [=============================== _Scrollbars ===============================] */
		void init_Scrollbars(); 
		void hdl_ScrollbarsInstanceSetup(); 
		void hdl_ScrollbarsSetupCheckHK();
		void hk_ScrollbarsCheck(wxTimerEvent& _Event);

		wxTimer m_ScrollbarsCheckTimer;

		ProgramSlider* m_ScrollbarsVertical;
		ProgramSlider* m_ScrollbarsHorizontal;
	/* [=============================== _Buffer ===============================] */
		void hk_BufferRender(wxAutoBufferedPaintDC& _Canvas);
		void hk_BufferRenderBackground(wxAutoBufferedPaintDC& _Canvas);
		void hk_BufferRenderText(wxAutoBufferedPaintDC& _Canvas);

		const std::array<int, 2> g_BufferMaximalScroll();
		const std::array<size_t, 2> g_BufferRenderingRange(); 
		
		const size_t g_BufferMaximalCharsVisible();
		const size_t g_BufferMaximalLinesVisible();
		const size_t g_BufferCharSizeX() const; 

		const wxPoint g_BufferTextCenteredPosition(const size_t& _Where);

		Framework::Geometry::BoundingBox g_BufferLineBBAtGiven(
			const size_t& _Where
		);
		Framework::Geometry::BoundingBox g_BufferLineBBAtGivenLineAndChar(
			const size_t& _WhereLine, const size_t& _WhereChar
		);
		
		const int g_BufferLineSizeY() const;
	/* [=============================== _MouseWheelGlobalActions ===============================] */
		void init_MouseWheelGlobalActions();

		void hk_MouseWheelGlobalActionsMouseWheelEvent(
			wxMouseEvent& _Event);

		void hk_MouseWheelGlobalActionsResizeFonts(
			wxMouseEvent& _Event);
		void hk_MouseWheelGlobalActionsVerticalScroll(
			wxMouseEvent& _Event);
	/* [=============================== _Cursor ===============================] */
		enum m_CursorArrowsDirection
		{
			m_Forward = 0,
			m_Backward = 1
		};

		void init_Cursor(); 

		void hk_CursorRender(wxAutoBufferedPaintDC& _Canvas);

		void hk_CursorType(wxKeyEvent& _Event, const std::string& _Char);
		void hk_CursorTypeTabulator(wxKeyEvent& _Event);

		void hk_CursorDelete(wxKeyEvent& _Event);

		void hk_CursorEnter(wxKeyEvent& _Event);

		void hk_CursorArrows(wxKeyEvent& _Event);
		void hdl_CursorHorizontalArrows(wxKeyEvent& _Event);
		void hdl_CursorVerticalArrows(wxKeyEvent& _Event);

		void hdl_CursorHorizontalArrowsMoveToNextLine(
			size_t& _AtLine, size_t& _AtChar, const std::vector<std::string>& _Buffer
		);

		void hdl_CursorHorizontalArrowsMoveToPreviousLine(
			size_t& _AtLine, size_t& _AtChar, const std::vector<std::string>& _Buffer
		);

		void hdl_CursorVerticalArrowsMove(
			size_t& _AtLine, size_t& _AtChar, const std::vector<std::string>& _Buffer, m_CursorArrowsDirection _Direction
		);

		const bool hdl_CursorHorizontalArrowsJump(m_CursorArrowsDirection _Direction);

		void hdl_CursorInsertIntoBuffer(const char& _Char, const size_t& _Count);
		void hdl_CursorDeleteAfter(const size_t& _Count, const bool& _DeletedLine);
		void hdl_CursorInsertIntoBufferAfter(const size_t& _Count);
		void hdl_CursorEnterAfter(const bool& _NewLineWasAdded);
		void hdl_CursorNewLine();
		void hdl_CursorStayInPlace();

		void hdl_CursorGlobalChange();
		void hdl_CursorGlobalChangeModifyBufferScroll(); /* NEEDS REFACTORING : @SPAGGETI STYLE ALERT */

		const bool g_CursorIsPlaced() const; 
		const size_t g_CursorDeletionEraseCount(const bool& _IsCtrlDown) const;
		const size_t g_CursorTabulationCount(const std::string& _BufferLine) const; 
		const std::string g_CursorTabulationInStr(const size_t& _Count) const; 

		const size_t g_CursorJumpingPosition(
			const std::string& _Line, const size_t& _At, m_CursorArrowsDirection _Direction
		) const;

		Framework::Geometry::BoundingBox g_CursorBB();
		Framework::Geometry::BoundingBox g_CursorOutlineBB();
	/* [=============================== _OnChar ===============================] */
		enum m_OnCharKeys
		{
			m_Backspace = WXK_BACK,
			m_Tab = WXK_TAB,
			m_Enter = WXK_RETURN,
			m_Shift = WXK_SHIFT,
			m_Control = WXK_CONTROL,
			m_Alt = WXK_ALT,
			m_Escape = WXK_ESCAPE,
			m_Space = WXK_SPACE,
			m_LeftArrow = WXK_LEFT,
			m_RightArrow = WXK_RIGHT,
			m_UpArrow = WXK_UP,
			m_DownArrow = WXK_DOWN,
			m_Delete = WXK_DELETE,
			m_Insert = WXK_INSERT,
			m_Home = WXK_HOME,
			m_End = WXK_END,
			m_PageUp = WXK_PAGEUP,
			m_PageDown = WXK_PAGEDOWN,
			m_F1 = WXK_F1,
			m_F2 = WXK_F2,
			m_F3 = WXK_F3,
			m_F4 = WXK_F4,
			m_F5 = WXK_F5,
			m_F6 = WXK_F6,
			m_F7 = WXK_F7,
			m_F8 = WXK_F8,
			m_F9 = WXK_F9,
			m_F10 = WXK_F10,
			m_F11 = WXK_F11,
			m_F12 = WXK_F12
		};

		void init_OnChar();

		void hk_OnChar(wxKeyEvent& _Event); 

		const std::string g_OnCharRealChar(const int& _Unicode, const bool& _ShiftIsDown) const;
	/* [=============================== _SmartRefresh ===============================] */
		enum m_SmartRefreshObjects
		{
			m_Buffer = 1,
			m_LineIndexing = 2,
			m_Cursor = 3,
		};

		void init_SmartRefresh();
		
		void hk_SmartRefreshUpdate(
			wxTimerEvent& _Event);
		void hdl_SmartRefreshPerformUpdate();

		wxTimer m_SmartRefreshUpdateTimer;
		std::vector<m_SmartRefreshObjects> m_SmartRefreshBuffer;


};

