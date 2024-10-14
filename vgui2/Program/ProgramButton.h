#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/artprov.h>
#include <wx/image.h>
#include "../Framework/Framework.h"

class ProgramButton : public
	wxPanel
{ public: 
	/*
		Program Button 
			Render Modes: Image, String 
	*/

	/*
		Structs 
	*/

	enum rModes
	{
		Image = 0,
		String = 0
	};

	/*
		On Startup 
	*/
	ProgramButton(
		wxPanel* _PARENT, ProgramButton::rModes _MODE, const std::string& _rPATH_OR_STRING,
			const wxPoint& _POSITION, const wxSize& _SIZE, const wxColor& _BG_COLOR, const wxColor& _HEAD_COLOR, const wxColor& _ACTIVATED_COLOR
	);
	/*
		External Get Functions 
	*/
	const bool gActivation();

	/*
		External Provide Functions 
	*/
	const void pStringColor(const wxColor& _STRING_COLOR);
	const void sPositionXMinus(const int& X); // Position sizer (from minus) 
private:
	/*
		On Startup
	*/
	const void lPanelParams();
	const void lPaintHook();
	const void lMouseHooks();
	const void lIconBitmaps(); 

	/*
		Hooks
	*/
	void PaintHook(
		wxPaintEvent& _EVT
	);

	void MouseHook(
		wxMouseEvent& _EVT
	);

	void MouseLMBHook(
		wxMouseEvent& _EVT
	);

	void MouseExitHook(
		wxMouseEvent& _EVT
	); 

	/*
		Render
	*/
	const void rBackground(
		wxAutoBufferedPaintDC& _CANVAS
	); 

	const void rHeadHandler(
		wxAutoBufferedPaintDC& _CANVAS
	); 

	const void rImage(
		wxAutoBufferedPaintDC& _CANVAS
	);

	/*
		Mouse 
	*/
	const void mLClickHandler(
		wxMouseEvent& _EVT
	);

	const void mMotionHandler(
		wxMouseEvent& _EVT
	);

	const void mExitHandler(
		wxMouseEvent& _EVT
	); 

	/*
		Variables 
	*/

	bool _ACTIVATED = false; 
	wxPanel* _PARENT; 
	ProgramButton::rModes _MODE; 
	std::string _rPATH_OR_STRING;
	wxColor _BG_COLOR;
	wxPoint _POSITION;
	wxSize _SIZE; 
	wxColor _STRING_COLOR; 
	wxColor _HEAD_COLOR;
	wxColor _ACTIVATED_COLOR;
	wxColor _BG_COLOR_STATIC;
	wxPoint _STATIC_POSITION;

	wxBitmap _IMAGE_BITMAP;
};