#pragma once
/* [=============================== Includes ===============================] */
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/glcanvas.h>
#include <wx/graphics.h>
#include <wx/matrix.h>
#include <array>
#include <execution>
#include <algorithm>
#include <mutex>
#include <cctype>

#include "../../Framework/Framework.h"
#include "../ProgramSlider.h"

/* [=============================== TextEditor ===============================] */

class TextEditor : 
	public wxPanel
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
		wxColor _LayoutBufferChunkColor; 
		wxColor _LayoutBufferChunkOutlineColor;
		wxColor _LayoutInfoBackgroundColor;
		wxColor _LayoutInfoFontColor; 
		wxColor _LayoutInfoSeparatorColor;
		wxColor _LayoutInfoElementCommentColor; 
		wxColor _LayoutInfoElementActiveBackgroundColor;

		std::string _LayoutInfoEndlineStandardCurrent = "CRLF";

		std::vector<std::string> _Buffer; 

		std::vector<size_t> _LineIndexingActivatedLines;
		bool _LineIndexingActivatedLinesIncludeBG = false;
		bool _CursorActive = true;

		size_t _BufferActivatedLine = 0; 

		int _LayoutLineIndexingSpacingXStart = 10;
		int _LayoutLineIndexingSpacingXEnd = 18;
		int _LayoutScrollbarsAxisLength = 10; 
		int m_LineIndexingSpacingY = 8;
		int _ScrollbarsCheckTimerMs = 5; 
		int _SmartRefreshUpdateTimerMs = 5; 
		int m_BufferSpacingY = 8;
		int _MouseWheelGlobalActionsVerticalScrollLinesPerWheelTick = 2;
		int _LayoutCursorOutlineSize = 1; 
		int _LayoutCursorSizeX = 2; 
		int _LayoutCursorTabLength = 2; 
		int _LayoutLineIndexingSpacingY = 8;
		int _LayoutBufferSpacingY = 8;
		int _LayoutInfoSpacingY = 10;
		int _LayoutInfoSeparatorY = 1; 
		int _LayoutInfoElementSpacingX = 4; 
		int _LayoutInfoElementCommentSpacingX = 12; 

		std::string _LayoutInfoCharSystem = "ASCII";
		
		std::array<int, 2> _BufferScroll = { 0, 0 };
		std::array<size_t, 2> _CursorAt = { 0, 0 };
		
		wxFont _LayoutLinesIndexingFont;
		wxFont _LayoutBufferFont; 
		wxFont _LayoutInfoFont; 

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
			std::array<int, 4> _InfoBB; 

			Framework::Geometry::BoundingBox g_BBFromPackedArray(
				std::array<int, 4>& _Arr
			) const
			{
				return { wxPoint(_Arr[0], _Arr[1]), wxPoint(_Arr[2], _Arr[3]) };
			}

            std::array<int, 4> g_PackedArrayFromBB(
                Framework::Geometry::BoundingBox& _BB
            ) {
                const wxPoint _Start = _BB.gStarting();
                const wxPoint _End = _BB.gFinal();
                return {
                    _Start.x, _Start.y, _End.x, _End.y
                };
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

		void hdl_LineIndexingSetupClippingRegion(
			wxAutoBufferedPaintDC& _Canvas);

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
		void hdl_ScrollbarsUpdatePositions();
		void hdl_ScrollbarsVisibility();

		wxTimer m_ScrollbarsCheckTimer;

		ProgramSlider* m_ScrollbarsVertical;
		ProgramSlider* m_ScrollbarsHorizontal;
	/* [=============================== _Buffer ===============================] */
		void hk_BufferRender(wxAutoBufferedPaintDC& _Canvas);
		void hdl_BufferSetupClippingRegion(wxAutoBufferedPaintDC& _Canvas);

		void hk_BufferRenderBackground(wxAutoBufferedPaintDC& _Canvas);
		void hk_BufferRenderText(wxAutoBufferedPaintDC& _Canvas);

		void hdl_BufferRenderTextReal(wxAutoBufferedPaintDC& _Canvas);

		void hdl_BufferRenderTextBlocks(wxAutoBufferedPaintDC& _Canvas);
		void hdl_BufferRenderTextBlocksRenderChunks(wxAutoBufferedPaintDC& _Canvas, 
			const std::vector<std::array<size_t, 2>>& _Chunks, const size_t& _ChunkSize);

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
        const std::array<size_t, 2> g_BufferLineDataAtGivenPosition(
            const wxPoint& _Position
        );

		const size_t g_BufferChunkLength(const std::array<size_t, 2>& _Range);
		const std::vector<std::array<size_t, 2>> g_BufferGeneratedChunks(
			const std::array<size_t, 2> _RenderingRange, const size_t& _ChunkAmount, const size_t& _ChunkSize
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

        void hk_CursorPlace(wxMouseEvent& _Event);

		void hk_CursorRender(wxAutoBufferedPaintDC& _Canvas);
		void hk_CursorRenderObject(wxAutoBufferedPaintDC& _Canvas);
		void hdl_CursorSetupClippingRegion(wxAutoBufferedPaintDC& _Canvas);

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
            m_Global = 4,
			m_Info = 5
		};

		void init_SmartRefresh();
		
		void hk_SmartRefreshUpdate(
			wxTimerEvent& _Event);
        void hdl_SmartRefreshUpdate();

        void p_SmartRefreshObjects(
            const std::vector<m_SmartRefreshObjects>& _Objects
        );

		wxTimer m_SmartRefreshUpdateTimer;
		std::vector<m_SmartRefreshObjects> m_SmartRefreshBuffer;
	/* [=============================== _Info ===============================] */
		void init_Info(); 
		void hdl_InfoSetupElements(); 

		struct m_InfoElement
		{
			enum m_LayoutPositions
			{
				m_FromLeft = 0, 
				m_FromRight = 1
			};
			size_t m_CountForCurrentLayout;

			m_LayoutPositions m_Layout; 
			std::string m_Text;
			std::string m_Comment; 

			size_t m_Index;
		};

		void hk_InfoRender(
			wxAutoBufferedPaintDC& _Canvas); 

		void hk_InfoRenderBackground(wxAutoBufferedPaintDC& _Canvas);
		void hk_InfoRenderSeparator(wxAutoBufferedPaintDC& _Canvas);
		void hk_InfoRenderComment(wxAutoBufferedPaintDC& _Canvas);

		void hdl_InfoDeactivateElement();
		void hk_InfoActivateElement(wxMouseEvent& _Event);

		void hk_InfoRenderElements(wxAutoBufferedPaintDC& _Canvas);
		void hdl_InfoRenderElement(wxAutoBufferedPaintDC& _Canvas, const m_InfoElement& _Element);

		void p_InfoElement(
			const std::string& _Text, const std::string& _Comment, 
			m_InfoElement::m_LayoutPositions _LayoutPosition
		);

		const size_t g_InfoElementCurrentLayoutCount(m_InfoElement::m_LayoutPositions _CurrentLayout) const;
		
		const wxPoint g_InfoElementCenteredTextPosition(const m_InfoElement& _Element);
		const wxPoint g_InfoElementCommentCenteredTextPosition(const m_InfoElement& _Element);

		Framework::Geometry::BoundingBox g_InfoElementBB(const m_InfoElement& _Element); 
		Framework::Geometry::BoundingBox g_InfoSeparatorBB();
		Framework::Geometry::BoundingBox g_InfoElementCommentBB(const m_InfoElement& _Element);

		const int g_InfoElementPushX(const m_InfoElement& _Element);

		std::vector<m_InfoElement> m_InfoElements;
		size_t m_InfoElementActivated = -1;
	/* [=============================== _MouseMotionHook ===============================] */
		void init_MouseMotionHook();

		void hk_MouseMotion(
			wxMouseEvent& _Event);

};

