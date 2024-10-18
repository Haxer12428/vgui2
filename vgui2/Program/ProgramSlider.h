#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include "../Framework/Framework.h"

class ProgramSlider :
	public wxPanel 
{ public:
	enum Mode
	{
		Vertical = 0,
		Horizontal = 1
	};

	/*
		On Startup 
	*/
	ProgramSlider(
		wxPanel* _PARENT, Framework::Debug* _DEBUG,
			const wxSize& _SIZE, const wxPoint& _POSITION, 
			const wxColor& _BG_COLOR, const wxColor& _ELEMENT_COLOR, 
			const double &_ELEMENT_SIZE_BY_SCROLL, const double& _ELEMENT_SIZE_BY_HEIGHT,
		ProgramSlider::Mode _MODE 
	);

	const double gScroll() const;
	const double gScrollProcentage() const; 

	const void sScrollProcentage(const double& _NEW);

	const void s_Position(const wxPoint& _Position);
	const void s_Size(const wxSize& _Size);
private:
	/*
		On Startup
	*/
	const void lPanelParams();
	const void lPaintHook(); 
	const void lMouseHooks();
	const void lDragUpdateHook();
	const void lDragAnimateHook();

	/*
		Hooks 
	*/
	void hPaint(wxPaintEvent& _EVT);
	void hMouseMotion(wxMouseEvent& _EVT);
	void hMouseExitPanel(wxMouseEvent& _EVT);
	void hDragUpdate(wxTimerEvent& _EVT); 
	void hDragAnimate(wxTimerEvent& _EVT);

	/*
		Mouse 
	*/
	const void mHandleDragStart(
		wxMouseEvent& _EVT
	); // Motion 

	const void mHandleExit(); // WindowLeave

	const void mHandleDragUpdate(); // Timer 

	/*
		Render 
	*/
	const void rBackground(wxAutoBufferedPaintDC& _CANVAS);
	const void rElement(wxAutoBufferedPaintDC& _CANVAS); 
	
	/*
		Animate 
	*/
	const void aDragHandler();

	/*
		Geometry 
	*/
	Framework::Geometry::BoundingBox gElementBoundingBox() const; 
	const int gMaxScroll() const; 

	/*
		Variables	
	*/

	wxSize _SIZE; 
	wxPoint _POSITION; 

	Framework::Debug* _DEBUG;
	wxPanel* _PARENT; 

	wxColor _BG_COLOR;
	wxColor _ELEMENT_COLOR;

	double _ELEMENT_SIZE_BY_SCROLL;
	double _ELEMENT_SIZE_BY_HEIGHT;

	double _SCROLL = 0; 
	double _SCROLL_WANTED = 0;

	ProgramSlider::Mode _MODE; 

	bool _DRAG_LAST_UPDATE_MOUSE_HELD = true; 
	bool _DRAG = false; 

	wxPoint _DRAG_MOUSE_POSITION_AT_THE_START; 
	
	wxTimer _DRAG_UPDATE_TIMER; 
	wxTimer _DRAG_ANIMATE_TIMER; 

	int _DRAG_ANIMATE_MS = 10; 
};
