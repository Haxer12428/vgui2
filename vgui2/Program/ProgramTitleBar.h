#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include "../Framework/Framework.h"
#include "ProgramButton.h"
#include <windows.h>

class ProgramTitleBar : public
	wxPanel
{ public:
	/*
		Program TitleBar includes: 
			gProgramCenterFixedPositon 
			Full Screen 
			Closing 
			Minimalizing 
			Name 
	*/

	/*
		On Startup 
	*/	
	ProgramTitleBar(
		Framework::Debug* _DEBUG, Framework::Config::File* _CONFIG, wxFrame* _PARENT, const std::string& _NAME
	);

	void exSizeHook();

	const void lPaintHook();
	const void lButtons();
	const void lButtonsUpdateTimer(); 
	const void lButtonsUpdateHook();
	const void lMouseMotionHook(); 
	const void lNameFont(); 
	/*
		External get functions 
	*/
	const wxPoint gProgramCenterFixedPosition() const;
private:
	/*
		Hooks 
	*/
	void PaintHook(
		wxPaintEvent& _EVT
	);

	void ButtonsUpdateHook(
		wxTimerEvent& _EVT
	); 

	void MouseMotionHook(
		wxMouseEvent& _EVT
	); 

	void hMouseExit(
		wxMouseEvent& _EVT
	); 

	/*
		Mouse 
	*/
	const void mHandleWindowMoving(
		wxMouseEvent& _EVt
	); // with sizers

	const void mHandleDragStart(
		wxMouseEvent& _EVT
	);

	const void mHandleExit(
		wxMouseEvent& _EVT
	); 

	/*
		Buttons actions 
	*/
	const void aMinimalize();
	const void aMaximalize(); 
	const void aExit(); 

	/*
		Sizers 
	*/
	const void sButtons();
	const void sPanel(); 

	/*
		Render 
	*/

	const void DrawBackground(
		wxAutoBufferedPaintDC& _CANVAS
	);

	const void DrawName(
		wxAutoBufferedPaintDC& _CANVAS
	); 

	/*
		Instances 
	*/
	Framework::Debug* _DEBUG;
	Framework::Config::File* _CONFIG; 

	/*
		Variables 
	*/
	wxSize Size; 
	wxPoint Position; 
	wxTimer _BUTTON_UPDATE_TIMER; 

	wxColor BackgroundColor = wxColor(90, 90, 90);

	ProgramButton* ExitButton;
	ProgramButton* MinimalizeButton; 
	ProgramButton* MaximalizeButton;

	wxSize _SIZE_BEFORE_MAXIMIZING;
	bool _MAXIMIZED = false; 
	wxPoint _MOUSE_POSITION_BEFORE_WINDOW_MOVING;

	bool _DRAG = false; 
	bool _DRAG_LAST_MOUSE_STATE = true; 
	

	wxFrame* _PARENT; 
	std::string _NAME; 

	wxFont _NAME_FONT; 
};