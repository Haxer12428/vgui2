#include "ProgramResizeIcon.h"

ProgramResizeIcon::ProgramResizeIcon(
	wxFrame* _PARENT, Framework::Debug* _DEBUG, Framework::Config::File* _CONFIG, const wxColor& _BG_COLOR
) :
	_PARENT{ _PARENT }, _DEBUG{ _DEBUG }, _CONFIG{ _CONFIG }, _BG_COLOR{ _BG_COLOR }, 
	wxPanel(_PARENT, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	try 
	{ // load 'l' Functios 
		lConfigParams(); 
		lPanelParams();
		sSelf();
		lPaintHook(); 
		lMouseHooks();
		lResizeUpdateTimer();
	}
	catch (
		const std::exception &ex 
		) { // Pass the exception to let lower-level object handle it 
		throw ex; 
	}
}

const void ProgramResizeIcon::lResizeUpdateTimer()
{
	/*
		Resize timer 
			refresh every: 1 ms 
	*/
	_RESIZE_UPDATE.SetOwner(this);
	_RESIZE_UPDATE.Start(3);

	Bind(wxEVT_TIMER, &ProgramResizeIcon::mHandleResizing, this, _RESIZE_UPDATE.GetId());
}

const void ProgramResizeIcon::lPaintHook()
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	Bind(wxEVT_PAINT, &ProgramResizeIcon::PaintHook, this);
}

const void ProgramResizeIcon::lConfigParams()
{
	_LEFT_CORNER_MINUS =
		wxPoint
		(
			_CONFIG->gValue("ResizeIconMinusFromLeftCornerX", true, _DEBUG).gInt(),
			_CONFIG->gValue("ResizeIconMinusFromLeftCornerY", true, _DEBUG).gInt()
		);
		

	_SIZE =
		wxSize
		(
			_CONFIG->gValue("ResizeIconSize", true, _DEBUG).gInt(),
			_CONFIG->gValue("ResizeIconSize", true, _DEBUG).gInt()
		);

	_THICKNESS = _CONFIG->gValue("ResizeIconThickness", true, _DEBUG).gInt();

	_ELEMENT_COLOR =
		wxColor
		(
			_CONFIG->gValue("ResizeIconElementColorR", true, _DEBUG).gInt(),
			_CONFIG->gValue("ResizeIconElementColorG", true, _DEBUG).gInt(),
			_CONFIG->gValue("ResizeIconElementColorB", true, _DEBUG).gInt()
		);
		
}

const void ProgramResizeIcon::lPanelParams()
{

}

const void ProgramResizeIcon::sSelf()
{
	/*
		position formula:
			left corner - param - size 
	*/

	const wxPoint _WANTED_POSITION = (
		Framework::Transform::WxSizeToWxPoint2D(_PARENT->GetClientSize()) - Framework::Transform::WxSizeToWxPoint2D(_SIZE) - _LEFT_CORNER_MINUS
		);

	SetSize(_SIZE);
	SetPosition(_WANTED_POSITION);

	Refresh();
}

void ProgramResizeIcon::PaintHook(
	wxPaintEvent& _EVT
) {
	/*
		This hook handles all render operations for this class
			works on: wxAutoBufferedPaintDC 
	*/
	try 
	{
		wxAutoBufferedPaintDC _CANVAS{ this };
		// Call render operations (functions with 'r')

		rBackground(_CANVAS);
		rIcon(_CANVAS);
	}
	catch (
		const std::exception 
	) { wxLogMessage("PROGRAM_RESIZE_ICON_PAINT_HOOK_EXCEPTION_CATCHED"); }
}

const void ProgramResizeIcon::rBackground(
	wxAutoBufferedPaintDC& _CANVAS)
{
	_CANVAS.SetPen(_BG_COLOR);
	_CANVAS.SetBrush(_BG_COLOR);
	_CANVAS.DrawRectangle(GetClientRect());
}

