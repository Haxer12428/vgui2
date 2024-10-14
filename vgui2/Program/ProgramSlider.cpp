#include "ProgramSlider.h"

ProgramSlider::ProgramSlider(
	wxPanel* _PARENT, Framework::Debug* _DEBUG, 
	const wxSize& _SIZE, const wxPoint& _POSITION,
	const wxColor& _BG_COLOR, const wxColor& _ELEMENT_COLOR,
	const double& _ELEMENT_SIZE_BY_SCROLL, const double& _ELEMENT_SIZE_BY_HEIGHT,
	ProgramSlider::Mode _MODE
) :
	// Instances 
	_PARENT{_PARENT}, _DEBUG{_DEBUG}, 
	// Colors 
	_BG_COLOR{_BG_COLOR}, _ELEMENT_COLOR{_ELEMENT_COLOR}, 
	// Doubles 
	_ELEMENT_SIZE_BY_SCROLL{_ELEMENT_SIZE_BY_SCROLL}, _ELEMENT_SIZE_BY_HEIGHT{_ELEMENT_SIZE_BY_HEIGHT},
	// Internal Enums 
	_MODE{_MODE},
	// Positions & Sizes
	_SIZE{_SIZE}, _POSITION{_POSITION},
	// Main Object 
	wxPanel(_PARENT, wxID_ANY, _POSITION, _SIZE)
{
	try 
	{ // On innit functions 
		lPanelParams();
		lDragUpdateHook();
		lPaintHook();
		lMouseHooks();
		lDragAnimateHook();
	} catch (const std::exception& ex) 
	{ // Pass the exception to a object lower 
		throw ex;
	}
}

const void ProgramSlider::lPanelParams()
{
	SetSize(_SIZE);
	SetPosition(_POSITION);
	SetDoubleBuffered(true);

	Refresh();
}


const void ProgramSlider::lPaintHook()
{
	SetBackgroundStyle(wxBG_STYLE_COLOUR);
	Bind(wxEVT_PAINT, &ProgramSlider::hPaint, this);
}

void ProgramSlider::hPaint(
	wxPaintEvent& _EVT 
) {
	/* Working on 'wxAutoBufferedPaintDC'*/
	try {
		wxAutoBufferedPaintDC _CANVAS{ this };

		rBackground(_CANVAS);
		rElement(_CANVAS);
	} catch (const std::exception &ex) { _DEBUG->Push(ex.what()); }
}

const void ProgramSlider::rBackground(
	wxAutoBufferedPaintDC& _CANVAS)
{
	_CANVAS.SetBrush(_BG_COLOR);
	_CANVAS.SetPen(_BG_COLOR);
	_CANVAS.DrawRectangle(GetClientRect());
}

const void ProgramSlider::rElement(
	wxAutoBufferedPaintDC& _CANVAS)
{
	Framework::Geometry::BoundingBox _ELEMENT_BOUNDINGBOX = gElementBoundingBox();

	_CANVAS.SetBrush(_ELEMENT_COLOR);
	_CANVAS.SetPen(_ELEMENT_COLOR);
	_CANVAS.DrawRoundedRectangle(_ELEMENT_BOUNDINGBOX.gRect(), 2);
}

Framework::Geometry::BoundingBox ProgramSlider::gElementBoundingBox() const
{
	wxSize _ELEMENT_SIZE; 
	wxPoint _ELEMENT_POSITION; 
	wxSize _ELEMENT_SCROLL; 

	int _ELEMENT_SIZE_X, _ELEMENT_SIZE_Y;

	if (this->_MODE == // Vertical (down, up)
		ProgramSlider::Mode::Vertical)
	{
		_ELEMENT_SIZE_X = (
			(double)_SIZE.x * this->_ELEMENT_SIZE_BY_HEIGHT);
		_ELEMENT_SIZE_Y = (
			(double)_SIZE.y * this->_ELEMENT_SIZE_BY_SCROLL);
		_ELEMENT_SIZE = wxSize(_ELEMENT_SIZE_X, _ELEMENT_SIZE_Y);

		_ELEMENT_POSITION = wxPoint(
			(_SIZE.x - _ELEMENT_SIZE_X) / 2, _SCROLL
		);
	}
	else // Horizontal (left, right)
	{
		_ELEMENT_SIZE_X = (
			(double)_SIZE.x * this->_ELEMENT_SIZE_BY_SCROLL);
		_ELEMENT_SIZE_Y = (
			(double)_SIZE.y * this->_ELEMENT_SIZE_BY_HEIGHT);
		_ELEMENT_SIZE = wxSize(_ELEMENT_SIZE_X, _ELEMENT_SIZE_Y);


		_ELEMENT_POSITION = wxPoint(
			_SCROLL, (_SIZE.y - _ELEMENT_SIZE_Y) / 2
		);
	}

	return Framework::Geometry::BoundingBox( // Return 'BoundingBox' with data of the element positions 
		_ELEMENT_POSITION, _ELEMENT_POSITION + Framework::Transform::WxSizeToWxPoint2D(_ELEMENT_SIZE));
};

const void ProgramSlider::lMouseHooks()
{
	Bind(wxEVT_MOTION, &ProgramSlider::hMouseMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &ProgramSlider::hMouseExitPanel, this);
}

void ProgramSlider::hMouseExitPanel(
	wxMouseEvent& _EVT
) {
	try {
		mHandleExit();
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }
}

const void ProgramSlider::mHandleExit() 
{ _DRAG_LAST_UPDATE_MOUSE_HELD = true; // assures that mouse will be double checked next time it crosses panel bounds
}

