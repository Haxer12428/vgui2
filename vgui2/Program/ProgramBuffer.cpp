#include "ProgramBuffer.h"

ProgramBuffer::ProgramBuffer(
	wxPanel* _PARENT, Framework::Debug* _DEBUG, const wxPoint& _POSITION, const wxSize& _SIZE,
	const wxColor& _BG_COLOR, const wxColor& _SLIDERS_COLOR, const wxColor& _DEFAULT_TEXT_COLOR
) 
	: 
	// Instances 
	_PARENT{_PARENT}, _DEBUG{_DEBUG}, 
	// Positions 
	_POSITION{_POSITION}, _SIZE{_SIZE},
	// Colors 
	_SLIDERS_COLOR{_SLIDERS_COLOR}, _BG_COLOR{_BG_COLOR}, _DEFAULT_TEXT_COLOR{_DEFAULT_TEXT_COLOR},
	// Main Obj 
	wxPanel(_PARENT, wxID_ANY, _POSITION, _SIZE)
{

	

	try {
		// load 'l' Functions 
		lPanelParams();
		lSlidersPositions();
		lSliders();
		lTextFont(); 
		lOnCharHooks();
		lPaintHook();
		lScrollUpdateHook();
		lMouseHooks();
		lCursorBlinkHook();
		l_SmartRefresh();
		l_SmartSyntax();
	} catch (const std::exception &ex) {
		throw ex; // Pass the error to an object higher above 
	}
}

const void ProgramBuffer::lPaintHook()
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	SetDoubleBuffered(true);
	Bind(wxEVT_PAINT, &ProgramBuffer::hPaint, this);
}

const void ProgramBuffer::lOnCharHooks()
{
	SetFocus();
	Bind(wxEVT_CHAR_HOOK, &ProgramBuffer::hOnChar, this);
}

const void ProgramBuffer::lTextFont()
{
	wxFont _FONT_BASIC = wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	_FONT_BASIC.SetFaceName("Consolas");

	_TEXT_FONT = _FONT_BASIC; 
}

const void ProgramBuffer::lPanelParams()
{
	SetBackgroundColour(_BG_COLOR);

	Show(true);
}

const void ProgramBuffer::lSlidersPositions()
{
	Framework::Geometry::BoundingBox _vBoundngBox = g_SLIDER_VERTICAL_BoundingBox();
	Framework::Geometry::BoundingBox _hBoundingBox = g_SLIDER_HORIZONTAL_BoundingBox();

	_SLIDER_HORIZONTAL_POSITION = _hBoundingBox.gStarting();
	_SLIDER_VERTICAL_POSITION = _vBoundngBox.gStarting();

	_SLIDER_HORIZONTAL_SIZE = _hBoundingBox.gSize();
	_SLIDER_VERTICAL_SIZE = _vBoundngBox.gSize();
}

const void ProgramBuffer::lSliders()
{
	_SLIDER_VERTICAL = new ProgramSlider(
		this, _DEBUG, 
			_SLIDER_VERTICAL_SIZE, 
			_SLIDER_VERTICAL_POSITION, 
			_SLIDERS_BG_COLOR, _SLIDERS_COLOR,
			0.5, 0.9, 
			ProgramSlider::Mode::Vertical
	);

	_SLIDER_HORIZONTAL = new ProgramSlider(
		this, _DEBUG,
			_SLIDER_HORIZONTAL_SIZE,
			_SLIDER_HORIZONTAL_POSITION,
			_SLIDERS_BG_COLOR, _SLIDERS_COLOR,
			0.5, 0.9,
			ProgramSlider::Mode::Horizontal
	);
}

Framework::Geometry::BoundingBox ProgramBuffer::g_SLIDER_HORIZONTAL_BoundingBox() const 
{
	const wxPoint _POSITION_1 = (
		wxPoint(0, _SIZE.y - _SLIDERS_HEIGHT)
		);
	const wxPoint _POSITION_2 = (
		_POSITION_1 + wxPoint(_SIZE.x - _SLIDERS_HEIGHT, _SLIDERS_HEIGHT)
		);
	return { _POSITION_1, _POSITION_2 };
}

Framework::Geometry::BoundingBox ProgramBuffer::g_SLIDER_VERTICAL_BoundingBox() const
{
	const wxPoint _POSITION_1 = (
		wxPoint(_SIZE.x - _SLIDERS_HEIGHT, 0)
		);
	const wxPoint _POSITION_2 = (
		_POSITION_1 + wxPoint(_SLIDERS_HEIGHT, _SIZE.y - _SLIDERS_HEIGHT)
		);
	return { _POSITION_1, _POSITION_2 };
}

Framework::Geometry::BoundingBox ProgramBuffer::g_BufferRenderBoundingBox() const
{
	wxPoint _POSITION_1 = FRAMEWORK_EMPTY_wxPOINT;

	if (_MODE ==
		ProgramBuffer::Mode::BufferEdit)
	{ // Buffer editing mode, needs show current line
		_POSITION_1 = wxPoint(_BUFFER_EDIT_RESERVED_SPACE_X, 0);
	}

	const wxPoint _POSITION_2 = (Framework::Transform::WxSizeToWxPoint2D(_SIZE) - wxPoint(_SLIDERS_HEIGHT, _SLIDERS_HEIGHT));

	return { _POSITION_1, _POSITION_2 };
}

void ProgramBuffer::hPaint(
	wxPaintEvent& _EVT
) {
	/* Main hook for paint operations in this class works on: wxAutoBufferedPaintDC */
	try {
		wxAutoBufferedPaintDC _CANVAS{ this };
		// Call render operations labeled with 'r'
		this->_BUFFER_EDIT_RESERVED_SPACE_X = g_BufferEditReservedSpace(); /* nessesary for buffer-edit to work at all */

		rBackground(_CANVAS);
		rSelection(_CANVAS);
		rBuffer(_CANVAS);

		if (_MODE ==
			ProgramBuffer::Mode::BufferEdit)
		{ // Buffer edit operations for render 
			rBufferEditBackground(_CANVAS);
			rBufferEditNumbers(_CANVAS);
			rBufferEditSeparator(_CANVAS);
		}
		rCursor(_CANVAS);
	} catch (const std::exception &ex) { } _EVT.Skip();
}


const void ProgramBuffer::rBackground(
	wxAutoBufferedPaintDC& _CANVAS
) {
	_CANVAS.SetPen(_BG_COLOR);
	_CANVAS.SetBrush(_BG_COLOR);
	_CANVAS.DrawRectangle(GetRect());

	/* Sliders connection (rect) */
	const wxPoint _SLIDERS_CONNECT_POS = (Framework::Transform::WxSizeToWxPoint2D(GetSize()) - wxPoint(_SLIDERS_HEIGHT, _SLIDERS_HEIGHT));
	const wxSize _SLIDERS_CONNECT_SIZE = wxSize(_SLIDERS_HEIGHT, _SLIDERS_HEIGHT);

	_CANVAS.SetPen(_SLIDERS_BG_COLOR);
	_CANVAS.SetBrush(_SLIDERS_BG_COLOR);

	_CANVAS.DrawRectangle(
		_SLIDERS_CONNECT_POS, _SLIDERS_CONNECT_SIZE
	);
}