const void ProgramResizeIcon::rIcon(
	wxAutoBufferedPaintDC& _CANVAS
) {
	_CANVAS.SetPen(_ELEMENT_COLOR);
	_CANVAS.SetBrush(_ELEMENT_COLOR);

	const wxSize _SELF_SIZE = GetClientSize();

	const wxSize _FIRST_BOX_SIZE = wxSize(_THICKNESS, _SELF_SIZE.y - _THICKNESS);
	const wxSize _SECOND_BOX_SIZE = wxSize(_SELF_SIZE.x, _THICKNESS);

	const wxPoint _FIRST_BOX_POSITION = wxPoint(_SELF_SIZE.x - _THICKNESS, 0);
	const wxPoint _SECOND_BOX_POSITION = wxPoint(0, _SELF_SIZE.y - _THICKNESS);

	_CANVAS.DrawRectangle(_FIRST_BOX_POSITION, _FIRST_BOX_SIZE);
	_CANVAS.DrawRectangle(_SECOND_BOX_POSITION, _SECOND_BOX_SIZE);
}

const void ProgramResizeIcon::lMouseHooks()
{
	Bind(wxEVT_MOTION, &ProgramResizeIcon::MouseMotionHook, this);
	Bind(wxEVT_LEFT_DOWN, &ProgramResizeIcon::MouseLeftDownHook, this);
	Bind(wxEVT_LEAVE_WINDOW, &ProgramResizeIcon::MouseExitHook, this);
}


void ProgramResizeIcon::MouseMotionHook(
	wxMouseEvent& _EVT
) {
	mHandleResizingEnd(_EVT);
	mHandleResizingStart(_EVT);

}

void ProgramResizeIcon::MouseLeftDownHook(
	wxMouseEvent& _EVT
) {

}

const void ProgramResizeIcon::mHandleResizingStart(
	wxMouseEvent& _EVT)
{
	/*
		Abort if resizing ||  
			if not resizing:
				last input was true:
					wait for false input 
				last input was false:
					this input is true:
						resizing: true 
		Update last input 
	*/
	if (_RESIZING) return; 

	const bool _LEFT_DOWN = _EVT.LeftIsDown();

	if (_LAST_INPUT_DOWN ==
		true && _LEFT_DOWN == true)
		return;

	if (_LAST_INPUT_DOWN == true &&
		_LEFT_DOWN == false)
	{
		_LAST_INPUT_DOWN = false; return;
	}

	if (_LAST_INPUT_DOWN ==
		false && _LEFT_DOWN == false)
		return; 

	_LAST_INPUT_DOWN = true;
	_RESIZING = true; 
	_MOUSE_POSITION_RESIZE_START = (wxGetMousePosition() - (_PARENT->GetPosition() + GetPosition()));
}

void ProgramResizeIcon::mHandleResizing(
	wxTimerEvent& _EVT
) {
	/* 
		Centered for mouse, _RESIZING: must be true 
	*/
	if (not _RESIZING)
		return;

	const wxPoint _MOUSE_POSITION = wxGetMousePosition() - _PARENT->GetPosition();
	const wxPoint _PANEL_POSITION = GetPosition();

	const wxPoint _CENTERED = (_MOUSE_POSITION + _MOUSE_POSITION_RESIZE_START + _LEFT_CORNER_MINUS);

	if (_PARENT->GetSize() // Size is equal no need to overload the system 
		== Framework::Transform::WxPointToWxSize2D(_CENTERED))
		return; 

	_PARENT->SetSize(Framework::Transform::WxPointToWxSize2D(
		_CENTERED));
}

const void ProgramResizeIcon::mHandleResizingEnd(
	wxMouseEvent& _EVT
) {
	if (_RESIZING ==
		false) return;
	// Must hold left down for resizing to be accepted 
	if (_EVT.LeftIsDown()
		== true) return;

	_RESIZING = false; 
}

void ProgramResizeIcon::MouseExitHook(
	wxMouseEvent& _EVT 
) { this->mHandleExit(_EVT); }

const void ProgramResizeIcon::mHandleExit(
	wxMouseEvent& _EVT
) { // Force double check for mouse left down after entering the window next time 
	_LAST_INPUT_DOWN = true; 
}

