#include "ProgramButton.h"



ProgramButton::ProgramButton(
	wxPanel* _PARENT, ProgramButton::rModes _MODE, const std::string& _rPATH_OR_STRING,
		const wxPoint& _POSITION, const wxSize& _SIZE, const wxColor& _BG_COLOR, const wxColor& _HEAD_COLOR, const wxColor& _ACTIVATED_COLOR
)
	: _PARENT{_PARENT}, _MODE{_MODE}, _rPATH_OR_STRING{_rPATH_OR_STRING}, 
		_POSITION{_POSITION}, _SIZE{_SIZE}, _BG_COLOR{_BG_COLOR}, _HEAD_COLOR{_HEAD_COLOR}, _STATIC_POSITION{_POSITION}, _ACTIVATED_COLOR{_ACTIVATED_COLOR}, _BG_COLOR_STATIC{_BG_COLOR},
	wxPanel(_PARENT, wxID_ANY, _POSITION, _SIZE) // Panel -> Main Class Object 
{
	try
	{ // Load On Startup 
		wxInitAllImageHandlers();

		lPanelParams();
		lIconBitmaps();
		lPaintHook();
		lMouseHooks();
	}
	catch (
		const std::exception& ex
		) // Report error, pass exception
	{
		wxLogMessage(
			(std::string("PROGRAM_BUTTON : EXCEPTION OCCURED ") + ex.what()).c_str()
		); throw ex;
	}
	
	// Finalize with showing up the panel 
	this->Show();
}

const void ProgramButton::sPositionXMinus(const int& X)
{ // Size x axis 
	this->_POSITION = wxPoint(X + _STATIC_POSITION.x, _STATIC_POSITION.y);
	
	this->SetPosition(_POSITION);
	Refresh();
}

const void ProgramButton::lIconBitmaps()
{
	wxImage _IMAGE; 

	if (!_IMAGE.LoadFile(
		_rPATH_OR_STRING, wxBITMAP_TYPE_PNG
	)) throw std::exception("lIconBitmaps : FAILED_TO_LOAD_IMAGE");

	try {
		_IMAGE = Framework::Image::cSize(_IMAGE, _SIZE);
		_IMAGE = Framework::Image::cColor(_IMAGE, _HEAD_COLOR);

		this->_IMAGE_BITMAP = wxBitmap(_IMAGE);
	} catch	(const std::exception& ex) 
		{ throw ex; } // Pass error 
}

const bool ProgramButton::gActivation()
{
	const bool _STATUS = this->_ACTIVATED;
	this->_ACTIVATED = false; // Destructor 

	return _STATUS; 
}

const void ProgramButton::pStringColor(
	const wxColor& _STRING_COLOR)
{
	this->_STRING_COLOR = _STRING_COLOR; 
}

const void ProgramButton::lPanelParams()
{
	//SetBackgroundColour(_BG_COLOR);
}

const void ProgramButton::lMouseHooks()
{
	Bind(wxEVT_MOTION, &ProgramButton::MouseHook, this);
	Bind(wxEVT_LEFT_UP, &ProgramButton::MouseLMBHook, this);
	Bind(wxEVT_LEAVE_WINDOW, &ProgramButton::MouseExitHook, this);
}

const void ProgramButton::lPaintHook()
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	Bind(wxEVT_PAINT, &ProgramButton::PaintHook, this);
}

void ProgramButton::PaintHook(
	wxPaintEvent& _EVT) {
	/*
		This hook handles render operation in this class 
			Works on 'wxAutoBufferedDC'
	*/
	try {
		wxAutoBufferedPaintDC _CANVAS{ this };

		rBackground(_CANVAS);
		rHeadHandler(_CANVAS);
	} catch (const std::exception &) { 
		wxLogMessage(
			"PROGRAM_BUTTON_PAINT_HOOK_EXCEPTION"
		); }
}

void ProgramButton::MouseExitHook(
	wxMouseEvent& _EVT
) {
	/*
		This hook handles mouse exit (exited from panel) operations in this class
	*/
	try
	{
		mExitHandler(_EVT);
	}
	catch (const std::exception&) {
		wxLogMessage(
			"PROGRAM_BUTTON_MOUSE_EXIT_HOOK_EXCEPTION"
		);
	};
}

void ProgramButton::MouseHook(
	wxMouseEvent& _EVT
) {
	/*
		This hook handles mouse motion operations in this class
	*/
	try 
	{
		mMotionHandler(_EVT);
	}
	catch (const std::exception&) {
		wxLogMessage(
			"PROGRAM_BUTTON_MOUSE_HOOK_EXCEPTION"
		); }; 
}

void ProgramButton::MouseLMBHook(
	wxMouseEvent& _EVT
) {
	/*
		This hook handles mouse LBM hook (mouse click, (mouse left button up)) operations in this class
	*/
	try
	{
		mLClickHandler(_EVT);
	}
	catch (const std::exception&) {
		wxLogMessage(
			"PROGRAM_BUTTON_MOUSELMB_HOOK_EXCEPTION"
		);
	};
}

const void ProgramButton::mMotionHandler(
	wxMouseEvent& _EVT)
{ // Change color of bg & refresh 
	this->_BG_COLOR = _ACTIVATED_COLOR; this->Refresh();
}

const void ProgramButton::mExitHandler(
	wxMouseEvent& _EVT
) {
	this->_BG_COLOR = _BG_COLOR_STATIC; this->Refresh();
}

const void ProgramButton::mLClickHandler(
	wxMouseEvent& _EVT)
{ // Works on mouse up hook (experimental, should work as intended as long as class object is panel) 
	this->_ACTIVATED = true;
}

const void ProgramButton::rBackground(
	wxAutoBufferedPaintDC& _CANVAS)
{
	_CANVAS.SetPen(_BG_COLOR);
	_CANVAS.SetBrush(_BG_COLOR);
	_CANVAS.DrawRectangle(_POSITION, _SIZE);
}

const void ProgramButton::rHeadHandler(
	wxAutoBufferedPaintDC& _CANVAS
) {
	/*
		Main hook for rending head part of the button 
			"image" : "text" -> 
				Located in ProgramButton::rModes 
			"image" types: 
				"exit-img"
	*/
	try
	{
		if ( // Image renderer
			this->_MODE == ProgramButton::rModes::Image
			)
		{ // Call hook responsible for drawing images 
			rImage(_CANVAS);
			return; 
		}

		// Pass exception to paint hook 
	} catch (const std::exception& ex) { throw ex; };
}

const void ProgramButton::rImage(
	wxAutoBufferedPaintDC& _CANVAS
) {
	if (_IMAGE_BITMAP.IsOk() // Check if bitmap is loaded correctly 
		== false) throw std::exception(
			"rExitImage : BITMAP_NOT_LOADED");
	// Render 
	_CANVAS.DrawRectangle(GetClientRect());
	_CANVAS.DrawBitmap(_IMAGE_BITMAP, 0, 0);
}