const void ProgramBuffer::rSelection(
	wxAutoBufferedPaintDC& _CANVAS
) {
	_CANVAS.SetPen(*wxRED);
	_CANVAS.SetBrush(*wxRED);
	_CANVAS.SetFont(_TEXT_FONT);

	if (_SELECTION_STARTED_AT_LINE ==
		-1 || _SELECTION_STARTED_AT_CHAR == -1) return; /* No text selected: abort */

	const bool _1_LINER = (
		_SELECTION_ENDED_AT_LINE == _SELECTION_STARTED_AT_LINE ? true : false
		);

	const int _CHAR_LEN_X = _CANVAS.GetTextExtent("a").x;
	const int _BUFFER_LINE_SIZE_Y = g_BufferLineSizeY();

	if (_1_LINER ==
		true) // 1 Line only 
	{
		int _cSTART = _SELECTION_STARTED_AT_CHAR;
		int _cEND = _SELECTION_ENDED_AT_CHAR;

		if (_cSTART > _cEND)
		{ // Case: ending char is further than starting char 
			_cEND = _SELECTION_STARTED_AT_CHAR;
			_cSTART = _SELECTION_ENDED_AT_CHAR;
		}
		
		const wxPoint _lPosition = (
			g_BufferLinePositionInBoundingBox(_SELECTION_ENDED_AT_LINE) + 
				wxPoint(_cSTART * _CHAR_LEN_X ,0)
			);
		const wxSize _lSize = wxSize(
			_CHAR_LEN_X * (_cEND - _cSTART + 1), _BUFFER_LINE_SIZE_Y
			);

		_CANVAS.DrawRectangle(_lPosition, _lSize);

		return; 
	}

	const bool _SELECTION_FROM_ABOVE_TO_DOWN = (
		_SELECTION_ENDED_AT_LINE > _SELECTION_STARTED_AT_LINE
		);

	size_t _START_ITERATING_AT = -1;
	size_t _END_ITERATING_AT = -1;

	if (_SELECTION_FROM_ABOVE_TO_DOWN
		== true)
	{
		int _cSTARTc = _SELECTION_STARTED_AT_CHAR;

		const wxPoint _lPosition = (
			g_BufferLinePositionInBoundingBox(_SELECTION_STARTED_AT_LINE) +
			wxPoint(_cSTARTc * _CHAR_LEN_X, 0)
			);

		const wxSize _lSize = wxSize(
			_CHAR_LEN_X * (_DATA[_SELECTION_STARTED_AT_LINE]._TEXT.length() - _cSTARTc), _BUFFER_LINE_SIZE_Y
		);

		const wxPoint _lPosition2 = (
			g_BufferLinePositionInBoundingBox(_SELECTION_ENDED_AT_LINE)
			);

		const wxSize _lSize2 = wxSize(
			_CHAR_LEN_X * (_SELECTION_ENDED_AT_CHAR + 1), _BUFFER_LINE_SIZE_Y
		);

		_CANVAS.DrawRectangle(_lPosition2, _lSize2);
		_CANVAS.DrawRectangle(_lPosition, _lSize);

		_START_ITERATING_AT = (_SELECTION_STARTED_AT_LINE + 1);
		_END_ITERATING_AT = (_SELECTION_ENDED_AT_LINE - 1);
	}

	const bool _SELECTION_FROM_DOWN_TO_ABOVE = (
		_SELECTION_ENDED_AT_LINE < _SELECTION_STARTED_AT_LINE
		);

	if (_SELECTION_FROM_DOWN_TO_ABOVE
		== true)
	{
		int _cSTARTd = _SELECTION_STARTED_AT_CHAR;

		const wxPoint _lPosition = (
			g_BufferLinePositionInBoundingBox(_SELECTION_STARTED_AT_LINE)
			);

		const wxSize _lSize = wxSize(
			_CHAR_LEN_X * (_cSTARTd + 1), _BUFFER_LINE_SIZE_Y
		);

		const wxPoint _lPosition2 = (
			g_BufferLinePositionInBoundingBox(_SELECTION_ENDED_AT_LINE)
			) + wxPoint(_SELECTION_ENDED_AT_CHAR * _CHAR_LEN_X, 0);

		const wxSize _lSize2 = wxSize(
			_CHAR_LEN_X * (_DATA[_SELECTION_STARTED_AT_LINE]._TEXT.length() - _SELECTION_ENDED_AT_CHAR + 1), _BUFFER_LINE_SIZE_Y
		);

		_CANVAS.DrawRectangle(_lPosition2, _lSize2);
		_CANVAS.DrawRectangle(_lPosition, _lSize);

		_START_ITERATING_AT = (_SELECTION_ENDED_AT_LINE + 1);
		_END_ITERATING_AT = (_SELECTION_STARTED_AT_LINE - 1);
	}

	if (_START_ITERATING_AT == -1
		||
		_END_ITERATING_AT == -1
		) return; 

	Framework::Geometry::BoundingBox _BUFFER_RENDER_BOUDING_BOX = g_BufferRenderBoundingBox();

	const int _LINE_SIZE_Y = g_BufferLineSizeY();

	const size_t _RENDER_CHECK_START = (
		(std::abs(_SCROLL_Y) / _LINE_SIZE_Y)
		);

	const size_t _RENDER_CHECK_END = (
		_RENDER_CHECK_START + (_BUFFER_RENDER_BOUDING_BOX.gSize().y / _LINE_SIZE_Y)
		);

	for (
		size_t _ITERATOR = _RENDER_CHECK_START; _ITERATOR <= _RENDER_CHECK_END; _ITERATOR++
		)
	{
		if (_START_ITERATING_AT <= _ITERATOR && _ITERATOR <= _END_ITERATING_AT)
		{
			const wxPoint _POS = g_BufferLinePositionInBoundingBox(_ITERATOR);

			wxSize _SIZE = wxSize(
				(_DATA[_ITERATOR]._TEXT.length() * _CHAR_LEN_X), _BUFFER_LINE_SIZE_Y
			);

			if (_SIZE.x == 0) 
			{ /* Case: size equals 0 -> line is empty */
				_SIZE = wxSize(_CHAR_LEN_X, _SIZE.y);
			}

			_CANVAS.DrawRectangle(_POS, _SIZE);
		}
	}

}


const void ProgramBuffer::rBuffer(
	wxAutoBufferedPaintDC& _CANVAS
) {
	_CANVAS.SetTextForeground(_DEFAULT_TEXT_COLOR);
	_CANVAS.SetTextBackground(_DEFAULT_TEXT_COLOR);
	_CANVAS.SetFont(_TEXT_FONT);

	Framework::Geometry::BoundingBox _rBUFFER_BOUNDING_BOX = g_BufferRenderBoundingBox();

	_CANVAS.SetClippingRegion(_rBUFFER_BOUNDING_BOX.gRect());

	const wxPoint _STARTING_POS = (
		wxPoint(0, 0)
		);
	

	const int _LINE_Y = g_BufferLineSizeY();

	int _RENDER_START_AT = std::max(0, (
		std::abs((int)_SCROLL_Y) / _LINE_Y - 1
		));

	int _RENDER_END_AT = std::min((int)_DATA.size(), (
		_RENDER_START_AT + (_rBUFFER_BOUNDING_BOX.gSize().y / _LINE_Y) + 2
		));

	for (size_t _AT
		= _RENDER_START_AT; _AT < _RENDER_END_AT; _AT++)
	{
		int _PUSH_X = 0; 

		for (const Framework::Transform::ColoredLineHex& _COLORED_LINE
			: Framework::Transform::StringToColoredLines(_DATA[_AT]._TEXT_RAW))
		{
			const wxPoint _cPOSITION = (
				_STARTING_POS + g_BufferLinePositionInBoundingBox(_AT) + wxPoint(_PUSH_X, 0)
				);

			if (_COLORED_LINE.gColor() 
				!= "DEFCLR"
				) { // Color given in param 
				_CANVAS.SetTextForeground(
					Framework::Transform::StringToHexColor(_COLORED_LINE.gColor())
					);
				_CANVAS.SetTextBackground(
					Framework::Transform::StringToHexColor(_COLORED_LINE.gColor())
					);
				
			} else {
				// Default color 
				_CANVAS.SetTextForeground(_DEFAULT_TEXT_COLOR);
				_CANVAS.SetTextBackground(_DEFAULT_TEXT_COLOR);
			}

			_CANVAS.DrawText(_COLORED_LINE.gText(), _cPOSITION);

			_PUSH_X += _CANVAS.GetTextExtent(_COLORED_LINE.gText()).x;
		}
	}

	_CANVAS.DestroyClippingRegion();
}

const wxPoint ProgramBuffer::g_BufferLinePositionInBoundingBox(
	const size_t& _AT
) const {
	const int _SPACING_BETWEEN_EVERY_LINE = g_BufferLineSizeY();

	return (g_BufferRenderBoundingBox().gStarting() + wxPoint(0, 
		((int)_AT * _SPACING_BETWEEN_EVERY_LINE))
		+ wxPoint(_SCROLL_X, _SCROLL_Y) 
		); // With scroll included 
}

const int ProgramBuffer::g_BufferLineSizeX(
	const std::string& _LINE
) const {
	wxMemoryDC _CANVAS; 
	_CANVAS.SetFont(_TEXT_FONT);
	return _CANVAS.GetTextExtent(_LINE).x; 
}

const int ProgramBuffer::g_BufferLineSizeY() const
{
	wxMemoryDC _CANVAS; 
	_CANVAS.SetFont(_TEXT_FONT);

	return (_CANVAS.GetTextExtent("RegularTextLine").y + _TEXT_SPACING_HEIGHT);
}

const wxPoint ProgramBuffer::g_MaxScrolls() const
{
	size_t _LONGEST_X = 0;
	size_t _LONGEST_X_ID = 0;

	size_t _AT = 0;
	for (const BufferElement& _LINE : _DATA)
	{
		const size_t _LEN = _LINE._TEXT.length();
		if (_LEN >
			_LONGEST_X)
		{
			_LONGEST_X = _LEN;
			_LONGEST_X_ID = _AT;
		} _AT++;
	}
	
	const int _MAX_SCROLL_X = g_BufferLineSizeX(_DATA.size() != 0 ? _DATA[_LONGEST_X_ID]._TEXT : FRAMEWORK_EMPTY_STRING);// _LONGEST_X;
	const int _MAX_SCROLL_Y = (g_BufferLineSizeY() * _DATA.size());

	const wxSize _rBOUNDING_BOX_SIZE = g_BufferRenderBoundingBox().gSize();

	wxPoint _DIFs = Framework::Transform::WxSizeToWxPoint2D(
		(wxSize(_MAX_SCROLL_X, _MAX_SCROLL_Y) - _rBOUNDING_BOX_SIZE));

	_DIFs = wxPoint(
		std::max(0, _DIFs.x), std::max(0, _DIFs.y)
	); return _DIFs; 
}

