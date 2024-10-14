#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include "ProgramSlider.h"
#include "../Framework/Framework.h"
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <wx/event.h>

#define _DefaultColorEscape "\\aDEFCLR"

wxDEFINE_EVENT(_SmartRefreshBeforeUpdate, wxCommandEvent);

class ProgramBuffer :
	public wxPanel
{ public:
	/*
		On Startup 
	*/
	ProgramBuffer(
		wxPanel* _PARENT, Framework::Debug* _DEBUG, const wxPoint& _POSITION, const wxSize& _SIZE,
		const wxColor& _BG_COLOR, const wxColor& _SLIDERS_COLOR, const wxColor& _DEFAULT_TEXT_COLOR
	); 

	struct BufferElement
	{ public: 
		std::string _TEXT; 
		std::string _TEXT_RAW; 
	};

	/*
		Set (public functions) 
	*/
	const void pNextLine(
		const std::string& _LINE
	);

	const void mLine(
		const size_t& _AT, const std::string& _LINE
	); 

	enum Mode
	{ 
		BufferEdit = 0,
		Terminal = 1
	};
	/* 
		Get (public function) 
	*/
	const bool gBufferDoesNeedUpdate();
	std::vector<ProgramBuffer::BufferElement>& gBufferData();

	void fSmartRefresh(
		const wxRect& _Area
	);
private:
	/*
		On Startup 
	*/
	const void lPanelParams(); 
	const void lSlidersPositions(); 
	const void lSliders(); 
	const void lTextFont(); 
	const void lPaintHook();
	const void lScrollUpdateHook(); 
	const void lMouseHooks(); 
	const void lOnCharHooks(); 
	const void lCursorBlinkHook();

	/*
		Hooks 
	*/
	void hPaint(wxPaintEvent& _EVT); 
	void hScrollUpdate(wxTimerEvent& _EVT); 
	void hMouseMotion(wxMouseEvent& _EVT); 
	void hMouseExit(wxMouseEvent& _EVT);
	void hMouseUpdate(wxTimerEvent& _EVT); // On timer to handle 'whole window' mouse operations (probably there is another way but im a old-habit fucker ;] )
	void hMouseLeftUp(wxMouseEvent& _EVT);
	void hOnChar(wxKeyEvent& _EVT);
	void hMouseDClick(wxMouseEvent& _EVT);
	void hCursorBlink(wxTimerEvent& _EVT);

	/*
		Render 
	*/
	const void rBackground(wxAutoBufferedPaintDC& _CANVAS);
	const void rBuffer(wxAutoBufferedPaintDC& _CANVAS);
	const void rSelection(wxAutoBufferedPaintDC& _CANVAS);
	const void rCursor(wxAutoBufferedPaintDC& _CANVAS);
	/* MODES */
		/* BufferEdit */
			const void rBufferEditBackground(wxAutoBufferedPaintDC& _CANVAS);
			const void rBufferEditNumbers(wxAutoBufferedPaintDC& _CANVAS);
			const void rBufferEditSeparator(wxAutoBufferedPaintDC& _CANVAS);
			
	/*
		Char 
	*/
	const void cBufferEditHandleTyping(
		const std::string& _CHAR);
	const void cBufferEditHandleDeletion(wxKeyEvent& _EVT);
	const void cBufferEditHandleNextLine();
	const void cBufferEditHandleCursorMovementWithArrows(wxKeyEvent& _EVT);

	const void cBufferEditHandleCursorMovementWithArrowsHandleXAxis(
		const int& _WANTED_LINE);
	const void cBufferEditHandleCursorMovementWithArrowsHandleYAxis(
		const int& _WANTED_CHAR);
	const void cHandleSelectionCopyToClipboard();
	const void cBufferEditSelectionDestroy();

	/*
		Default handlers 
	*/
	const void dSetScrollCorespondingToCursorPositionY();
	const void dSetScrollCorespondingToCursorPositionX();
	const void dSelectionExit();
	const void dCursorBlink(); 

	/*
		Mouse 
	*/
	const void mHandleSelectionStart(
		wxMouseEvent& _EVT
	);

	const void mHandleMouseExit(); 
	const void mHandleSelection();
	const void mHandleCursorPlacement(wxMouseEvent& _EVT); 
	const void mHandleSelectionAbort();
	/*
		Scroll update 
	*/

	const void suHandle();

	/*
		Get 
	*/
	Framework::Geometry::BoundingBox g_SLIDER_HORIZONTAL_BoundingBox() const; 
	Framework::Geometry::BoundingBox g_SLIDER_VERTICAL_BoundingBox() const; 
	Framework::Geometry::BoundingBox g_BufferRenderBoundingBox() const; 

	const int g_BufferLineSizeY() const;
	const int g_BufferLineSizeX(const std::string& _LINE) const; 

	const wxPoint g_BufferLinePositionInBoundingBox(const size_t& _AT) const;
	
	const wxPoint g_MaxScrolls() const;

	const int g_BufferLineAtGivenPosition(
		const wxPoint& _POSITION
	) const; 

	const size_t g_BufferLineCharAtGivenPosition(
		const wxPoint& _POSITION, const size_t& _AT, const bool _LEN_CHECK = true
	) const; 

	const int g_BufferEditReservedSpace() const; 
	Framework::Geometry::BoundingBox g_BufferRenderFullBoundingBox() const; 

	/*
		Instances 
	*/
	wxPanel* _PARENT; 
	Framework::Debug* _DEBUG; 

	ProgramSlider* _SLIDER_HORIZONTAL;
	ProgramSlider* _SLIDER_VERTICAL;

	wxTimer _SCROLL_UPDATE_TIMER; 
	wxTimer _MOUSE_UPDATE_TIMER;
	wxTimer _CURSOR_BLIK_TIMER;
	/* 
		Variables 
	*/
	wxPoint _POSITION;
	wxPoint _SLIDER_HORIZONTAL_POSITION;
	wxPoint _SLIDER_VERTICAL_POSITION;
	
	wxSize _SIZE; 
	wxSize _SLIDER_HORIZONTAL_SIZE; 
	wxSize _SLIDER_VERTICAL_SIZE; 

	wxColor _BG_COLOR; 
	wxColor _SLIDERS_COLOR; 
	wxColor _DEFAULT_TEXT_COLOR; 
	wxColor _SLIDERS_BG_COLOR = wxColor(21, 26, 28);
	wxColor _BUFFER_EDIT_NUMBERS_BG_COLOR = wxColor(30, 30, 30);
	wxColor _BUFFER_EDIT_NUMBERS_COLOR = wxColor(255, 255, 255);
	wxColor _BUFFER_EDIT_SEPARATOR_COLOR = wxColor(21, 26, 28);

	wxFont _TEXT_FONT; 

	int _SLIDERS_HEIGHT = 15;
	int _TEXT_SPACING_HEIGHT = 5; 

	double _SCROLL_X = 0; 
	double _SCROLL_Y = 0; 

	std::vector<ProgramBuffer::BufferElement> _DATA; // = Framework::File::Open("C:\\vgui2\\config.cfg").Read().gVector();

	bool _SELECTION = false; 
	bool _SELECTION_MOUSE_LAST_CHECKED_ACTIVE = true; 

	int _SELECTION_UPDATE_MS = 10; 

	int _SELECTION_STARTED_AT_LINE = -1;
	int _SELECTION_STARTED_AT_CHAR = -1; 
	int _SELECTION_ENDED_AT_LINE;
	int _SELECTION_ENDED_AT_CHAR;

	int _BUFFER_EDIT_RESERVED_SPACE_X = 0;
	int _BUFFER_EDIT_SEPARATOR_LEN_X = 2; 
	int _BUFFER_EDIT_SEPARATION_LEN_X = 8;
	int _BUFFER_EDIT_NUMBERS_SEPARATION_LEN_X = 10;

	int _CURSOR_AT_LINE = -1; 
	int _CURSOR_AT_CHAR = -1;

	bool _RENDER_CURSOR = true; 
	bool _BUFFER_UPDATED = true; 

	ProgramBuffer::Mode _MODE = ProgramBuffer::Mode::BufferEdit;

	/* [ =================================== MODERN CODE =================================== ] */

	/* _SmartRefresh */
		bool _SmartRefreshForce = true; 
		bool _SmartRefreshUpdated = true;
		wxRect _SmartRefreshArea; 
		
		wxTimer _SmartRefreshTimer; 
		int _SmartRefreshClock = 5; 

		void l_SmartRefresh(); 
		void hk_SmartRefresh(wxTimerEvent& _Event); 
		void hdl_SmartRefreshUpdate(); 
		void hdl_SmartRefreshEvent__SmartRefreshBeforeUpdate();
	/* _SmartSyntax */
		bool _SmartSyntaxEnabled = false; 

		std::vector<std::string> _SmartSyntaxKeywords; 
		std::unordered_map<std::string, std::string> _SmartSyntaxColors; 

		void l_SmartSyntax(); 
		void hk_SmartSyntax_SmartRefreshBeforeUpdate(wxCommandEvent& _Event);
		
		const std::string hdl_SmartSyntaxHighlightKeywords(const std::string& _Line) const; 
		const std::string hdl_SmartSyntaxHighlightLiterals(const std::string& _Line) const; 
		const std::string hdl_SmartSyntaxHighlightComments(const std::string& _Line) const; 
		const std::string hdl_SmartSyntaxHighlightFunctions(const std::string& _Line) const;

		void hdl_SmartSyntaxApply();
};