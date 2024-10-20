#include "ProgramTitleBar.h"

ProgramTitleBar::ProgramTitleBar(
	Framework::Debug* _DEBUG, Framework::Config::File* _CONFIG, wxFrame* _PARENT, const std::string& _NAME
) : wxPanel(_PARENT, wxID_ANY, Position, wxSize(_PARENT->GetClientSize().x, 25)), _NAME{_NAME}, _DEBUG{ _DEBUG }, _CONFIG{ _CONFIG }, _PARENT{ _PARENT } {
	try 
	{
		lNameFont();
		lPaintHook();
		lButtons();
		lButtonsUpdateTimer();
		lButtonsUpdateHook();
		lMouseMotionHook();
	}
	catch (
		const std::exception& ex
		) {
		// Throw error into LogMessage
		wxLogMessage(
			((std::string)"PROGRAM_TITLE_BAR: EXCEPTION OCCURED WHEN LOADING HOOKS: " + ex.what()).c_str()
		); // Refuse to continue with throwing exception, ProgramCenter will handle it; 
		throw ex;
	}


}


const wxPoint ProgramTitleBar::gProgramCenterFixedPosition() const 
{
	return wxPoint(0, this->Size.y);
}

const void ProgramTitleBar::lButtons()
{
	this->ExitButton = new ProgramButton(
		this, ProgramButton::rModes::Image, "C:\\vgui2\\closeicon.png", wxPoint(-25, 0), wxSize(25, 25), this->BackgroundColor, wxColor(255, 255, 255), wxColor(149, 185, 255));

	this->MaximalizeButton = new ProgramButton(
		this, ProgramButton::rModes::Image, "C:\\vgui2\\maximalizeicon.png", wxPoint(-50, 0), wxSize(25, 25), this->BackgroundColor, wxColor(255, 255, 255), wxColor(149, 185, 255));

	this->MinimalizeButton = new ProgramButton(
		this, ProgramButton::rModes::Image, "C:\\vgui2\\minimalizeicon.png", wxPoint(-75, 0), wxSize(25, 25), this->BackgroundColor, wxColor(255, 255, 255), wxColor(149, 185, 255));

	// Run sizer just incase 
	sButtons();
}


const void ProgramTitleBar::lButtonsUpdateHook()
{
	/*
		Buttons Update Hook:
			Works on _BUTTON_UPDATE_TIMER (with it's id) 
	*/
	Bind(wxEVT_TIMER, &ProgramTitleBar::ButtonsUpdateHook, this, _BUTTON_UPDATE_TIMER.GetId());
}

const void ProgramTitleBar::lButtonsUpdateTimer()
{
	/*
		Button update timer:
			Refreshes every 10 seconds
			Infinite 
	*/
	this->_BUTTON_UPDATE_TIMER.SetOwner(this);
	this->_BUTTON_UPDATE_TIMER.Start(10);
}

const void ProgramTitleBar::lMouseMotionHook()
{
	Bind(wxEVT_MOTION, &ProgramTitleBar::MouseMotionHook, this);
	Bind(wxEVT_LEAVE_WINDOW, &ProgramTitleBar::hMouseExit, this);
}

const void ProgramTitleBar::sPanel()
{
	/*
		Sizer: Size x_axis, leave y_axis as it was || Refresh after 
	*/
	this->SetSize(wxSize(_PARENT->GetClientSize().x, GetSize().y)); Refresh();
}

void ProgramTitleBar::exSizeHook()
{
	sPanel(); // Refresh for operations
	sButtons(); // Buttons sizers
}

void ProgramTitleBar::ButtonsUpdateHook(
	wxTimerEvent& _EVT
) { // might be a heavy operation if called so it needs further optimisation 
	/*
		Every time called checks for values & proceeds with actions if needed 
	*/
	bool _MINIMALIZE, _MAXIMALIZE, _EXIT; 

	_MINIMALIZE = MinimalizeButton->gActivation();
	_MAXIMALIZE = MaximalizeButton->gActivation();
	_EXIT = ExitButton->gActivation();

	if (!( // Check if any actions are required if not -> abort 
		_MINIMALIZE || _MAXIMALIZE || _EXIT
		)) return; 
	// Actions are required, choose a valid one and proceed with it 
	if (_MINIMALIZE)
		aMinimalize();
	if (_EXIT)
		aExit();
	if (_MAXIMALIZE)
		aMaximalize(); 
}

const void ProgramTitleBar::aExit()
{ // Close with param, force: true 
	wxExecute(std::string("taskkill /im " + std::to_string(GetCurrentProcessId()) + " /f").c_str(),
		wxEXEC_HIDE_CONSOLE);
}

const void ProgramTitleBar::aMaximalize()
{ // Check if maximized if not then proceed and maximize, if yes then go back to old size 
	if (this->_MAXIMIZED)
	{ _PARENT->Maximize(false); this->_MAXIMIZED = false; return; }
	// Proceed to maximize 
	_PARENT->Maximize(true); _PARENT->Refresh(); this->_MAXIMIZED = true; 
}

const void ProgramTitleBar::aMinimalize()
{ // Minimalize 
	_PARENT->Iconize(true); _PARENT->Refresh();
}