const void ProgramBuffer::lScrollUpdateHook()
{
	_SCROLL_UPDATE_TIMER.SetOwner(this);
	_SCROLL_UPDATE_TIMER.Start(5);

	// Refresh : 5 ms 
	Bind(wxEVT_TIMER, &ProgramBuffer::hScrollUpdate, this, _SCROLL_UPDATE_TIMER.GetId());
}

void ProgramBuffer::hScrollUpdate(
	wxTimerEvent& _EVT)
{ 
	try {
		suHandle();
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); } _EVT.Skip();
}

const void ProgramBuffer::suHandle()
{
	const wxPoint _MAX_SCROLLs = g_MaxScrolls(); 

	const int _SCROLL_X_c
		= -((double)_MAX_SCROLLs.x * _SLIDER_HORIZONTAL->gScrollProcentage());
	
	const int _SCROLL_Y_c
		= -((double)_MAX_SCROLLs.y * _SLIDER_VERTICAL->gScrollProcentage());

	if (_SCROLL_X == _SCROLL_X_c && _SCROLL_Y == _SCROLL_Y_c)
		return;

	_SCROLL_X = _SCROLL_X_c;
	_SCROLL_Y = _SCROLL_Y_c;

	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const void ProgramBuffer::pNextLine(
	const std::string& _LINE
) {
	/* Format element to match data type */
	ProgramBuffer::BufferElement _ELEMENT; 
	_ELEMENT._TEXT = Framework::Transform::RemoveAllColorEscapesFromAString(_LINE);
	_ELEMENT._TEXT_RAW = _LINE;
	/* Push element, should be done :) */
	this->_DATA.push_back(
		_ELEMENT);
	
	/* Force buffer update variable to apply syntax etc */
	_BUFFER_UPDATED = true; 
}

const void ProgramBuffer::lMouseHooks()
{
	Bind(wxEVT_MOTION, &ProgramBuffer::hMouseMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &ProgramBuffer::hMouseExit, this);
	Bind(wxEVT_LEFT_UP, &ProgramBuffer::hMouseLeftUp, this);
	Bind(wxEVT_LEFT_DCLICK, &ProgramBuffer::hMouseDClick, this);
	// setup _MOUSE_UPDATE_TIMER 
	_MOUSE_UPDATE_TIMER.SetOwner(this);
	_MOUSE_UPDATE_TIMER.Start(_SELECTION_UPDATE_MS);

	Bind(wxEVT_TIMER, &ProgramBuffer::hMouseUpdate, this, _MOUSE_UPDATE_TIMER.GetId());
}

void ProgramBuffer::hMouseMotion(
	wxMouseEvent& _EVT
) {
	try {
		mHandleSelectionStart(_EVT);
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); } _EVT.Skip();
}

const void ProgramBuffer::mHandleSelectionStart(
	wxMouseEvent& _EVT
) {
	if (_SELECTION) return; // If selecting then why start?

	const bool _LMB_STATE = _EVT.LeftIsDown(); 

	if (_SELECTION_MOUSE_LAST_CHECKED_ACTIVE && _LMB_STATE)
		return; 
	if (_SELECTION_MOUSE_LAST_CHECKED_ACTIVE && not _LMB_STATE)
	{ _SELECTION_MOUSE_LAST_CHECKED_ACTIVE = false; return; }

	if (not _SELECTION_MOUSE_LAST_CHECKED_ACTIVE && not _LMB_STATE)
		return; 

	Framework::Geometry::BoundingBox _BUFFER_RENDER_BOUNDING_BOX = g_BufferRenderBoundingBox();

	if (Framework::Geometry::cBounds(
		_EVT.GetPosition(), _BUFFER_RENDER_BOUNDING_BOX.gStarting(), _BUFFER_RENDER_BOUNDING_BOX.gSize()
	) != true) return; // Not in bounding box of buffer render, how did we get that far???

	// Apply settings for selection mode 
	_SELECTION_MOUSE_LAST_CHECKED_ACTIVE = true;
	_SELECTION = true; 

	_SELECTION_STARTED_AT_LINE = std::min( /* Limit to avoid vector overflow */
		(int)(_DATA.size() - 1), g_BufferLineAtGivenPosition(_EVT.GetPosition()));

	_SELECTION_STARTED_AT_CHAR = g_BufferLineCharAtGivenPosition(_EVT.GetPosition(), _SELECTION_STARTED_AT_LINE);

	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

void ProgramBuffer::hMouseExit(
	wxMouseEvent& _EVT
) { // Mouse exited the panel 
	this->mHandleMouseExit(); _EVT.Skip();
}

const void ProgramBuffer::mHandleMouseExit()
{ // Just politely ask for reconfirmation if mouse wasn't held when going into a panel :) 
	_SELECTION_MOUSE_LAST_CHECKED_ACTIVE = true; // Force double check in 'mHandleSelectionStart'
}

void ProgramBuffer::hMouseUpdate(
	wxTimerEvent& _EVT
) {
	/* Hook running every timer call, change runtime: in 'l' function */
	try {
		mHandleSelection(); 
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); } _EVT.Skip();
}