void ProgramSlider::hMouseMotion(
	wxMouseEvent& _EVT
) {
	try {
		mHandleDragStart(_EVT);
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }
}

const void ProgramSlider::mHandleDragStart(
	wxMouseEvent& _EVT
) {
	if (_DRAG == true)
		return;
	const bool _HOTKEY_LMB = _EVT.LeftIsDown();
	if (_HOTKEY_LMB && _DRAG_LAST_UPDATE_MOUSE_HELD)
		return; 

	if (not _HOTKEY_LMB && _DRAG_LAST_UPDATE_MOUSE_HELD)
	{ _DRAG_LAST_UPDATE_MOUSE_HELD = false; return; }

	if (not _HOTKEY_LMB)
		return;

	Framework::Geometry::BoundingBox _ELEMENT_BOUNDING_BOX = gElementBoundingBox();

	if (!Framework::Geometry::cBounds( // check if mouse is in bounds of element 
		_EVT.GetPosition(), _ELEMENT_BOUNDING_BOX.gStarting(), _ELEMENT_BOUNDING_BOX.gSize()
		)) return; 

	_DRAG = true; 
	_DRAG_LAST_UPDATE_MOUSE_HELD = true; 
	_DRAG_MOUSE_POSITION_AT_THE_START = (_EVT.GetPosition() - _ELEMENT_BOUNDING_BOX.gStarting());
}

const void ProgramSlider::lDragUpdateHook()
{
	_DRAG_UPDATE_TIMER.SetOwner(this);
	_DRAG_UPDATE_TIMER.Start(1);

	Bind(wxEVT_TIMER, &ProgramSlider::hDragUpdate, this, _DRAG_UPDATE_TIMER.GetId());
}

void ProgramSlider::hDragUpdate(
	wxTimerEvent& _EVT
) {
	try {
		mHandleDragUpdate();
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }
}

const void ProgramSlider::mHandleDragUpdate()
{ // Check is its needed to run this handler 
	if (!_DRAG) return; 
	// 2nd run check & fix state if needed 
	if (!wxGetMouseState().LeftIsDown())
	{ _DRAG = false; return; }

	// Calculate positions and _DIF (mouse to window) 
	const wxPoint _MOUSE_POSITION_IN_WINDOW = (wxGetMousePosition());
	const wxPoint _ELEMENT_POSITION_IN_WINDOW = (GetScreenPosition());

	const wxPoint _DIF = (_MOUSE_POSITION_IN_WINDOW - _ELEMENT_POSITION_IN_WINDOW - _DRAG_MOUSE_POSITION_AT_THE_START);

	/*
		Determinate the scroll 
	*/
	int _cSCROLL;

	if (_MODE == Mode::Vertical)
	{ _cSCROLL = (int)_DIF.y; // vertical 
	} else { _cSCROLL = (int)_DIF.x;} // horizontal 

	// Limit _cScroll to behave in given bounds 
	_cSCROLL = std::max(0, _cSCROLL); // - bound 
	_cSCROLL = std::min(gMaxScroll(), _cSCROLL); // + bound 

	// Check if its needed to refresh & update _SCROLL
	_SCROLL_WANTED = _cSCROLL;
}

const void ProgramSlider::lDragAnimateHook()
{
	_DRAG_ANIMATE_TIMER.SetOwner(this);
	_DRAG_ANIMATE_TIMER.Start(_DRAG_ANIMATE_MS);
	
	Bind(wxEVT_TIMER, &ProgramSlider::hDragAnimate, this, _DRAG_ANIMATE_TIMER.GetId());
}

void ProgramSlider::hDragAnimate(
	wxTimerEvent& _EVT
) { // Main animation event handler for drag 
	try { aDragHandler(); } catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }
}

const void ProgramSlider::aDragHandler()
{

	/*
	if (_SCROLL == _SCROLL_WANTED)
		return;

	this->_SCROLL = _SCROLL_WANTED; 

	Refresh();
	if (true)
		return;
		*/

	if (_SCROLL == _SCROLL_WANTED)
		return; // Matching, no need to proceed 
	const wxSize _SIZE_DIF = (GetSize() - gElementBoundingBox().gSize()); 

	Framework::Geometry::BoundingBox ELEMENT_BOUNDING_BOX = gElementBoundingBox();

	const double _VALUE_PER_MS = ((
		(double)(_MODE == Mode::Vertical ? _SIZE_DIF.y : _SIZE_DIF.x) / (((double)1000 / (double)_DRAG_ANIMATE_MS) / (double)2))
	); // Full slider will be moved in 100ms maximum

	if (_SCROLL > _SCROLL_WANTED) // - operation 
	{ this->_SCROLL = std::max(_SCROLL_WANTED, (double)_SCROLL - _VALUE_PER_MS); RefreshRect(ELEMENT_BOUNDING_BOX.gRect());
		return; }
	this->_SCROLL = std::min(_SCROLL_WANTED, (double)_SCROLL + _VALUE_PER_MS); RefreshRect(ELEMENT_BOUNDING_BOX.gRect()); // + operation 
}

const int ProgramSlider::gMaxScroll() const
{
	const wxSize _MAX_SCROLLS = (GetSize() - gElementBoundingBox().gSize());

	return (_MODE == Mode::Vertical ? _MAX_SCROLLS.y : _MAX_SCROLLS.x);
}

const double ProgramSlider::gScroll() const 
{
	return _SCROLL;
}

const double ProgramSlider::gScrollProcentage() const
{
	return (double)(_SCROLL / (double)gMaxScroll());
}

const void ProgramSlider::sScrollProcentage(const double& 
	_NEW) { _SCROLL_WANTED = ((double)gMaxScroll() * _NEW); }