const void ProgramTitleBar::sButtons()
{
	/*
		Handle sizers for buttons for:
			x_axis
	*/
	const int X = this->GetClientSize().x; // x axis 

	this->MinimalizeButton->sPositionXMinus(X);
	this->ExitButton->sPositionXMinus(X);
	this->MaximalizeButton->sPositionXMinus(X);
}


const void ProgramTitleBar::lPaintHook()
{
	/*
		lPaintHook:
			Load for: wxAutoBufferedDC
			Main function: 'PaintHook'
	*/

	SetBackgroundStyle(wxBG_STYLE_COLOUR);
	Bind(wxEVT_PAINT, &ProgramTitleBar::PaintHook, this);
}

void ProgramTitleBar::PaintHook(
	wxPaintEvent& _EVT)
{
	/*
		Paint hook: 
			Paint mode: wxAutoBufferedDC
	*/
	try {
		wxAutoBufferedPaintDC _CANVAS{ this };

		DrawBackground(_CANVAS);
		DrawName(_CANVAS);
	} catch(
		const std::exception&)
	{ _DEBUG->Push("PROGRAM_TITLE_BAR : PAINT_HOOK EXCEPTION"); }
}

const void ProgramTitleBar::DrawBackground(
	wxAutoBufferedPaintDC& _CANVAS) 
{
	_CANVAS.SetPen(BackgroundColor);
	_CANVAS.SetBrush(BackgroundColor);
	_CANVAS.DrawRectangle(GetPosition(), GetClientSize());
}

void ProgramTitleBar::MouseMotionHook(
	wxMouseEvent& _EVT
) {
	/*
		This hook handles mouse motion operations for this class:
			Main window move
	*/
	try {
		mHandleDragStart(_EVT);
		mHandleWindowMoving(_EVT);
	} catch (const std::exception &ex) 
	{ wxLogMessage("PROGRAM_TITLE_BAR_MOUSE_MOTION_HOOK_EXCEPTION"); }
}

const void ProgramTitleBar::mHandleWindowMoving(
	wxMouseEvent& _EVT
) {
	/*
		This function handles main window moving
			conditions: 
				in panel bounds (handled by event) 
				lmb held 
	*/

	if (!_DRAG) return;

	if (_EVT.LeftIsDown() // LMB hold check | if not then update static mouse position before dragging the window 
		!= true) {
		this->_DRAG = false; 
		return;
	}

	const wxPoint _WINDOW_CURRENT_POSITION = _PARENT->GetPosition();
	const wxPoint _MOUSE_POSITION = _EVT.GetPosition();

	const wxPoint _WANTED_POSITION = (_WINDOW_CURRENT_POSITION + _MOUSE_POSITION - _MOUSE_POSITION_BEFORE_WINDOW_MOVING); // Fix for mouse input at TitleBar;

	// Set Position -> Refresh will be handled automaticly 
	_PARENT->SetPosition(_WANTED_POSITION);
}

const void ProgramTitleBar::lNameFont()
{
	wxFont _FONT_BASIC = wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	_FONT_BASIC.SetFaceName("Consolas");

	_NAME_FONT = _FONT_BASIC;
}

const void ProgramTitleBar::DrawName(
	wxAutoBufferedPaintDC& _CANVAS 
) {
	const wxPoint _CENTER = Framework::Transform::WxSizeToWxPoint2D(
		(GetClientSize() / 2));

	_CANVAS.SetFont(_NAME_FONT);
	_CANVAS.SetTextForeground(*wxWHITE);
	_CANVAS.SetTextBackground(*wxBLACK);
	
	const wxSize _FONT_SIZE = _CANVAS.GetTextExtent(_NAME);
	const wxSize _FONT_SIZE_CENTER = (_FONT_SIZE / 2);

	const wxPoint _CENTER_FIXED = ( // Final position 
		_CENTER - Framework::Transform::WxSizeToWxPoint2D(_FONT_SIZE_CENTER)
		);

	_CANVAS.DrawText(_NAME, _CENTER_FIXED);
}

const void ProgramTitleBar::mHandleDragStart(
	wxMouseEvent& _EVT
) {
	if (_DRAG == true) return;
	const bool _STATE = _EVT.LeftIsDown();

	if (_DRAG_LAST_MOUSE_STATE == true && _STATE == true)
		return; 

	if (_DRAG_LAST_MOUSE_STATE == true && _STATE == false)
	{
		_DRAG_LAST_MOUSE_STATE = false; return; 
	}

	if (_DRAG_LAST_MOUSE_STATE == false && _STATE == false)
		return;

	_DRAG = true; 
	_DRAG_LAST_MOUSE_STATE = true; 
	_MOUSE_POSITION_BEFORE_WINDOW_MOVING = _EVT.GetPosition();
}

const void ProgramTitleBar::mHandleExit(wxMouseEvent& _EVT) 
{ this->_DRAG_LAST_MOUSE_STATE = true; }

void ProgramTitleBar::hMouseExit(wxMouseEvent& _EVT) 
{ try { mHandleExit(_EVT); } catch (const std::exception& ex) { } }