const void ProgramBuffer::mHandleSelection()
{
	/*
		Reminder: Comments in this function are completly useless, they dont have any
			impact on reading the function, those are correct but we could have the same result with just one or two.
	*/
	if (not _SELECTION) return; // abort if not selecing, selection start is handled in 'mHandleSelectionStart' :)

	if (not wxGetMouseState().LeftIsDown())
	{ _SELECTION = false; return; } // Why the fuck still selecting? CHANGE IT!

	const wxPoint _MOUSE_POSITION = wxGetMousePosition(); // Position in window needs fixing 

	const wxPoint _MOUSE_POSITION_FIXED = (
		_MOUSE_POSITION - GetScreenPosition()
		);

	bool _MODIFY_VERTICAL_SCROLL = false; 
	int _MODIFY_VERTICAL_SCROLL_DIRECTION = 0;
	int _MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX = -1; 

	int _MODIFY_HORIZONTAL_SCROLL = false; 
	int _MODIFY_HORIZONTAL_SCROLL_DIRECTION = 0;
	int _MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX_HORIZONTAL = -1;


	Framework::Geometry::BoundingBox _RENDER_BUFFER_BOUNDING_BOX = g_BufferRenderBoundingBox();

	if (Framework::Geometry::cBound1D( // 1D check just to check y axis (vertical)
		_MOUSE_POSITION_FIXED.y, _RENDER_BUFFER_BOUNDING_BOX.gStarting().y, _RENDER_BUFFER_BOUNDING_BOX.gSize().y
	) != true) // Vertical scroll has to be modified
	{ // Perform actions 
		_MODIFY_VERTICAL_SCROLL = true; 
		// Determinate direction of scrolling 
		if (_MOUSE_POSITION_FIXED.y > _RENDER_BUFFER_BOUNDING_BOX.gFinal().y)
		{ 
			_MODIFY_VERTICAL_SCROLL_DIRECTION = 1; 

			_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX = (
				_MOUSE_POSITION_FIXED.y - _RENDER_BUFFER_BOUNDING_BOX.gFinal().y 
				); // dist from bottom 
		} else // + operation
		{ 
			_MODIFY_VERTICAL_SCROLL_DIRECTION = -1; 
			
			_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX = (
				_RENDER_BUFFER_BOUNDING_BOX.gStarting().y - _MOUSE_POSITION_FIXED.y
				); // dist from top 
		} // - operation
	}

	if (Framework::Geometry::cBound1D( // 1D check just to check x axis (horizontal)
		_MOUSE_POSITION_FIXED.x, _RENDER_BUFFER_BOUNDING_BOX.gStarting().x, _RENDER_BUFFER_BOUNDING_BOX.gSize().x
	) != true) // Vertical scroll has to be modified
	{ // Perform actions 
		_MODIFY_HORIZONTAL_SCROLL = true;
		// Determinate direction of scrolling 
		if (_MOUSE_POSITION_FIXED.x > _RENDER_BUFFER_BOUNDING_BOX.gFinal().x)
		{
			_MODIFY_HORIZONTAL_SCROLL_DIRECTION = 1;

			_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX_HORIZONTAL = (
				_MOUSE_POSITION_FIXED.x - _RENDER_BUFFER_BOUNDING_BOX.gFinal().x
				); // dist from bottom 
		}
		else // + operation
		{
			_MODIFY_HORIZONTAL_SCROLL_DIRECTION = -1;

			_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX_HORIZONTAL = (
				_RENDER_BUFFER_BOUNDING_BOX.gStarting().x - _MOUSE_POSITION_FIXED.x
				); // dist from top 
		} // - operation
	}

	int _MOUSE_DISTANCE_TO_ROLL_ONE_LINE_PER_QUARTER_OF_A_SECOND = 20;

	if (_MODIFY_VERTICAL_SCROLL == true || _MODIFY_HORIZONTAL_SCROLL 
		== true) // Perform a check to validate scroll modifying operations (cool words but no need for them xd)
	{ // Modify scroll 
		double _MODIFY_SCROLL_BY = (
			100.0 / (
				((double)g_MaxScrolls().y / (double)g_BufferLineSizeY()) * (double)g_BufferLineSizeY()
				) / (1000.0 / (double)_SELECTION_UPDATE_MS)
			) * 2; 

	

		double _MODIFY_SCROLL_BY_HORIZONTAL = (1.0 / (1000.0 / (double)_SELECTION_UPDATE_MS)) / 4;
		double _MODIFY_SCROLL_BY_VERTICAL = _MODIFY_SCROLL_BY;

		if (_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX >
			_MOUSE_DISTANCE_TO_ROLL_ONE_LINE_PER_QUARTER_OF_A_SECOND)
		{ // Change scrolling value to match the mouse distance
			_MODIFY_SCROLL_BY_VERTICAL = (_MODIFY_SCROLL_BY *
				double(_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX - _MOUSE_DISTANCE_TO_ROLL_ONE_LINE_PER_QUARTER_OF_A_SECOND) / (double) 10
				); // every 10 pixels from given positon equals x2
		}

		if (_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX_HORIZONTAL >
			_MOUSE_DISTANCE_TO_ROLL_ONE_LINE_PER_QUARTER_OF_A_SECOND)
		{ // Change scrolling value to match the mouse distance
			_MODIFY_SCROLL_BY_HORIZONTAL = (_MODIFY_SCROLL_BY_HORIZONTAL *
				double(_MOUSE_POSITION_DISTANCE_FROM_BOUNDING_BOX_HORIZONTAL - _MOUSE_DISTANCE_TO_ROLL_ONE_LINE_PER_QUARTER_OF_A_SECOND) / (double)10
				); // every 10 pixels from given positon equals x2
		}

		if (_MODIFY_VERTICAL_SCROLL_DIRECTION == 
			-1) // Change modify mode from: +, to: -
		{ _MODIFY_SCROLL_BY_VERTICAL = -_MODIFY_SCROLL_BY_VERTICAL; }

		if (_MODIFY_HORIZONTAL_SCROLL_DIRECTION == 
			-1) // Change modify mode from: +, to: -
		{ _MODIFY_SCROLL_BY_HORIZONTAL = -_MODIFY_SCROLL_BY_HORIZONTAL;
			
		}

		if (_MODIFY_VERTICAL_SCROLL
			== true)
		{
			_SLIDER_VERTICAL->sScrollProcentage(
				std::max(0.0, std::min(1.0,
					(_SLIDER_VERTICAL->gScrollProcentage() + _MODIFY_SCROLL_BY_VERTICAL)
				)));
		}

		if (_MODIFY_HORIZONTAL_SCROLL
			== true)
		{
			_SLIDER_HORIZONTAL->sScrollProcentage(
				std::max(0.0, std::min(1.0,
					(_SLIDER_HORIZONTAL->gScrollProcentage() + _MODIFY_SCROLL_BY_HORIZONTAL)
				)));
		}

		// Refresh to finalize all the things done 
	}

	const int _BOUNDING_BOX_START_X = _RENDER_BUFFER_BOUNDING_BOX.gStarting().x;
	const int _BOUNDING_BOX_START_Y = _RENDER_BUFFER_BOUNDING_BOX.gStarting().y;
	
	const int _BOUNDING_BOX_END_X = _RENDER_BUFFER_BOUNDING_BOX.gFinal().x; 
	const int _BOUNDING_BOX_END_Y = _RENDER_BUFFER_BOUNDING_BOX.gFinal().y;

	const wxPoint _MOUSE_POSITION_IN_FRAME = wxPoint
		(
			std::max(_BOUNDING_BOX_START_X, 
				std::min(_BOUNDING_BOX_END_X, _MOUSE_POSITION_FIXED.x)
			),
			std::max(_BOUNDING_BOX_START_Y, 
				std::min(_BOUNDING_BOX_END_Y, _MOUSE_POSITION_FIXED.y)
			)
		);

	// Apply selection 
	_SELECTION_ENDED_AT_LINE = g_BufferLineAtGivenPosition(
		_MOUSE_POSITION_IN_FRAME
	);

	_SELECTION_ENDED_AT_CHAR = g_BufferLineCharAtGivenPosition(
		_MOUSE_POSITION_IN_FRAME, _SELECTION_ENDED_AT_LINE
	);

	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const int ProgramBuffer::g_BufferLineAtGivenPosition(
	const wxPoint& m_POSITION
) const 
{
	Framework::Geometry::BoundingBox _RENDER_BOUNDING_BOX = g_BufferRenderBoundingBox();

	const wxPoint m_POSITION_IN_BOUNDING_BOX = (
		m_POSITION - _RENDER_BOUNDING_BOX.gStarting()
		);

	const int _LINE_HEIGHT = g_BufferLineSizeY();

	const long double _SCROLLED_LINES_Y = (
		(long double)std::abs(_SCROLL_Y) / (long double)_LINE_HEIGHT
		);

	const long double _ON_SCREEN_LINE_NUMBER = (
		(long double)m_POSITION_IN_BOUNDING_BOX.y / (long double)_LINE_HEIGHT
		);

	const long double _DEST_LINE = (
		(long double)_SCROLLED_LINES_Y + (long double)_ON_SCREEN_LINE_NUMBER
		);

	return (size_t)_DEST_LINE;
}

const size_t ProgramBuffer::g_BufferLineCharAtGivenPosition(
	const wxPoint& m_POSITION, const size_t& _AT, const bool _LEN_CHECK 
) const {
	Framework::Geometry::BoundingBox _RENDER_BOUNDING_BOX = g_BufferRenderBoundingBox();
	// Create canvas 
	wxMemoryDC _CANVAS;
	_CANVAS.SetFont(_TEXT_FONT);
	// Determinate len of a char 
	const int _CHAR_LEN = 
		_CANVAS.GetTextExtent("a").x;

	// Divide fixed mouse pos by char 
	const size_t _AT_CHAR = (
		((m_POSITION - _RENDER_BOUNDING_BOX.gStarting()).x + std::abs(_SCROLL_X)) / _CHAR_LEN
		);

	const size_t _LAST_LINE_LEN = _DATA[_AT]._TEXT.length();
	// Return with values cooresponding to a string 
	return std::max((size_t)0,
		std::min((_LEN_CHECK == true ? (_LAST_LINE_LEN == 0 ? 0 : _LAST_LINE_LEN - 1) : _AT_CHAR),
			_AT_CHAR)
		); // plz work 0-0
}

const int ProgramBuffer::g_BufferEditReservedSpace() const
{
	wxMemoryDC _CANVAS; 
	_CANVAS.SetFont(_TEXT_FONT);

	return (
		(std::to_string(_DATA.size()).length() * _CANVAS.GetTextExtent("a").x) + // Numbers len 
		_BUFFER_EDIT_SEPARATION_LEN_X + 
		_BUFFER_EDIT_SEPARATOR_LEN_X +
		_BUFFER_EDIT_NUMBERS_SEPARATION_LEN_X
		);
}

const void ProgramBuffer::rBufferEditBackground(
	wxAutoBufferedPaintDC& _CANVAS
) {
	_CANVAS.SetBrush(_BUFFER_EDIT_NUMBERS_BG_COLOR);
	_CANVAS.SetPen(_BUFFER_EDIT_NUMBERS_BG_COLOR);

	_CANVAS.DrawRectangle(wxPoint(0, 0),
		wxSize(g_BufferEditReservedSpace() - _BUFFER_EDIT_SEPARATION_LEN_X, g_BufferRenderBoundingBox().gSize().y)
		);
}

const void ProgramBuffer::rBufferEditNumbers(
	wxAutoBufferedPaintDC& _CANVAS
) {
	const wxPoint _STARTING_POSITION = wxPoint
		(_BUFFER_EDIT_NUMBERS_SEPARATION_LEN_X / 2, _SCROLL_Y);

	_CANVAS.SetFont(_TEXT_FONT);
	_CANVAS.SetTextForeground(_BUFFER_EDIT_NUMBERS_COLOR);
	_CANVAS.SetTextBackground(_BUFFER_EDIT_NUMBERS_COLOR);

	wxPoint _POS = _STARTING_POSITION;

	Framework::Geometry::BoundingBox _rBUFFER_BOUNDING_BOX = g_BufferRenderFullBoundingBox();

	const int _LINE_Y = g_BufferLineSizeY();

	int _RENDER_START_AT = std::max(0, (
		std::abs((int)_SCROLL_Y) / _LINE_Y - 1
		));

	int _RENDER_END_AT = std::min((int)_DATA.size(), (
		_RENDER_START_AT + (_rBUFFER_BOUNDING_BOX.gSize().y / _LINE_Y) + 2
		));

	for (size_t _ITERATOR = _RENDER_START_AT; _ITERATOR < _RENDER_END_AT; _ITERATOR++)
	{
		const int _POSITION_Y = _ITERATOR * _LINE_Y; 
		
		const wxPoint _CURRENT_POS = (_STARTING_POSITION +
			wxPoint(0, _POSITION_Y));

		_CANVAS.DrawText(std::to_string(
			_ITERATOR 
		), _CURRENT_POS);
	}
}

Framework::Geometry::BoundingBox ProgramBuffer::g_BufferRenderFullBoundingBox() const
{
	return { wxPoint(0, 0), g_BufferRenderBoundingBox().gFinal() };
}

const void ProgramBuffer::rBufferEditSeparator(
	wxAutoBufferedPaintDC& _CANVAS
) {
	const wxPoint _SEPARATOR_POS = (
		wxPoint(_BUFFER_EDIT_RESERVED_SPACE_X - _BUFFER_EDIT_SEPARATOR_LEN_X - _BUFFER_EDIT_SEPARATION_LEN_X, 0)
		);
	const wxSize _SEPARATOR_SIZE = (
		wxSize(_BUFFER_EDIT_SEPARATOR_LEN_X, g_BufferRenderBoundingBox().gSize().y)
		);
	_CANVAS.SetPen(_BUFFER_EDIT_SEPARATOR_COLOR);
	_CANVAS.SetBrush(_BUFFER_EDIT_SEPARATOR_COLOR);
	_CANVAS.DrawRectangle(_SEPARATOR_POS, _SEPARATOR_SIZE);
}

void ProgramBuffer::hMouseLeftUp(
	wxMouseEvent& _EVT
) {
	try {
		mHandleCursorPlacement(_EVT);
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }; _EVT.Skip();
}

const void ProgramBuffer::mHandleCursorPlacement(
	wxMouseEvent& _EVT
) {
	if ( // "Buffer edit" mode 
		_MODE == ProgramBuffer::Mode::BufferEdit)
	{
		const wxPoint _MOUSE_POSITION = _EVT.GetPosition();

		const wxPoint _MOUSE_POSITION_FIXED = (
			_MOUSE_POSITION - g_BufferRenderBoundingBox().gStarting() 
			);

		_CURSOR_AT_LINE = 
			std::max(0, 
				std::min(
					(int)(_DATA.size() - 1),
				g_BufferLineAtGivenPosition(_MOUSE_POSITION_FIXED)));

		_CURSOR_AT_CHAR = std::max((int)0,
				std::min(
					(int)(_DATA[_CURSOR_AT_LINE]._TEXT.length()), (int)g_BufferLineCharAtGivenPosition(_MOUSE_POSITION, _CURSOR_AT_LINE, false)
				)
			);
		
		/* Refresh */
		fSmartRefresh(g_BufferRenderBoundingBox().gRect());
		return;
	}

}

const void ProgramBuffer::rCursor(
	wxAutoBufferedPaintDC& _CANVAS
) {
	if (_CURSOR_AT_CHAR == -1
		|| _CURSOR_AT_LINE == -1)
		return; 

	if (_RENDER_CURSOR ==
		false) return; /* Cursor blinking purposes */

	_CANVAS.SetFont(_TEXT_FONT);

	const wxPoint _CURSOR_POSITION = (
		g_BufferLinePositionInBoundingBox(_CURSOR_AT_LINE) + wxPoint(
			_CANVAS.GetTextExtent("a").x * _CURSOR_AT_CHAR,
		0));


	const wxSize _CURSOR_SIZE = wxSize(1, g_BufferLineSizeY());

	_CANVAS.SetPen(*wxWHITE);
	_CANVAS.SetBrush(*wxBLUE);
	_CANVAS.DrawRectangle(_CURSOR_POSITION, _CURSOR_SIZE);
}

void ProgramBuffer::hOnChar(
	wxKeyEvent& _EVT
) {

	try {
		//_DEBUG->Push(std::to_string(_EVT.GetKeyCode()));// 8 : 308 (backspace : ctrl) 315 : 317 || 314 : 316

		/* Sent buffer update await for syntax etc */
		_BUFFER_UPDATED = true; 

		std::string _ACCEPTED_CHARS = "1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm, ./`~!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?";
		
		const int _EVENT_KEYCODE = _EVT.GetKeyCode(); 
		bool _ACCEPTED = false; 

		const bool _ENTER = (_EVENT_KEYCODE == 13);
		const bool _BACKSPACE = (_EVENT_KEYCODE == 8);

		const bool _ARROW_UP = (_EVENT_KEYCODE == 315);
		const bool _ARROW_DOWN = (_EVENT_KEYCODE == 317);
		const bool _ARROW_LEFT = (_EVENT_KEYCODE == 314); 
		const bool _ARROW_RIGHT = (_EVENT_KEYCODE == 316);
		const bool _CTRL_PLUS_C = (_EVENT_KEYCODE == 67 && _EVT.m_controlDown);
		const bool _SELECTED_DESTROY = (_SELECTION_STARTED_AT_LINE != -1 && _SELECTION_ENDED_AT_LINE != -1 && _BACKSPACE);

		if (_MODE == ProgramBuffer::Mode::BufferEdit)
		{
			if (_ENTER)
			{
				cBufferEditHandleNextLine();
			}
			if (_BACKSPACE)
			{
				if (_SELECTED_DESTROY)
				{
					cBufferEditSelectionDestroy(); return;
				}
				cBufferEditHandleDeletion(_EVT);
			}
			if (_ARROW_UP || _ARROW_DOWN
				|| _ARROW_LEFT || _ARROW_RIGHT)
			{ // arrows 
				cBufferEditHandleCursorMovementWithArrows(_EVT);
			}

			if (_CTRL_PLUS_C)
			{
				cHandleSelectionCopyToClipboard();
				return; 
			}
		}


		for (size_t _ITERATOR = 0; _ITERATOR < _ACCEPTED_CHARS.length();
			_ITERATOR++)
		{ // Check if accepted
			const std::string _CURRENT_CHAR = std::string(1, _ACCEPTED_CHARS.at(_ITERATOR));

			
			if (_CURRENT_CHAR == std::string(1, _EVT.GetUnicodeKey()))
			{ _ACCEPTED = true; break; }
		}
		if (_ACCEPTED == false)
			return; // Abort if char verification failed 

		std::string _GIVEN_CHAR = FRAMEWORK_EMPTY_STRING;

		const std::string _CHARS = "~1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./ ";
		const std::string _REPLACEMENTS = "~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>? ";

		for (size_t _ITERATOR = 0; _ITERATOR < _CHARS.length();
			_ITERATOR++)
		{
			const std::string _C_CHAR = std::string(1, _CHARS.at(_ITERATOR));
			if ( _C_CHAR == 
				std::string(1, std::tolower(_EVT.GetUnicodeKey())) )
			{ // Found
				if (_EVT.m_shiftDown)
				{ // Convert 
					_GIVEN_CHAR = std::string(1, _REPLACEMENTS[_ITERATOR]);
				} else { _GIVEN_CHAR = std::string(1, std::tolower(_EVT.GetUnicodeKey())); }

				break;
			}

		}

		
		if (_GIVEN_CHAR.empty() ==
			true) return; 

		if (_MODE == ProgramBuffer::Mode::BufferEdit)
		{ // Buffer edit actions 
			cBufferEditHandleTyping(_GIVEN_CHAR);
		}
	} catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }

	_EVT.Skip();
}

