#pragma once
#include "../Framework/Framework.h"
#include <wx/wx.h>
#include <wx/dcbuffer.h>

class ProgramResizeIcon : public wxPanel 
{ public: 
	ProgramResizeIcon(
		wxFrame* _PARENT, Framework::Debug* _DEBUG, Framework::Config::File* _CONFIG, const wxColor& _BG_COLOR
	);

	/*
		Sizers 
	*/
	const void sSelf(); // Size Self 
private:
	/*
		Load 
	*/
	const void lPaintHook();
	const void lPanelParams(); 
	const void lConfigParams(); 
	const void lMouseHooks(); 
	const void lResizeUpdateTimer();

	/*
		Hooks 
	*/
	void PaintHook(
		wxPaintEvent& _EVT
	);

	void MouseMotionHook(
		wxMouseEvent& _EVT
	);

	void MouseLeftDownHook(
		wxMouseEvent& _EVT
	);

	void MouseExitHook(
		wxMouseEvent& _EVT
	);

	/*
		Mouse 
	*/
	const void mHandleExit(
		wxMouseEvent& _EVT
	); // Leave Window 

	void mHandleResizing(
		wxTimerEvent& _EVT
	); // Update Timer 

	const void mHandleResizingStart(
		wxMouseEvent& _EVT
	); // Left Down 

	const void mHandleResizingEnd(
		wxMouseEvent& _EVT
	); // Motion first 
	

	/*
		Render 
	*/
	const void rBackground(
		wxAutoBufferedPaintDC& _CANVAS
	); 

	const void rIcon(
		wxAutoBufferedPaintDC& _CANVAS
	);

	/*d
		Variables 
	*/
	wxFrame* _PARENT; 
	wxColor _BG_COLOR; 
	Framework::Debug* _DEBUG; 
	Framework::Config::File* _CONFIG; 

	wxSize _SIZE; 
	wxPoint _LEFT_CORNER_MINUS; 
	int _THICKNESS;
	wxColor _ELEMENT_COLOR;


	bool _LAST_INPUT_DOWN = true;
	bool _RESIZING = false; 

	wxPoint _MOUSE_POSITION_RESIZE_START;

	wxTimer _RESIZE_UPDATE;
};