const void ProgramBuffer::cBufferEditHandleTyping(
	const std::string& _CHAR
) {
	if (_CURSOR_AT_CHAR == -1 ||
		_CURSOR_AT_LINE == -1)
		return; 
	// Working on a given ptr&
	ProgramBuffer::BufferElement& _LINE_REFERENCE = _DATA[_CURSOR_AT_LINE];

	const bool _AT_CHAR_ZERO = (_CURSOR_AT_CHAR == 0);
	const bool _AT_CHAR_MAX = ((_CURSOR_AT_CHAR == _LINE_REFERENCE._TEXT.length()) && not _AT_CHAR_ZERO);
	const bool _AT_CHAR_IN_THE_MIDDLE = (not _AT_CHAR_ZERO && not _AT_CHAR_MAX);
		
	if (_AT_CHAR_ZERO)
	{ _LINE_REFERENCE._TEXT = (_CHAR + 
		_LINE_REFERENCE._TEXT); }

	if (_AT_CHAR_MAX)
	{ _LINE_REFERENCE._TEXT += _CHAR; }

	if (_AT_CHAR_IN_THE_MIDDLE)
	{
		const std::string _SEGMENT_FIRST = _LINE_REFERENCE._TEXT.substr(0, _CURSOR_AT_CHAR);
		const std::string _SEGMENT_LAST = _LINE_REFERENCE._TEXT.substr(_CURSOR_AT_CHAR);

		const std::string _ASSEMBLED = (
			_SEGMENT_FIRST + _CHAR + _SEGMENT_LAST
			);
		_LINE_REFERENCE._TEXT = _ASSEMBLED;
	}

	// Fullfill external data 
	_LINE_REFERENCE._TEXT_RAW = (std::string)_LINE_REFERENCE._TEXT;
	hdl_SmartSyntaxApply();
	_CURSOR_AT_CHAR++;

	/* Refresh */
	dSetScrollCorespondingToCursorPositionY();
	dSetScrollCorespondingToCursorPositionX();
	_RENDER_CURSOR = true; 
	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const void ProgramBuffer::cBufferEditHandleNextLine()
{
	if (_CURSOR_AT_LINE == -1 ||
		_CURSOR_AT_CHAR == -1) return;
	
	// Working on a given ptr&
	ProgramBuffer::BufferElement& _LINE_REFERENCE = _DATA[_CURSOR_AT_LINE];

	const std::string _LINE_LEFTOVER = _LINE_REFERENCE._TEXT.substr(_CURSOR_AT_CHAR);
	
	// Handle the line cursor's at currently 
	_LINE_REFERENCE._TEXT = _LINE_REFERENCE._TEXT.substr(0, _CURSOR_AT_CHAR);
	_LINE_REFERENCE._TEXT_RAW = _LINE_REFERENCE._TEXT;

	// Handle insertion data for cursor's next line 
	const auto _VECTOR_WHERE_TO_INSERT = (_DATA.begin() +
		(size_t)_CURSOR_AT_LINE + 1);

	ProgramBuffer::BufferElement _INSERTION_DATA;
	_INSERTION_DATA._TEXT = _LINE_LEFTOVER;
	_INSERTION_DATA._TEXT_RAW = _INSERTION_DATA._TEXT;
	// Handle insertion 
	_DATA.insert(_VECTOR_WHERE_TO_INSERT, _INSERTION_DATA);

	// Handle variables so 'ProgramBuffer' handlers can update 
	_CURSOR_AT_CHAR = 0;
	_CURSOR_AT_LINE++;

	hdl_SmartSyntaxApply();

	/* Refresh */
	dSetScrollCorespondingToCursorPositionY();
	dSetScrollCorespondingToCursorPositionX();
	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const void ProgramBuffer::cBufferEditHandleDeletion(
	wxKeyEvent& _EVT
) {
	if (_CURSOR_AT_LINE == -1 || // Check to avoid errors in case of invaild lines (shouldn't happen btw)
		_CURSOR_AT_CHAR == -1) return;

	// Working on a given ptr&
	ProgramBuffer::BufferElement& _LINE_REFERENCE = _DATA[_CURSOR_AT_LINE];

	if (_CURSOR_AT_CHAR == // Current line deletion 
		0 && _CURSOR_AT_LINE != 0) // cannot be lines < 0 
	{
		const auto _VECTOR_ERASE_POSITION = (
			_DATA.begin() + _CURSOR_AT_LINE
			);
		
		ProgramBuffer::BufferElement& _AHEAD_LINE_REFERENCE = _DATA[(
			_CURSOR_AT_LINE - 1)]; // ptr to line ahead 

		const std::string _LINE_REFERENCE_TEXT_COPY = _LINE_REFERENCE._TEXT;
		const size_t _AHEAD_LINE_REFERENCE_TEXT_LEN = _AHEAD_LINE_REFERENCE._TEXT.length();

		// Fullfill the data of the line ahead
		_AHEAD_LINE_REFERENCE._TEXT += _LINE_REFERENCE_TEXT_COPY;
		_AHEAD_LINE_REFERENCE._TEXT_RAW = _AHEAD_LINE_REFERENCE._TEXT;
		// Remove current line 
		_DATA.erase(_VECTOR_ERASE_POSITION);

		// Handle cursor data 
		_CURSOR_AT_CHAR = _AHEAD_LINE_REFERENCE_TEXT_LEN;
		_CURSOR_AT_LINE--;

		// Finalize 
		fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
		return; // just to avoid memory leaks ;)
	}

	if (_CURSOR_AT_CHAR == 0) // handle 0-line bug: copying text over 
		return; 

	if (_EVT.m_controlDown // Control down, delete whole segment of the text infront instantly 
		== true) {
		_LINE_REFERENCE._TEXT = _LINE_REFERENCE._TEXT.substr(0, 
			_CURSOR_AT_CHAR);
		_CURSOR_AT_CHAR = 0;
	}
	else // Delete just char in front of cursor 
	{
		const std::string _SEGMENT_FIRST = _LINE_REFERENCE._TEXT.substr(0,
			_CURSOR_AT_CHAR - 1);
		const std::string _SEGMENT_LAST = _LINE_REFERENCE._TEXT.substr(_CURSOR_AT_CHAR);

		_LINE_REFERENCE._TEXT = (
			_SEGMENT_FIRST + _SEGMENT_LAST
			);
		_CURSOR_AT_CHAR = std::max(0, 
			_CURSOR_AT_CHAR - 1); // position of a cursor: char infront 
	}
	// Update values 
	if (_LINE_REFERENCE._TEXT == _LINE_REFERENCE._TEXT_RAW) return; // ensure memory leak won't happen

	_LINE_REFERENCE._TEXT_RAW = _LINE_REFERENCE._TEXT;

	/* Refresh */
	hdl_SmartSyntaxApply();

	dSetScrollCorespondingToCursorPositionY();
	dSetScrollCorespondingToCursorPositionX();
	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const void ProgramBuffer::cBufferEditHandleCursorMovementWithArrows(
	wxKeyEvent& _EVT
) {
	const int _ARROW_UP = 315;
	const int _ARROW_DOWN = 317;
	const int _ARROW_LEFT = 314;
	const int _ARROW_RIGHT = 316;
	
	const int _KEYCODE = _EVT.GetKeyCode();
	
	if (_ARROW_UP // Cursor up
		== _KEYCODE)
	{
		const int _WANTED_LINE = std::max(0,
			_CURSOR_AT_LINE - 1);

		cBufferEditHandleCursorMovementWithArrowsHandleXAxis(
			_WANTED_LINE);
		_CURSOR_AT_LINE = _WANTED_LINE;
	}

	if (_ARROW_DOWN // Cursor down 
		== _KEYCODE)
	{
		const int _WANTED_LINE = std::min((int)(_DATA.size() - 1),
			_CURSOR_AT_LINE + 1);

		cBufferEditHandleCursorMovementWithArrowsHandleXAxis(
			_WANTED_LINE);
		_CURSOR_AT_LINE = _WANTED_LINE;
	}

	if (_ARROW_LEFT // Cursor left 
		== _KEYCODE)
		cBufferEditHandleCursorMovementWithArrowsHandleYAxis(_CURSOR_AT_CHAR - 1);

	if (_ARROW_RIGHT // Cursor right 
		== _KEYCODE)
		cBufferEditHandleCursorMovementWithArrowsHandleYAxis(_CURSOR_AT_CHAR + 1);

	// Refresh 
	_RENDER_CURSOR = true;
	dSetScrollCorespondingToCursorPositionY();
	dSetScrollCorespondingToCursorPositionX();
	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const void ProgramBuffer::dSetScrollCorespondingToCursorPositionY()
{
	const int _LINE_HEIGHT = g_BufferLineSizeY();
	const int _SCREEN_Y = g_BufferRenderBoundingBox().gSize().y;

	const int _LINE_POSITION_Y = ((_CURSOR_AT_LINE + 1) * _LINE_HEIGHT);
	const int _SCROLL_Y_ABSOLUTE = std::abs(_SCROLL_Y);

	/* Lines data */
	const int _LINES_VISIBLE_ON_SCREEN = static_cast<int>(
		_SCREEN_Y / _LINE_HEIGHT);
	
	const int _LINES_VISIBLE_FROM = static_cast<int>(
		(const double)_SCROLL_Y_ABSOLUTE / (const double)_LINE_HEIGHT);
	
	/* Check if needed to perform any actions */
	if (Framework::Geometry::cBound1D(
		_CURSOR_AT_LINE, _LINES_VISIBLE_FROM, (_LINES_VISIBLE_ON_SCREEN - 1)
	) == true) return; 

	const int _CURSOR_POSITION_IN_SCROLL = (
		((_CURSOR_AT_LINE + 1) - _LINES_VISIBLE_ON_SCREEN) * _LINE_HEIGHT
		);

	long double _SCROLL_WANTED = 0; ((long double)_CURSOR_POSITION_IN_SCROLL / (long double)g_MaxScrolls().y);
	if (_CURSOR_AT_LINE >
		_LINES_VISIBLE_FROM)
	{ /* Scroll needs to go up */
		_SCROLL_WANTED = (
			(long double)_CURSOR_POSITION_IN_SCROLL / (long double)g_MaxScrolls().y
			);
	}

	if (_CURSOR_AT_LINE <
		_LINES_VISIBLE_FROM)
	{ /* Scroll needs to go down */
		_SCROLL_WANTED = (
			(long double)(_CURSOR_AT_LINE * _LINE_HEIGHT) / (long double)g_MaxScrolls().y
			);
	}

	_SLIDER_VERTICAL->sScrollProcentage(_SCROLL_WANTED);
}

const void ProgramBuffer::dSetScrollCorespondingToCursorPositionX()
{
	/* create canvas to obtain informations about text len */
	wxMemoryDC _CANVAS; _CANVAS.SetFont(_TEXT_FONT);
	
	/* essential variables */
	const int _CHAR_LEN = _CANVAS.GetTextExtent("a").x;
	const int _SCROLL_X_ABSOLUTE = std::abs(_SCROLL_X);
	const int _SCREEN_X = g_BufferRenderBoundingBox().gSize().x; 

	/* Chars data */
	const int _CHARS_VISIBLE_ON_SCREEN =
		(_SCREEN_X / _CHAR_LEN);

	const int _CHARS_VISIBLE_FROM =
		(_SCROLL_X_ABSOLUTE / _CHAR_LEN);

	/* Check if needed to perform any actions */
	if (Framework::Geometry::cBound1D(
		_CURSOR_AT_CHAR, _CHARS_VISIBLE_FROM, (_CHARS_VISIBLE_ON_SCREEN - 1)
	) == true) return;
	long double _SCROLL_WANTED = 0;

	if (_CHARS_VISIBLE_FROM <
		_CURSOR_AT_CHAR) { /* Scroll up */
		_SCROLL_WANTED = (
			(_CURSOR_AT_CHAR - _CHARS_VISIBLE_ON_SCREEN + 2) * _CHAR_LEN
			);
	}

	if (_CHARS_VISIBLE_FROM >
		_CURSOR_AT_CHAR) { /* Scroll down */
		_SCROLL_WANTED = (
			(_CURSOR_AT_CHAR - 2) * _CHAR_LEN
			);
	}
	/* Modify scroll offset */
	_SLIDER_HORIZONTAL->sScrollProcentage(
		(_SCROLL_WANTED / (long double)g_MaxScrolls().x));
}

const void ProgramBuffer::cBufferEditHandleCursorMovementWithArrowsHandleXAxis(
	const int& _WANTED_LINE
) {
	_CURSOR_AT_CHAR = std::min(_CURSOR_AT_CHAR, (int)_DATA[_WANTED_LINE]._TEXT.length());
};

const void ProgramBuffer::cBufferEditHandleCursorMovementWithArrowsHandleYAxis(
	const int& _WANTED_CHAR
) {
	if (_WANTED_CHAR ==
		-1)
	{ /* Back to old line */
		const int _LINE_BEFORE = (_CURSOR_AT_LINE - 1);
		if (_LINE_BEFORE /* Avoid buffer crashing on invaild line */
			== -1) return; 
		/* Perform action */
		const int _LINE_BEFORE_LEN = (_DATA[_LINE_BEFORE]._TEXT.length());

		_CURSOR_AT_CHAR = _LINE_BEFORE_LEN; 
		_CURSOR_AT_LINE = _LINE_BEFORE;
		return; 
	} 
	_CURSOR_AT_CHAR = std::min(
		(int)_DATA[_CURSOR_AT_LINE]._TEXT.length(), _WANTED_CHAR
		); /* Limit to avoid cursor crash */
}

const void ProgramBuffer::dSelectionExit()
{
	_SELECTION_STARTED_AT_CHAR = -1; 
	_SELECTION_STARTED_AT_LINE = -1;
	_SELECTION_ENDED_AT_CHAR = -1; 
	_SELECTION_ENDED_AT_LINE = -1;

	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

void ProgramBuffer::hMouseDClick(
	wxMouseEvent& _EVT
) {
	try { mHandleSelectionAbort(); } catch (const std::exception& ex) { _DEBUG->Push(ex.what()); }
}

const void ProgramBuffer::mHandleSelectionAbort()
{ dSelectionExit(); }

const void ProgramBuffer::cHandleSelectionCopyToClipboard()
{
	if (_SELECTION_STARTED_AT_CHAR == -1 /* Check if selected */
		|| _SELECTION_ENDED_AT_CHAR == -1
		|| _SELECTION_ENDED_AT_LINE == -1
		|| _SELECTION_STARTED_AT_LINE == -1) return;
	try {
		/* Obtain data to copy */
		std::string _DATA_TO_COPY; 

		/* Handle lines switch */
		int _STARTED = _SELECTION_STARTED_AT_LINE;
		int _ENDED = _SELECTION_ENDED_AT_LINE; 
		int _CHAR_STARTED = _SELECTION_STARTED_AT_CHAR;
		int _CHAR_ENDED = _SELECTION_ENDED_AT_CHAR;

		if (_STARTED > /* Switch check */
			_ENDED) {
			/* Apply  switch */
			_STARTED = _ENDED;
			_ENDED = _SELECTION_STARTED_AT_LINE;
			_CHAR_STARTED = _CHAR_ENDED;
			_CHAR_ENDED = _SELECTION_STARTED_AT_CHAR;
		}
		/* First string */
		_DATA_TO_COPY = (_DATA[_STARTED]._TEXT.substr(_CHAR_STARTED) + "\n");

		/* Middle */
		for (int _ITERATOR = (_STARTED + 1); _ITERATOR < _ENDED;
			_ITERATOR++) { /* Collect */
			_DATA_TO_COPY += (_DATA[_ITERATOR]._TEXT + "\n");
		}

		/* Last string */
		_DATA_TO_COPY += (_DATA[_ENDED]._TEXT.substr(0, (
			_CHAR_ENDED + 1)));

		/* Finalize */
		Framework::System::Clipboard::Set(_DATA_TO_COPY);
	} catch (const std::exception& ex) { _DEBUG->Push("SELECTION_COPY_FAILED"); }
}

const void ProgramBuffer::lCursorBlinkHook()
{
	_CURSOR_BLIK_TIMER.SetOwner(this);
	_CURSOR_BLIK_TIMER.Start(650);
	Bind(wxEVT_TIMER, &ProgramBuffer::hCursorBlink, this, _CURSOR_BLIK_TIMER.GetId());
}

void ProgramBuffer::hCursorBlink(
	wxTimerEvent& _EVT) 
{ dCursorBlink(); }

const void ProgramBuffer::dCursorBlink()
{ _RENDER_CURSOR = not _RENDER_CURSOR; fSmartRefresh(g_BufferRenderBoundingBox().gRect()); }

const void ProgramBuffer::cBufferEditSelectionDestroy()
{
	if (_SELECTION_STARTED_AT_CHAR == -1 /* Check if selected */
		|| _SELECTION_ENDED_AT_CHAR == -1
		|| _SELECTION_ENDED_AT_LINE == -1
		|| _SELECTION_STARTED_AT_LINE == -1) return;

	/* Handle lines switch */
	int _STARTED = _SELECTION_STARTED_AT_LINE;
	int _ENDED = _SELECTION_ENDED_AT_LINE;
	int _CHAR_STARTED = _SELECTION_STARTED_AT_CHAR;
	int _CHAR_ENDED = _SELECTION_ENDED_AT_CHAR;

	if (_STARTED > /* Switch check */
		_ENDED) {
		/* Apply  switch */
		_STARTED = _ENDED;
		_ENDED = _SELECTION_STARTED_AT_LINE;
		_CHAR_STARTED = _CHAR_ENDED;
		_CHAR_ENDED = _SELECTION_STARTED_AT_CHAR;
	}

	/* Collect lines to override */
	const std::string _FIRST_LINE_LEFT = _DATA[_STARTED]._TEXT.substr(0, _CHAR_STARTED);
	
	std::string _LAST_LINE_LEFT =
		FRAMEWORK_EMPTY_STRING;

	const std::string _LAST_LINE = _DATA[_ENDED]._TEXT; 

	if (_LAST_LINE.empty() !=
		true)
	{ /* Avoid invaild position on std::substr */
		_LAST_LINE_LEFT = _LAST_LINE.substr(_CHAR_ENDED + 1);
	}

	const std::string _LINES_SUM = (_FIRST_LINE_LEFT + _LAST_LINE_LEFT);
	/* Override first line */
	_DATA[_STARTED]._TEXT = _LINES_SUM; 
	_DATA[_STARTED]._TEXT_RAW = _DATA[_STARTED]._TEXT;
	
	/* Delete rest of the lines */
	/* If erased needs rehash */

	for (size_t _AT = (_STARTED + 1); _AT <=
		_ENDED; _AT++)
	{ /* Ranging from <start + 1, end> */
		const auto _ERASE_AT = (
			_DATA.begin() + (_STARTED + 1)
			); 
		if ((_DATA.begin() + _DATA.size()) <= _ERASE_AT) break; 

		_DATA.erase(_ERASE_AT);
	}
	/* Handle cursor placement */
	_CURSOR_AT_LINE = _STARTED; 
	_CURSOR_AT_CHAR = _CHAR_STARTED;

	/* Handle other buffer hooks */
	dSelectionExit(); 
	dSetScrollCorespondingToCursorPositionX();
	dSetScrollCorespondingToCursorPositionY();
	fSmartRefresh(g_BufferRenderFullBoundingBox().gRect());
}

const bool ProgramBuffer::gBufferDoesNeedUpdate()
{ /* Disable if checked to avoid memory leaks */
	if (_BUFFER_UPDATED == false) return false; 
	_BUFFER_UPDATED = false; return true; 
}

std::vector<ProgramBuffer::BufferElement>& ProgramBuffer::gBufferData() 
{ return _DATA; }

const void ProgramBuffer::mLine(
	const size_t& At, const std::string& Line
) {
	/* Out of array bounds */
	if (_DATA.size() <= At) return; 
	_DATA[At] = ProgramBuffer::BufferElement{ Line, Line };
}

/* [ =================================== MODERN CODE =================================== ] */

	/* [ =================================== _SmartRefresh =================================== ] */

	void ProgramBuffer::fSmartRefresh(
		const wxRect& _Area
	) {
		int x, y, sx, sy;
		x = _Area.x; 
		y = _Area.y; 
		sx = _Area.width;
		sy = _Area.height;
	
		int _x, _y, _sx, _sy; 
		_x = _SmartRefreshArea.x; 
		_y = _SmartRefreshArea.y; 
		_sx = _SmartRefreshArea.width; 
		_sy = _SmartRefreshArea.height; 

		int _fx, _fy; 
		_fx = (_x + _sx);
		_fy = (_y + _sy);

		int fx, fy; 
		fx = (x + sx);
		fy = (y + sy);

		_x = std::min(_x, x);
		_y = std::min(_y, y);
		_fx = std::max(_fx, fx);
		_fy = std::max(_fy, fy);

		/* Update values */
		_SmartRefreshArea = Framework::Geometry::BoundingBox(
			wxPoint(_x, _y), wxPoint(_fx, _fy)
		).gRect(); 

		_SmartRefreshForce = true; 
	}

	void ProgramBuffer::l_SmartRefresh()
	{ /* _SmartRefresh loader */
		_SmartRefreshTimer.SetOwner(this);
		_SmartRefreshTimer.Start(_SmartRefreshClock);
		Bind(wxEVT_TIMER, &ProgramBuffer::hk_SmartRefresh, this, _SmartRefreshTimer.GetId());
	}

	void ProgramBuffer::hk_SmartRefresh(
		wxTimerEvent& _Event)
	{ /* _SmartRefresh main hook */
		try { hdl_SmartRefreshUpdate(); } catch (const std::exception& _ex)
		{ /* Log in case of an error */
			_DEBUG->log_TaggedStdException("hk_SmartRefresh", _ex); 
		}
	}

	void ProgramBuffer::hdl_SmartRefreshUpdate()
	{ /* Update given rect if conditions are met */
		if (_SmartRefreshForce ==
			false) return;
		/* Force Before Update Event */
		hdl_SmartRefreshEvent__SmartRefreshBeforeUpdate();

		/* Update */
		RefreshRect(_SmartRefreshArea);

		_SmartRefreshArea = wxRect();
		_SmartRefreshForce = false; 
	}

	void ProgramBuffer::hdl_SmartRefreshEvent__SmartRefreshBeforeUpdate()
	{ /* Triggers special function event */
		wxCommandEvent _SmartRefreshBeforeUpdateInstance(_SmartRefreshBeforeUpdate);
		wxPostEvent(this, _SmartRefreshBeforeUpdateInstance);
	}

	/* [ =================================== _SmartSyntax =================================== ] */
	void ProgramBuffer::l_SmartSyntax()
	{ /* _SmartSyntax loader */
		_SmartSyntaxEnabled = true; 

		_SmartSyntaxKeywords = {
			"int", "float", "double", "char", "if", "else", "for", "while", "return",
			"class", "struct", "public", "private", "protected", "virtual", "override",
			"new", "delete", "const", "void", "static", "namespace", "using", "std"
		};

		_SmartSyntaxColors = {
			{"_syntax:keyword", "\\a0080FF"},      // Keywords in red
			{"_syntax:string", "\\a00FF00"},     // Strings in green
			{"_syntax:comment", "\\a00FFFF"},     // Comments in cyan
			{"_syntax:function", "\\aFFFF00"}
		};
		Bind(_SmartRefreshBeforeUpdate, &ProgramBuffer::hk_SmartSyntax_SmartRefreshBeforeUpdate, this);
	}

	void ProgramBuffer::hdl_SmartSyntaxApply()
	{
		/* Go throught data and swap it */
		for (ProgramBuffer::BufferElement& _Element :
			_DATA)
		{ /* Swap */
			_Element._TEXT_RAW = _Element._TEXT;
			/* Comments */
			_Element._TEXT_RAW = hdl_SmartSyntaxHighlightComments(
				_Element._TEXT_RAW);
			/* Literals */
			_Element._TEXT_RAW = hdl_SmartSyntaxHighlightLiterals(
				_Element._TEXT_RAW);
			/* Keywords */
			_Element._TEXT_RAW = hdl_SmartSyntaxHighlightKeywords(
				_Element._TEXT_RAW);
			/* Functions */
			_Element._TEXT_RAW = hdl_SmartSyntaxHighlightFunctions(
				_Element._TEXT_RAW);
		}
	}

	void ProgramBuffer::hk_SmartSyntax_SmartRefreshBeforeUpdate(
		wxCommandEvent& _Event) 
	{ /* _SmartSyntax main update hook */
		if (_SmartSyntaxEnabled
			== false) return; 
		try {
			hdl_SmartSyntaxApply(); 
			/* Log exception<tagged> if happens */
		} catch (const std::exception& _ex) { _DEBUG->log_TaggedStdException(
			"hk_SmartSyntax_SmartRefreshBeforeUpdate", _ex); }
	}

	const std::string ProgramBuffer::hdl_SmartSyntaxHighlightComments(
		const std::string& _Line) const 
	{ 
		std::regex _CommentRegrex("//.*");
		return std::regex_replace(
			_Line, _CommentRegrex, _SmartSyntaxColors.at("_syntax:comment") + "$&" + _DefaultColorEscape
		);
	}

	const std::string ProgramBuffer::hdl_SmartSyntaxHighlightKeywords(
		const std::string& _Line) const
	{ 
		std::string _Result = _Line;
		for (const std::string& _Keyword :
			_SmartSyntaxKeywords)
		{ /* Iterate throught and if found replace */
			std::regex _KeywordRegex((
				"\\b" + _Keyword + "\\b"));
			/* Replace regex */
			_Result = std::regex_replace(_Result, _KeywordRegex, (
				_SmartSyntaxColors.at("_syntax:keyword") + _Keyword + _DefaultColorEscape));
		} return _Result; 
	}

	const std::string ProgramBuffer::hdl_SmartSyntaxHighlightLiterals(
		const std::string& _Line) const
	{
		std::regex _CommentRegrex("\"([^\"]*)\"");
		return std::regex_replace(
			_Line, _CommentRegrex, _SmartSyntaxColors.at("_syntax:string") + "$&" + _DefaultColorEscape
		);
	}

	const std::string ProgramBuffer::hdl_SmartSyntaxHighlightFunctions(
		const std::string& _Line) const
	{
		/* To do */
		return _Line;
	}

