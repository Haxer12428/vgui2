/* [=============================== INCLUDES ===============================] */
#include "TextEditor.h"

/* [=============================== Initializer ===============================] */
	TextEditor::TextEditor(
		_CoreList& cr_List
	) : cr_List{ cr_List }, wxPanel( 
			cr_List._Parent, wxID_ANY, cr_List._Position, cr_List._Size
		){
		try 
		{ /* Initializers */
			init_hk_Render();
			init_Scrollbars();
			init_MouseWheelGlobalActions();
			init_Cursor();
			init_OnChar();
			init_Info();
			init_MouseMotionHook(); 
            init_SmartRefresh();
			init_OnExit();
		} catch (const std::exception& /* Pass the exception*/
			_exc) { throw _exc; }
	}

	TextEditor::~TextEditor()
	{
		cr_List._Buffer = {};
	}

/* [=============================== Layout Positions ===============================] */
	TextEditor::_LayoutPositions TextEditor::g_LayoutPositions()
	{ /* Getting layout positions and returning them */
		TextEditor::_LayoutPositions _Positions;

		const wxSize _GlobalSize = GetSize();

		/* Info */
		wxMemoryDC _InfoMemoryCanvas;
		_InfoMemoryCanvas.SetFont(cr_List._LayoutInfoFont);

		const int _InfoSY = (cr_List._LayoutInfoSpacingY
			+ _InfoMemoryCanvas.GetTextExtent("a").y);

		_Positions._InfoBB[0] = 0;
		_Positions._InfoBB[1] = GetSize().y - _InfoSY;
		_Positions._InfoBB[2] = GetSize().x;
		_Positions._InfoBB[3] = GetSize().y;

		/* _LineIndexing */
		/* Calculate */
		wxMemoryDC _LineIndexingMemoryCanvas;
		_LineIndexingMemoryCanvas.SetFont(cr_List._LayoutLinesIndexingFont);

		const int _LongestFontLen = (_LineIndexingMemoryCanvas.GetTextExtent(
			std::to_string(
				cr_List._Buffer.size()
			)
		).x);

		const int _LineIndexingSX = (
			cr_List._LayoutLineIndexingSpacingXStart + _LongestFontLen + cr_List._LayoutLineIndexingSpacingXEnd
			);

		/* x, y, fx, fy */
		_Positions._LineIndexingBB[0] = 0; 
		_Positions._LineIndexingBB[1] = 0;
		_Positions._LineIndexingBB[2] = (_Positions._LineIndexingBB[0] + _LineIndexingSX);
		_Positions._LineIndexingBB[3] = (_Positions._InfoBB[1]);


		/* Scrollbars */
			/* Vertical Scrollbar */
			_Positions._VerticalScrollbarBB[0] = (_GlobalSize.x - cr_List._LayoutScrollbarsAxisLength);
			_Positions._VerticalScrollbarBB[1] = 0;

			_Positions._VerticalScrollbarBB[2] = _Positions._VerticalScrollbarBB[0]
				+ cr_List._LayoutScrollbarsAxisLength;

			_Positions._VerticalScrollbarBB[3] = _Positions._VerticalScrollbarBB[1] 
				+ (_Positions._InfoBB[1] - cr_List._LayoutScrollbarsAxisLength);

			/* Horizontal Scrollbar*/
			_Positions._HorizontalScrollbarBB[0] = (_Positions._LineIndexingBB[2]);
			_Positions._HorizontalScrollbarBB[1] = (_Positions._InfoBB[1] - cr_List._LayoutScrollbarsAxisLength);

			_Positions._HorizontalScrollbarBB[2] = _Positions._HorizontalScrollbarBB[0] +
				((_GlobalSize.x - _Positions._LineIndexingBB[2]) - cr_List._LayoutScrollbarsAxisLength);

			_Positions._HorizontalScrollbarBB[3] = _Positions._HorizontalScrollbarBB[1] +
				(cr_List._LayoutScrollbarsAxisLength);

		/* _Buffer */
		const int _BufferSX = (
			_GlobalSize.x - (_Positions._LineIndexingBB[2] + _Positions._VerticalScrollbarBB[2] 
				- _Positions._LineIndexingBB[0] - _Positions._VerticalScrollbarBB[0])
			);
		const int _BufferSY = (
			_Positions._InfoBB[1] - (_Positions._HorizontalScrollbarBB[3] - _Positions._HorizontalScrollbarBB[1])
			);
		_Positions._BufferBB[0] = _Positions._LineIndexingBB[2]; 
		_Positions._BufferBB[1] = 0; 
		_Positions._BufferBB[2] = (_Positions._BufferBB[0] + _BufferSX);
		_Positions._BufferBB[3] = (_Positions._BufferBB[1] + _BufferSY);

		/* Return final */
		return _Positions;
	}
/* [=============================== _RenderHook ===============================] */
	void TextEditor::init_hk_Render()
	{
		SetBackgroundStyle(wxBG_STYLE_CUSTOM);
		Bind(wxEVT_PAINT, &TextEditor::hk_Render, this);
	}

	void TextEditor::hk_Render(
		wxPaintEvent& _event)
	{ /* Render hook operating on wxAutoBufferedDC */
		try {
			wxAutoBufferedPaintDC _Canvas{ this };

			/* Rendering calls */
			hk_BackgroundRender(_Canvas);
			hk_BufferRender(_Canvas);
			hk_LineIndexingRender(_Canvas);
			hk_CursorRender(_Canvas);
			hk_InfoRender(_Canvas);
		} catch (const std::exception& ex) { cr_List._Debug->log_TaggedStdException(
			__FUNCTION__, ex
		); }
	}
/* [=============================== _Background ===============================] */
	void TextEditor::hk_BackgroundRender(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render main bg for this panel*/
		_Canvas.SetPen(cr_List._LayoutBackgroundColor);
		_Canvas.SetBrush(cr_List._LayoutBackgroundColor);
		/* Draw */
		_Canvas.DrawRectangle(GetRect());
	}
/* [=============================== _LineIndexing ===============================] */
	void TextEditor::hk_LineIndexingRender(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render hook for _LineIndexing */
		hdl_LineIndexingSetupClippingRegion(_Canvas);

		hk_LineIndexingRenderBackground(_Canvas);
		hk_LineIndexingRenderNumbers(_Canvas);

		_Canvas.DestroyClippingRegion();
	}

	void TextEditor::hdl_LineIndexingSetupClippingRegion(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Setup clipping rect */
		_LayoutPositions _Layout = g_LayoutPositions();

		_Canvas.SetClippingRegion(
			_Layout.g_BBFromPackedArray(_Layout._LineIndexingBB).gRect()
		);
	}

	void TextEditor::hk_LineIndexingRenderBackground(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Draw bg */
		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _BackgroundBB = _Layout.g_BBFromPackedArray(
			_Layout._LineIndexingBB);
		/* Draw Static Background */
			_Canvas.SetPen(cr_List._LayoutLineIndexingBackgroundRegularColor);
			_Canvas.SetBrush(cr_List._LayoutLineIndexingBackgroundRegularColor);

			_Canvas.DrawRectangle(_BackgroundBB.gRect());
		/* Draw Dynamic Background */
			const std::vector<size_t>& _ActivatedLines = cr_List._LineIndexingActivatedLines;

			if (_ActivatedLines.size() == 0 || /* Check if it should include background */
					cr_List._LineIndexingActivatedLinesIncludeBG == false
				) return;

			_Canvas.SetPen(cr_List._LayoutLineIndexingBackgroundSelectionColor);
			_Canvas.SetBrush(cr_List._LayoutLineIndexingBackgroundSelectionColor);

			for (const size_t& _Element : 
				_ActivatedLines)
			{ /* Draw rectangles */
				const wxRect _CurrentRectangle = g_LineIndexingGivenAtBB(
					_Element).gRect();

				/* Check for compatibility*/
				if (_BackgroundBB.gVisibility(
					_CurrentRectangle
				) == true)
				{ /* Draw if visible */
					_Canvas.DrawRectangle(
						_CurrentRectangle
					);
				}
			}
	}

	void TextEditor::hk_LineIndexingRenderNumbers(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render numbers */
		_Canvas.SetFont(cr_List._LayoutLinesIndexingFont);

		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _LineIndexingBB = _Layout.g_BBFromPackedArray(
			_Layout._LineIndexingBB);
		/* Iterate and check what to draw and with what color */
		const std::array<size_t, 2> _IterationRange = g_LineIndexingRenderingRange();

		/* Check for font size if its equal to smallest abort drawing numbers */
		const int _SmallestFontSize = 1; 
		const int _CurrentFontSize = _Canvas.GetFont().GetPointSize(); 

		if (_CurrentFontSize
			== _SmallestFontSize) return;

		for (size_t _Number = _IterationRange[0]; _Number <=
			_IterationRange[1]; _Number++)
		{ 
			/* Determinate if render */
			const wxRect _CurrentRectangle = g_LineIndexingGivenAtBB(
				_Number).gRect();

			wxColor _Clr = cr_List._LayoutLineIndexingFontColorDeActivated;

			for (const size_t& _Selected : /* <= might cause bad performance */
				cr_List._LineIndexingActivatedLines)
			{ /* Check if selected */
				if (_Selected ==
					_Number)
				{
					_Clr = cr_List._LayoutLineIndexingFontColorActivated; break;
				}
			}
			/* Render */
			std::string _NumberStr = std::to_string((
				_Number + 1)); /* Fixed number ranging from 1-size() */

			const wxPoint _NumberPositionCentered = g_LineIndexingNumberCenteredPosition(
				_CurrentRectangle.GetBottomRight(), _NumberStr);

			_Canvas.SetTextBackground(_Clr);
			_Canvas.SetTextForeground(_Clr);
			_Canvas.DrawText(_NumberStr, _NumberPositionCentered);
		}
	}

	Framework::Geometry::BoundingBox TextEditor::g_LineIndexingGivenAtBB(
		const size_t& _Where) 
	{ /* Calc & return */
		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutLinesIndexingFont);

		_LayoutPositions _Layout = g_LayoutPositions();
		Framework::Geometry::BoundingBox _LineIndexingBB = _Layout.g_BBFromPackedArray(_Layout._LineIndexingBB);
		/* Calculate */
			const int _LineSY = (_MemoryCanvas.GetTextExtent("Rg").y +
				cr_List.m_LineIndexingSpacingY);
			const int _LineSX = (_LineIndexingBB.gSize().x);

			int _x, _y, _fx, _fy;
			
			_x = _LineIndexingBB.gStarting().x; 
			_y = (_LineSY * _Where);
			_y -= cr_List._BufferScroll[1];
			_fx = (_x + _LineSX);
			_fy = (_y + _LineSY);
		/* Return */
		return { wxPoint(_x, _y), wxPoint(_fx, _fy) };
	}

	const wxPoint TextEditor::g_LineIndexingNumberCenteredPosition(
		const wxPoint& _Final, const std::string& _Number) const 
	{ /* Calculate & return */
		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutLinesIndexingFont);

		/* Calculate */
			int _x, _y; 
			_x = _Final.x;
			_y = _Final.y;
			_x -= cr_List._LayoutLineIndexingSpacingXEnd;
			_y -= (_MemoryCanvas.GetTextExtent("Rg").y
				+ cr_List.m_LineIndexingSpacingY / 2);
			_x -= _MemoryCanvas.GetTextExtent(_Number).x; 
		/* Return*/
		return wxPoint{_x, _y + 1};
	}

	const std::array<size_t, 2> TextEditor::g_LineIndexingRenderingRange()
	{ /* Calculate & return */
		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutLinesIndexingFont);
		
		_LayoutPositions _Layout = g_LayoutPositions();
		/* Calculate */
			size_t _r1, _r2;

			const int _LineSizeY = (_MemoryCanvas.GetTextExtent("Rg").y
				+ cr_List.m_LineIndexingSpacingY);
			
			const int _MaximalVisibleLines = (_Layout.g_BBFromPackedArray(
				_Layout._LineIndexingBB).gSize().y / _LineSizeY );

			_r1 = (cr_List._BufferScroll[1] / _LineSizeY);
			_r2 = (_r1 + _MaximalVisibleLines);

			/* Avoid cutting lines */
			_r2++;
			/* Avoid overflow */
			_r1 = std::max(_r1, (size_t)0);
			_r2 = std::min(_r2, (cr_List._Buffer.size() - 1));
		/* Return */
		return {_r1, _r2};
	}
/* [=============================== _Scrollbars ===============================] */
	void TextEditor::init_Scrollbars()
	{ /* Initalizer for _Scrollbars */
		hdl_ScrollbarsInstanceSetup();
		hdl_ScrollbarsSetupCheckHK();
	}

	void TextEditor::hdl_ScrollbarsInstanceSetup()
	{ /* Initializer for Horizontal Scrollbar*/
		_LayoutPositions _Layout = g_LayoutPositions();

		/* Horizontal setup */
			Framework::Geometry::BoundingBox _HorizontalScrollbarLayout = _Layout.g_BBFromPackedArray(
				_Layout._HorizontalScrollbarBB);

			m_ScrollbarsHorizontal = new ProgramSlider(
				this, cr_List._Debug,
				_HorizontalScrollbarLayout.gSize(), _HorizontalScrollbarLayout.gStarting(),
				cr_List._LayoutScrollbarsBackgroundColor, cr_List._LayoutScrollbarsElementColorDeActivated,
				0.5, 0.6, ProgramSlider::Horizontal
			);
		/* Vertical setup */
			Framework::Geometry::BoundingBox _VerticalScrollbarLayout = _Layout.g_BBFromPackedArray(
				_Layout._VerticalScrollbarBB);

			m_ScrollbarsVertical = new ProgramSlider(
				this, cr_List._Debug,
				_VerticalScrollbarLayout.gSize(), _VerticalScrollbarLayout.gStarting(),
				cr_List._LayoutScrollbarsBackgroundColor, cr_List._LayoutScrollbarsElementColorDeActivated,
				0.5, 0.6, ProgramSlider::Vertical
			);
	}

	void TextEditor::hdl_ScrollbarsVisibility()
	{
		const std::array<int, 2> _MaximalScroll = g_BufferMaximalScroll();

		bool _VerticalVisible = (_MaximalScroll[1] > 0);
		bool _HorizontalVisible = (_MaximalScroll[0] > 0);

		m_ScrollbarsVertical->Show(_VerticalVisible);
		m_ScrollbarsHorizontal->Show(_HorizontalVisible);
	}

	void TextEditor::hdl_ScrollbarsUpdatePositions()
	{ /* Update positions & sizes of scrollbas */
		_LayoutPositions _Layout = g_LayoutPositions(); 
		/* Get layouts */
		Framework::Geometry::BoundingBox _HorizontalLayout =
			_Layout.g_BBFromPackedArray(_Layout._HorizontalScrollbarBB);

		Framework::Geometry::BoundingBox _VerticalLayout =
			_Layout.g_BBFromPackedArray(_Layout._VerticalScrollbarBB);
		/* Get Values */
		const wxPoint _VerticalPosition = _VerticalLayout.gStarting();
		const wxSize _VerticalSize = _VerticalLayout.gSize(); 
		
		const wxPoint _HorizontalPosition = _HorizontalLayout.gStarting();
		const wxSize _HorizontalSize = _HorizontalLayout.gSize();
		/* Set */
		m_ScrollbarsVertical->s_Size(_VerticalSize);
		m_ScrollbarsHorizontal->s_Size(_HorizontalSize);

		m_ScrollbarsVertical->s_Position(_VerticalPosition);
		m_ScrollbarsHorizontal->s_Position(_HorizontalPosition);
	}


	void TextEditor::hdl_ScrollbarsSetupCheckHK()
	{ /* Setup Scrollbars hk_ScrollbarsCheck*/
		m_ScrollbarsCheckTimer.SetOwner(this);
		m_ScrollbarsCheckTimer.Start(cr_List._ScrollbarsCheckTimerMs);
		/* Bind handler to timer id */
		Bind(wxEVT_TIMER, &TextEditor::hk_ScrollbarsCheck, this, m_ScrollbarsCheckTimer.GetId());
	}

	void TextEditor::hk_ScrollbarsCheck(
		wxTimerEvent& _Event)
	{ /* Scrollbar -> _Buffer | Scroll update &* Visibility check */
		/* Visiblity handler */
		hdl_ScrollbarsVisibility(); 
		/* Scroll update */
		double _x, _y; 
		/* Obtain raw */
		_x = m_ScrollbarsHorizontal->gScrollProcentage();
		_y = m_ScrollbarsVertical->gScrollProcentage();

		/* Multiply to obtain real value */
		const std::array<int, 2> _MaxScrolls = g_BufferMaximalScroll();
		_x *= _MaxScrolls[0];
		_y *= _MaxScrolls[1];

		/* Apply to buffer */
		cr_List._BufferScroll[0] = _x; 
		cr_List._BufferScroll[1] = _y; 

        p_SmartRefreshObjects({ m_Global });
	}
/* [=============================== _Buffer ===============================] */
	void TextEditor::hk_BufferRender(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render hook for _Buffer */
		hdl_BufferSetupClippingRegion(_Canvas);

		hk_BufferRenderBackground(_Canvas);
		hk_BufferRenderText(_Canvas);

		_Canvas.DestroyClippingRegion();
	}

	void TextEditor::hdl_BufferSetupClippingRegion(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Setup clipping region */
		_LayoutPositions _Layout = g_LayoutPositions();

		_Canvas.SetClippingRegion(
			_Layout.g_BBFromPackedArray(_Layout._BufferBB).gRect()
		);
	}

	void TextEditor::hk_BufferRenderBackground(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Draw background */
		_LayoutPositions _Layout = g_LayoutPositions();
		/* Draw  */
		_Canvas.SetPen(cr_List._LayoutBufferBackgroundColor);
		_Canvas.SetBrush(cr_List._LayoutBufferBackgroundColor);
		
		_Canvas.DrawRectangle(_Layout.g_BBFromPackedArray(_Layout._BufferBB).gRect());

		/* Draw Activated Background */
		if (cr_List._BufferActivatedLine
			== -1) return;
		
		_Canvas.SetPen(cr_List._LayoutBufferActivatedLineColor);
		_Canvas.SetBrush(cr_List._LayoutBufferActivatedLineColor);
		_Canvas.DrawRectangle(g_BufferLineBBAtGiven(cr_List._BufferActivatedLine).gRect());
	}

	void TextEditor::hdl_BufferRenderTextReal(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Rende real text */
		_Canvas.SetTextForeground(cr_List._LayoutBufferFontColor);

		/* Iterate throught the range & render */
		const std::array<size_t, 2> _RenderingRange = g_BufferRenderingRange();

		for (size_t _Iterator = _RenderingRange[0];
			_Iterator <= _RenderingRange[1]; _Iterator++)
		{ /* Prepare */
			const std::string& _Element = cr_List._Buffer[_Iterator];

			const wxPoint _Position = g_BufferTextCenteredPosition(
				_Iterator);
			/* Render */
			_Canvas.DrawText(
				_Element, _Position.x, _Position.y);
		}
	}

	void TextEditor::hdl_BufferRenderTextBlocks(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render blocks */
		const std::array<size_t, 2> _RenderingRange = g_BufferRenderingRange(); 

		/* Calculate chunk params */
		const size_t _RenderingRangeSize = (_RenderingRange[1] - _RenderingRange[0] + 1);
		const size_t _ChunksAmount = 256;

		const size_t _ChunkSize = std::ceil(static_cast<double>(
			_RenderingRangeSize) / _ChunksAmount);

		/* Get generated chunks */
		std::vector<std::array<size_t, 2>>
			_Chunks = g_BufferGeneratedChunks(
				_RenderingRange, _ChunksAmount, _ChunkSize
			);

		/* Render Chunks */
		hdl_BufferRenderTextBlocksRenderChunks(
			_Canvas, _Chunks, _ChunkSize
		);
	}

	void TextEditor::hdl_BufferRenderTextBlocksRenderChunks(
		wxAutoBufferedPaintDC& _Canvas, const std::vector<std::array<size_t, 2>>& _Chunks, const size_t& _ChunkSize)
	{ /* Chunks rendering */
		/* Setup canvas */
		_Canvas.SetPen(cr_List._LayoutBufferChunkOutlineColor);
		_Canvas.SetBrush(cr_List._LayoutBufferChunkColor);

		/* Get variables used to render chunks */
		const size_t _CharSize = g_BufferCharSizeX();
		const size_t _LineSize = g_BufferLineSizeY();

		/* Iterate & Render */
		for (const std::array<size_t, 2>&_Chunk : _Chunks)
		{
			/* Get Positions */
			const wxPoint _Starting = g_BufferLineBBAtGiven(_Chunk[0]).gStarting();

			int _x, _y, _fx, _fy;

			_x = _Starting.x;
			_y = _Starting.y;

			_fx = _x;
			_fy = _y;

			_fx += (_CharSize * _Chunk[1]);
			_fy += (_LineSize * _ChunkSize);

			/* Render */
			const wxRect _DrawRect = Framework::Geometry::BoundingBox(wxPoint{
				_x, _y
				}, wxPoint{ _fx, _fy }).gRect();

			_Canvas.DrawRectangle(_DrawRect);
		};

	}

	void TextEditor::hk_BufferRenderText(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render Text */
		/* Setup */
		_Canvas.SetFont(cr_List._LayoutBufferFont);

		/* Check whenever to render blocks or real text*/
		const int _MinimalFontSize = 1; 
		const int _CurrentFontSize = _Canvas.GetFont().GetPointSize();

		if (_MinimalFontSize ==
			_CurrentFontSize)
		{ /* Render blocks */
			hdl_BufferRenderTextBlocks(
				_Canvas);
			return; 
		} /* Render real text */
		hdl_BufferRenderTextReal(
			_Canvas);
	}

	const std::vector<std::array<size_t, 2>> TextEditor::g_BufferGeneratedChunks(
		const std::array<size_t, 2> _RenderingRange, const size_t& _ChunksAmount, const size_t& _ChunkSize)
	{ /* Generate & Return */
		std::vector<std::array<size_t, 2>> _Chunks;
		/* Generate */
		const std::vector<std::string>& _Buffer = cr_List._Buffer;
		const size_t _BufferMaximalIteration = (_Buffer.size() - 1);

		for (size_t _Iterator = 0;
			_Iterator < _ChunksAmount; _Iterator++)
		{ /* Generate singular chunk */
			const size_t _ChunkPosition = (
				_RenderingRange[0] - (_RenderingRange[0] % _ChunkSize) + (_ChunkSize * _Iterator)
				);

			/* Get Chunk Range */
			std::array<size_t, 2> _ChunkRange;

			_ChunkRange[0] = _ChunkPosition;
			_ChunkRange[1] = (_ChunkRange[0] + _ChunkSize);

			_ChunkRange[1] = std::min(_BufferMaximalIteration, _ChunkRange[1]);
			/* Get Chunk Lenght */
			const size_t _ChunkLongestText = g_BufferChunkLength(_ChunkRange);

			/* Push | position:size_x */
			_Chunks.push_back(
				std::array<size_t, 2>{_ChunkPosition, _ChunkLongestText}
			);
		}
		/* Return */
		return _Chunks; 
	}

	const size_t TextEditor::g_BufferChunkLength(
		const std::array<size_t, 2>& _Range)
	{ /* Calcualte & Return */
		size_t _ChunkLongestText = 0;

		for (size_t _BufferAt = _Range[0]; _BufferAt < _Range[1];
			_BufferAt++)
		{ /* Get Longest */
			const std::string& _CurrentString = 
				cr_List._Buffer[_BufferAt];

			size_t _CurrentLength = _CurrentString.length();
			/* Check if longer */
			if (_CurrentLength > _ChunkLongestText)
			{ /* Override */
				_ChunkLongestText = _CurrentLength;
			}
		} /* Return */
		return _ChunkLongestText;
	}

	const wxPoint TextEditor::g_BufferTextCenteredPosition(
		const size_t& _Where)
	{ /* Calculate & return */
		_LayoutPositions _Layout = g_LayoutPositions();
		
		Framework::Geometry::BoundingBox _BufferBB = _Layout.g_BBFromPackedArray(
			_Layout._BufferBB);
		const int _LineSizeY = g_BufferLineSizeY();

		/* Calculate */
			int _x, _y; 
			_x = _BufferBB.gStarting().x; 
			_y = _BufferBB.gStarting().y; 

			_x -= cr_List._BufferScroll[0];
			_y -= cr_List._BufferScroll[1];

			_y += ((_Where * _LineSizeY) + cr_List.m_BufferSpacingY / 2);
			/* Include scroll */
		/* Return */
		return wxPoint{_x, _y};
	}

	const size_t TextEditor::g_BufferCharSizeX() const
	{ /* Calculate & Return */
		wxMemoryDC _Canvas; 
		_Canvas.SetFont(cr_List._LayoutBufferFont);
		/* Return */
		return _Canvas.GetTextExtent("a").x;
	}

	const size_t TextEditor::g_BufferMaximalCharsVisible()
	{ /* Calculate & Return */
		_LayoutPositions _Layout = g_LayoutPositions();
		const int _BufferSizeX = _Layout.g_BBFromPackedArray(_Layout._BufferBB).gSize().x; 
		
		return (_BufferSizeX / g_BufferCharSizeX());
	}

	const size_t TextEditor::g_BufferMaximalLinesVisible()
	{ /* Calculate & Return */
		_LayoutPositions _Layout = g_LayoutPositions(); 
		const int _BufferSizeY = _Layout.g_BBFromPackedArray(
			_Layout._BufferBB).gSize().y; 
		/* Return */
		return (_BufferSizeY
			/ g_BufferLineSizeY());
	}

	const std::array<size_t, 2> TextEditor::g_BufferRenderingRange()
	{ /* Calculate & return */
		wxMemoryDC _MemoryCanvas;
		_MemoryCanvas.SetFont(cr_List._LayoutBufferFont);

		_LayoutPositions _Layout = g_LayoutPositions();
		/* Calculate */
		size_t _r1, _r2;

		const int _LineSizeY = (_MemoryCanvas.GetTextExtent("Rg").y
			+ cr_List.m_BufferSpacingY);

		const int _MaximalVisibleLines = (_Layout.g_BBFromPackedArray(
			_Layout._BufferBB).gSize().y / _LineSizeY);

		_r1 = (cr_List._BufferScroll[1] / _LineSizeY);
		_r2 = (_r1 + _MaximalVisibleLines);

		/* Avoid cutting lines */
		_r2++;
		/* Avoid overflow */
		_r1 = std::max(_r1, (size_t)0);
		_r2 = std::min(_r2, (cr_List._Buffer.size() - 1));
		/* Return */
		return { _r1, _r2 };
	}

	const std::array<int, 2> TextEditor::g_BufferMaximalScroll() 
	{ /* Calculate maximal scroll */
		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _BufferBB = _Layout.g_BBFromPackedArray(
			_Layout._BufferBB);

		const std::vector<std::string>& _BufferData = cr_List._Buffer;

		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutBufferFont);

		int _x, _y; 
		/* Calculate vertical */
			const int _BufferLineSizeY = g_BufferLineSizeY();
			
			const int _MaximalLinesVisible = g_BufferMaximalLinesVisible();
			
			const int _LinesInvisible = (_BufferData.size() - _MaximalLinesVisible);
			
			_y = (_LinesInvisible * _BufferLineSizeY);
		/* Calculate horizontal */
			const int _BufferCharSizeX = g_BufferCharSizeX();
			const int _MaximalCharsVisible = g_BufferMaximalCharsVisible();
			
			size_t _LongestX = 0; 
			for (const std::string& _Element :
				_BufferData)
			{ /* Iterate and find longest */
				if (_Element.length() > _LongestX)
					_LongestX = _Element.length();
			}
			/* Increment _x by one to make space for cursor placement */
			_x = (((_LongestX + 1) - _MaximalCharsVisible) 
				* _BufferCharSizeX);
		/* Limit */
		_x = std::max(0, _x);
		_y = std::max(0, _y);
		/* Return */
		return { _x, _y };
	}

	const int TextEditor::g_BufferLineSizeY() const
	{ /* Calculate & return */
		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutBufferFont);

		return (_MemoryCanvas.GetTextExtent("Rg").y + cr_List.m_BufferSpacingY);
	}

	Framework::Geometry::BoundingBox TextEditor::g_BufferLineBBAtGiven(
		const size_t& _Where) 
	{ /* Calculate & Return */
		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _BufferBB = _Layout.g_BBFromPackedArray(
			_Layout._BufferBB);

		const int _BufferLineY = g_BufferLineSizeY();
		/* Calculate */
			int _x, _y, _fx, _fy; 

			_x = _BufferBB.gStarting().x; 
			_y = _BufferBB.gStarting().y; 
			_fx = _BufferBB.gFinal().x; 

			_y -= cr_List._BufferScroll[1];
			_y += (_Where * (_BufferLineY));

			_fy = (_y + _BufferLineY);

		return { wxPoint {_x, _y}, wxPoint {_fx, _fy} };
	}

	Framework::Geometry::BoundingBox 
		TextEditor::g_BufferLineBBAtGivenLineAndChar(
			const size_t& _WhereLine, const size_t& _WhereChar
		)
	{ /* Calculate & Return */
		Framework::Geometry::BoundingBox _BufferLineBB = g_BufferLineBBAtGiven(
			_WhereLine);

		wxMemoryDC _Canvas;
		_Canvas.SetFont(cr_List._LayoutBufferFont);

		const std::string _Line = cr_List._Buffer[_WhereLine].substr(
			0, _WhereChar);
		/* Calculate */
			int _x, _y, _fx, _fy;
		
			_x = _BufferLineBB.gStarting().x;
			_y = _BufferLineBB.gStarting().y; 

			_x += _Canvas.GetTextExtent(_Line).x;
			
			_fx = _x;
			_fy = _y;

			_fx += _Canvas.GetTextExtent("a").x;
			_fy += _BufferLineBB.gSize().y;
		return { wxPoint { _x, _y }, wxPoint { _fx, _fy } };
	}

    const std::array<size_t, 2> TextEditor::g_BufferLineDataAtGivenPosition(
        const wxPoint& _Position)
    { /* Calculate & Return */
        /* Get _Buffer data */
        std::array<size_t, 2> _LineData = { 0, 0 };

        _LayoutPositions _Layout = g_LayoutPositions();

        Framework::Geometry::BoundingBox _BufferBB = _Layout.g_BBFromPackedArray(
            _Layout._BufferBB);
        const int _LineSize = g_BufferLineSizeY();
        const int _CharSize = g_BufferCharSizeX();

        const size_t _MinimalLine = 0;
        const size_t _MaximalLine = (cr_List._Buffer.size() - 1);

        const wxPoint _BufferBBStarting = _BufferBB.gStarting();

        /* Calculate main */
            int _posx, _posy;
            _posx = _Position.x;
            _posy = _Position.y;

            _posx -= _BufferBBStarting.x;
            _posy -= _BufferBBStarting.y; 

            _posx += cr_List._BufferScroll[0];
            _posy += cr_List._BufferScroll[1];

            _LineData[1] = (_posx / _CharSize);
            _LineData[0] = (_posy / _LineSize);

            /* Handle maximal and minimal values to avoid overflow */
            _LineData[0] = std::max(_MinimalLine, std::min(_MaximalLine, _LineData[0]));

            const std::string& _BufferLine = cr_List._Buffer[_LineData[0]];

            const size_t _MinimalChar = 0;
            const size_t _MaximalChar = _BufferLine.length();

            _LineData[1] = std::max(_MinimalChar, std::min(_MaximalChar, _LineData[1]));
        /* Return */
        return _LineData;
    }

	/* [=============================== _MouseWheelGlobalActions ===============================] */
	void TextEditor::init_MouseWheelGlobalActions()
	{ /* _MouseWheelGlobalActions initializer */
		Bind(wxEVT_MOUSEWHEEL, &TextEditor::hk_MouseWheelGlobalActionsMouseWheelEvent, this);
	}

	void TextEditor::hk_MouseWheelGlobalActionsMouseWheelEvent(
		wxMouseEvent& _Event)
	{ /* Main MouseWheel hook for this object*/
		try { /* Run 'hk' functions */
			hk_MouseWheelGlobalActionsVerticalScroll(
				_Event);
			hk_MouseWheelGlobalActionsResizeFonts(
				_Event);
		} catch (const std::exception& _ex) { cr_List._Debug->log_TaggedStdException(
			__FUNCTION__, _ex
		); }
	}

	void TextEditor::hk_MouseWheelGlobalActionsVerticalScroll(
		wxMouseEvent& _Event)
	{ /* Scroll Vertical Axis of the buffer */
		if (wxGetMouseState().m_controlDown
			== true) return; /* Check for ctrl */

		int _Rotation, _Delta; 

		/* Calculate */
		_Rotation = _Event.GetWheelRotation();
		_Delta = _Event.GetWheelDelta();

		int _Precentage = (static_cast<int>(_Rotation) / _Delta);
		int _MaximalScroll = g_BufferMaximalScroll()[1];
		int _ScrollPerLine = (_MaximalScroll / cr_List._Buffer.size());

		int _ScrollBy = (_ScrollPerLine * (_Precentage
			* cr_List._MouseWheelGlobalActionsVerticalScrollLinesPerWheelTick));

		double _ScrollByFixed = ((double)_ScrollBy / 
			(double)_MaximalScroll);
		/* Reverse _ScrollByFixed */
		_ScrollByFixed = -_ScrollByFixed;

		/* Apply */
		m_ScrollbarsVertical->sScrollProcentage(m_ScrollbarsVertical->gScrollProcentage()
			+ _ScrollByFixed);
	}

	void TextEditor::hk_MouseWheelGlobalActionsResizeFonts(
		wxMouseEvent& _Event)
	{ /* Resize layout fonts */
		if (wxGetMouseState().m_controlDown
			!= true) return; /* Check for ctrl */

		int _PointOfChange = (static_cast<int>(_Event.GetWheelRotation()) 
			/ _Event.GetWheelDelta());

		wxFont& _LayoutBufferFont = cr_List._LayoutBufferFont;
		wxFont& _LayoutLinesIndexingFont = cr_List._LayoutLinesIndexingFont;

		const int _LayoutBufferFontSize = _LayoutBufferFont.GetPointSize();
		const int _LayoutLinesIndexingFontSize = _LayoutLinesIndexingFont.GetPointSize();
		/* Apply */
		const int _LayoutLinesIndexingFontWantedSize = std::max(1,
			(_LayoutLinesIndexingFontSize + _PointOfChange));
		const int _LayoutBufferFontSizeWantedSize = std::max(1,
			(_LayoutBufferFontSize + _PointOfChange));

		_LayoutBufferFont.SetPointSize(_LayoutBufferFontSizeWantedSize);
		_LayoutLinesIndexingFont.SetPointSize(_LayoutLinesIndexingFontWantedSize);

		/* Handle spacing change */
		if (_LayoutBufferFont.GetPointSize()
			== 1)
		{ /* Swap to 0 */
			cr_List.m_BufferSpacingY = 0;
		} else {cr_List.m_BufferSpacingY = cr_List._LayoutBufferSpacingY;}

		if (_LayoutLinesIndexingFont.GetPointSize()
			== 1)
		{ /* Swap to 0 */
			cr_List.m_LineIndexingSpacingY = 0;
		} else {cr_List.m_LineIndexingSpacingY = cr_List._LayoutLineIndexingSpacingY;}


		p_SmartRefreshObjects({ m_Buffer, m_LineIndexing, m_Cursor });
		hdl_ScrollbarsUpdatePositions();
	}
	
	/* [=============================== _Cursor ===============================] */
	void TextEditor::init_Cursor()
	{ /* _Cursor initializer */
        Bind(wxEVT_LEFT_UP, &TextEditor::hk_CursorPlace, this);
	}

	void TextEditor::hk_CursorRender(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render cursor */
		if (cr_List._CursorActive ==
			false) return; 
		hdl_CursorSetupClippingRegion(_Canvas);

		hk_CursorRenderObject(_Canvas);
		
		_Canvas.DestroyClippingRegion();
	}

	void TextEditor::hdl_CursorSetupClippingRegion(
		wxAutoBufferedPaintDC& _Canvas) 
	{ /* Setup clipping region for _Cursor -> _BufferBB */
		_LayoutPositions _Layout = g_LayoutPositions();

		_Canvas.SetClippingRegion(
			_Layout.g_BBFromPackedArray(_Layout._BufferBB).gRect()
		);
	}

	void TextEditor::hk_CursorRenderObject(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render object of a cursor */
		/* Outline */
		_Canvas.SetPen(cr_List._LayoutCursorOutlineColor);
		_Canvas.SetBrush(cr_List._LayoutCursorOutlineColor);

		_Canvas.DrawRectangle(g_CursorOutlineBB().gRect());
		/* Real */
		_Canvas.SetPen(cr_List._LayoutCursorColor);
		_Canvas.SetBrush(cr_List._LayoutCursorColor);

		_Canvas.DrawRectangle(g_CursorBB().gRect());
	}

	void TextEditor::hk_CursorType(
		wxKeyEvent& _Event, const std::string& _Char
	) {
		if (g_CursorIsPlaced()
			== false) return; 
		hdl_CursorInsertIntoBuffer(_Char.at(0), 1);
	}

	void TextEditor::hdl_CursorInsertIntoBuffer(
		const char& _Char, const size_t& _Count)
	{ /* Insertion */
		std::string& _BufferLine = cr_List._Buffer[
			cr_List._CursorAt[0]];

		_BufferLine.insert(cr_List._CursorAt[
			1 /* At Char */
		], _Count, _Char);
		/* Handle */
		hdl_CursorInsertIntoBufferAfter(_Count);
	}

	void TextEditor::hk_CursorTypeTabulator(
		wxKeyEvent& _Event
	) {
		if (g_CursorIsPlaced()
			== false) return; 
		/* Check for tab key */
		if (_Event.GetKeyCode()
			!= m_OnCharKeys::m_Tab) return; 

		hdl_CursorInsertIntoBuffer(' ',
			cr_List._LayoutCursorTabLength);
	}

	void TextEditor::hdl_CursorInsertIntoBufferAfter(
		const size_t& _Count)
	{ /* Handle actions after text insertion */
		/* Increment cursor position */
		cr_List._CursorAt[1] += _Count;

		/* Other */
		hdl_CursorGlobalChange();
	}

	void TextEditor::hk_CursorDelete(
		wxKeyEvent& _Event 
	) { /* Handle Deletion for _Cursor -> _Buffer */
		if (g_CursorIsPlaced()
			== false) return; 
		/* Check for backspace */
		if (_Event.GetKeyCode()
			!= m_OnCharKeys::m_Backspace) return;

		const size_t& _AtChar = cr_List._CursorAt[1];
		const size_t& _AtLine = cr_List._CursorAt[0];

		std::string& _BufferLine = cr_List._Buffer[
			_AtLine
			];
		const size_t _EraseCount = g_CursorDeletionEraseCount(
			_Event.m_controlDown
			);

		bool _DeletingLine = (_AtChar == 0);

		/* Char deletion */
		if (_DeletingLine
			== false)
		{ /* Deletion singular char infront */
			size_t _EraseAt = (_AtChar );

			cr_List._Buffer[
				_AtLine
			] = Framework::String::Erase(_BufferLine, _EraseAt, _EraseCount);
			/* Call action afterwards */
			hdl_CursorDeleteAfter(_EraseCount, false);
			return; 
		} /* Line Deletion */
		const size_t _LowestLine = 0;

		if (_AtLine == /* Nothing do delete */
			_LowestLine) return;
		/* Calculate positions */
		const std::string _BufferLineDeletionRest = _BufferLine.substr(
			_AtChar);

		const auto _ErasePosition = (cr_List._Buffer.begin()
			+ _AtLine);
		/* Erase */
		cr_List._Buffer.erase(_ErasePosition);
		/* Override line before */
		cr_List._Buffer[_AtLine - 1] += _BufferLineDeletionRest;
		
		/* Call handler : use len to determinate how many chars are added to forwarding line */
		hdl_CursorDeleteAfter(
			_BufferLineDeletionRest.length(), true
		);
	}

	void TextEditor::hdl_CursorDeleteAfter(
		const size_t& _Count, const bool& _DeleteLine)
	{ /* Handle after deletion */
		size_t& _AtChar = cr_List._CursorAt[1];

		if (_DeleteLine
			== false)
		{ /* Deleted chars */
			_AtChar -= _Count;

			_AtChar = std::max(
				(size_t)0, _AtChar);
			/* Refresh & Return */
			hdl_CursorGlobalChange(); return;
		} /* Delete line */
		size_t& _AtLine = cr_List._CursorAt[0];
 
		const size_t _LineBeforeLength = (cr_List._Buffer[_AtLine - 1].length() - _Count);
		/* Mod variables */
		_AtLine--;
		_AtChar = _LineBeforeLength;
		/* Finalize */
		hdl_CursorGlobalChange();
	}

	void TextEditor::hk_CursorEnter(
		wxKeyEvent& _Event)
	{ /* Cursor enter key handler : _Cursor -> _Buffer */
		if (g_CursorIsPlaced()
			== false) return;
		/* Check for enter */
		if (_Event.GetKeyCode() !=
			m_OnCharKeys::m_Enter) return; 

		bool _CursorNewLine = (_Event.m_controlDown
			== false ? true : false);

		if (_CursorNewLine
			== true)
		{ /* Call new line handler */
			hdl_CursorNewLine(); 
		}
		else
		{ /* Call stay in place in order to leave cursor in place and push buffer forwards*/
			hdl_CursorStayInPlace();
		}
		/* Call Handler */
		hdl_CursorEnterAfter(_CursorNewLine);
	}

	void TextEditor::hdl_CursorNewLine()
	{ /* Create new cursor line */
		std::vector<std::string>& _Buffer = cr_List._Buffer;
		const size_t& _AtLine = cr_List._CursorAt[0];
		const size_t& _AtChar = cr_List._CursorAt[1];

		std::string& _BufferCurrentLine = _Buffer[_AtLine];

		std::string _TextToPass = (_BufferCurrentLine.substr(
			_AtChar));
		/* Inclue tabulation*/
		const size_t _TabulationWanted = ( /* Substr to exclude spaces after cursor*/
			g_CursorTabulationCount(_BufferCurrentLine.substr(0, _AtChar))
			);
		_TextToPass = (g_CursorTabulationInStr(_TabulationWanted) + _TextToPass);
		/* Fix current part of the buffer*/
		_BufferCurrentLine = _BufferCurrentLine.substr(0, _AtChar);
		/* Add another segement */
		const auto _BufferInsertionAt = (_Buffer.begin()
			+ (_AtLine + 1));
		_Buffer.insert(_BufferInsertionAt, _TextToPass);
	}

	void TextEditor::hdl_CursorStayInPlace()
	{ /* Create new line but stay in place */
		const size_t& _AtLine = cr_List._CursorAt[0];

		const auto _InsertAt = (cr_List._Buffer.begin()
			+ _AtLine);
		
		cr_List._Buffer.insert(
			_InsertAt, g_CursorTabulationInStr(g_CursorTabulationCount(cr_List._Buffer[_AtLine]))
		); 
	}

	void TextEditor::hdl_CursorEnterAfter(
		const bool& _NewLineAdded)
	{ /* Handle actions after cursor modification */
		std::array<size_t, 2>& _CursorAt = cr_List._CursorAt;

		const std::string _CurrentBufferLine = cr_List._Buffer[
			_CursorAt[0]];

		if (_NewLineAdded 
			== false)
		{ /* Stay in place action */
			_CursorAt[1] = g_CursorTabulationCount(_CurrentBufferLine);

			/* Call render update & Return */
			hdl_CursorGlobalChange(); return;
		} /* Line is added */
		_CursorAt[0]++;
		_CursorAt[1] = g_CursorTabulationCount(_CurrentBufferLine);

		/* Call refersher */
		hdl_CursorGlobalChange();
	}

	void TextEditor::hk_CursorArrows(
		wxKeyEvent& _Event)
	{ /* Cursor arrows _Cursor -> _Cursor & _Buffer */
		if (g_CursorIsPlaced()
			== false) return; 
		const int _KeyCode = _Event.GetKeyCode(); 

		const bool _ArrowsHorizontal = (
			(_KeyCode == m_OnCharKeys::m_LeftArrow) || (_KeyCode == m_OnCharKeys::m_RightArrow)
			);
		const bool _ArrowsVertical = (
			(_KeyCode == m_OnCharKeys::m_UpArrow) || (_KeyCode == m_OnCharKeys::m_DownArrow)
			);
		/* Check for arrows keys */
		if (!_ArrowsHorizontal &&
			!_ArrowsVertical) return;

		/* Calls */
		if (_ArrowsHorizontal == 
			true) /* Horizontal call */
		{ hdl_CursorHorizontalArrows(_Event); }

		if (_ArrowsVertical
			== true) /* Vertical call */
		{ hdl_CursorVerticalArrows(_Event); }

		/* Call globalhandler */
		hdl_CursorGlobalChange();
	}

	void TextEditor::hdl_CursorHorizontalArrows(
		wxKeyEvent& _Event) 
	{ /* Handle horizontal arrows */
		const int _KeyCode = _Event.GetKeyCode(); 
		const bool _Jump = (_Event.m_controlDown);
		/* Line variables */
		size_t& _AtChar = cr_List._CursorAt[1];
		size_t& _AtLine = cr_List._CursorAt[0];

		std::vector<std::string>& _Buffer = cr_List._Buffer;
		/* Function code */
		if (_Jump
			== true)
		{ /* Attempted jump */
			bool _JumpStatus = hdl_CursorHorizontalArrowsJump(
				(_KeyCode == m_OnCharKeys::m_LeftArrow) ? m_CursorArrowsDirection::m_Backward
				: m_CursorArrowsDirection::m_Forward
			);
			if (_JumpStatus /* Jumped no need to perform any action */
				== true) return;
		}

		if (_KeyCode
			== m_OnCharKeys::m_LeftArrow)
		{ /* Left arrow */
			if (_AtChar
				== 0)
			{ /* Go to forwarding line*/
				hdl_CursorHorizontalArrowsMoveToPreviousLine(
					_AtLine, _AtChar, _Buffer
				);
				/* Exit*/
				return;
			}
			_AtChar--;
			return;
		} /* Right arrow */
		if (_AtChar
			== _Buffer[_AtLine].length())
		{ /* Go to next line*/
			hdl_CursorHorizontalArrowsMoveToNextLine(
				_AtLine, _AtChar, _Buffer
			);
			/* Exit */
			return;
		}
		_AtChar++;
	}

	void TextEditor::hdl_CursorHorizontalArrowsMoveToNextLine(
		size_t& _AtLine, size_t& _AtChar, const std::vector<std::string>& _Buffer
	) {
		if (_AtLine == (
			_Buffer.size() - 1
			)) return;
		const size_t _NextLine = (_AtLine + 1);
		/* Modify */
		_AtLine++;
		_AtChar = 0;
	}

	void TextEditor::hdl_CursorHorizontalArrowsMoveToPreviousLine(
		size_t& _AtLine, size_t& _AtChar, const std::vector<std::string>& _Buffer
	) {
		if (_AtLine == 0)
			return;
		const size_t _ForwardingLine = (_AtLine - 1);
		/* Modify */
		_AtLine--;
		_AtChar = _Buffer[
			_ForwardingLine
		].length();
	}

	const bool TextEditor::hdl_CursorHorizontalArrowsJump(
		m_CursorArrowsDirection _Direction)
	{ /* Jumping */
		size_t& _AtChar = cr_List._CursorAt[1];
		const size_t& _AtLine = cr_List._CursorAt[0];

		const std::string& _CurrentLine = (cr_List._Buffer[
			_AtLine
		]);
		/* Get position */
		const size_t _JumpTo = g_CursorJumpingPosition(
			_CurrentLine, _AtChar, _Direction
		); 
		
		if (_JumpTo == /* Nowhere to jump */
			std::string::npos) return false;
		_AtChar = _JumpTo;
		/* Jump successfull */
		return true; 
	}

	void TextEditor::hdl_CursorVerticalArrows(
		wxKeyEvent& _Event)
	{ /* Handle vertical arrows */
		const int _Keycode = _Event.GetKeyCode(); 

		/* Line variables */
		size_t& _AtChar = cr_List._CursorAt[1];
		size_t& _AtLine = cr_List._CursorAt[0];

		std::vector<std::string>& _Buffer = cr_List._Buffer;
		/* Function code */
		if (_Keycode
			== m_OnCharKeys::m_UpArrow)
		{ /* Up arrow */
			hdl_CursorVerticalArrowsMove(
				_AtLine, _AtChar, _Buffer, m_Backward
			); return; 
		} /* Down arrow */
		hdl_CursorVerticalArrowsMove(
			_AtLine, _AtChar, _Buffer, m_Forward
		); 
	}

	void TextEditor::hdl_CursorVerticalArrowsMove(
		size_t& _AtLine, size_t& _AtChar, const std::vector<std::string>& _Buffer, 
		m_CursorArrowsDirection _Direction)
	{ /* Move to next line */
		const size_t _BufferStart = 0;
		const size_t _BufferSizeFixed = (_Buffer.size() - 1);
		const size_t _WantedPosition = (_AtLine + (
			(_Direction == m_Backward) ? -1 : 1
			));

		/* Check for bounds of a vector */
		if (Framework::Geometry::cBound1D(
			_WantedPosition, _BufferStart, _BufferSizeFixed
			) == false) return; 

		const size_t _WantedAtChar = std::min(_Buffer[_WantedPosition].length(),
			_AtChar);

		_AtLine = _WantedPosition;
		_AtChar = _WantedAtChar;
	}


	void TextEditor::hdl_CursorGlobalChangeModifyBufferScroll()
	{ /* Modify scroll based on cursor position */
		/* Initialize layout and bounding boxes */
		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _BufferBB = _Layout.g_BBFromPackedArray(
			_Layout._BufferBB);
		Framework::Geometry::BoundingBox _CursorBB = g_CursorBB();

		/* Positional differences to account for in visibility */
		const size_t _PositionalDif = 1;

		const size_t _MaximalCharsVisible = 
			(g_BufferMaximalCharsVisible() - _PositionalDif);

		const size_t _MaximalLinesVisible = 
			(g_BufferMaximalLinesVisible() - _PositionalDif);

        /* Character and line sizes */
        const size_t _CharSizeX = g_BufferCharSizeX();
        const size_t _LineSizeY = g_BufferLineSizeY();

        /* Get maximal scroll values */
        const std::array<int, 2> _MaximalScrolls = g_BufferMaximalScroll();

        const std::array<size_t, 2> _VisibleStartingFrom = {
             cr_List._BufferScroll[0] / _CharSizeX, cr_List._BufferScroll[1] / _LineSizeY
        };

		/* Current cursor position in buffer */
		const size_t& _AtChar = cr_List._CursorAt[1];
		const size_t& _AtLine = cr_List._CursorAt[0];

 
		/* Calculate differences between cursor and visible area */
		std::array<int, 2> _CursorDifs = {
			(_MaximalCharsVisible - _AtChar),
			(_MaximalLinesVisible - _AtLine)
		};

		/* Calculate the scroll position wanted based on cursor */
		std::array<int, 2> _ScrollWanted = { 0, 0 };

		/* Horizontal scroll: move left or right */
		_ScrollWanted[0] = (_VisibleStartingFrom[0] < _AtChar)
			? (std::abs(_CursorDifs[0]) * _CharSizeX)
			: (_AtChar * _CharSizeX);

		/* Vertical scroll: move up or down */
		_ScrollWanted[1] = (_VisibleStartingFrom[1] < _AtLine)
			? (std::abs(_CursorDifs[1]) * _LineSizeY)
			: (_AtLine * _LineSizeY);


		/* Calculate scroll percentage for horizontal and vertical scrolling */
		std::array<double, 2> _ScrollWantedProcentage = {
			static_cast<double>(_ScrollWanted[0]) / _MaximalScrolls[0],
			static_cast<double>(_ScrollWanted[1]) / _MaximalScrolls[1]
		};

		const wxPoint _CursorCenteredRectPosition = _CursorBB.gCenterRect().GetPosition();
		const wxPoint _BufferStartingPosition = _BufferBB.gStarting();
		const wxSize _BufferBBSize = _BufferBB.gSize();

		/* Apply horizontal scroll if cursor is out of bounds */
		if (!Framework::Geometry::cBound1D(
			_CursorCenteredRectPosition.x, _BufferStartingPosition.x, _BufferBBSize.x
			)) { /* Make changes to slider */
			m_ScrollbarsHorizontal->sScrollProcentage(_ScrollWantedProcentage[0]);
		}

		/* Apply vertical scroll if cursor is out of bounds */
		if (!Framework::Geometry::cBound1D(
			_CursorCenteredRectPosition.y, _BufferStartingPosition.y, _BufferBBSize.y
		)) { /* Make changes to slider */
			m_ScrollbarsVertical->sScrollProcentage(_ScrollWantedProcentage[1]);
		}
	}

    void TextEditor::hk_CursorPlace(
        wxMouseEvent& _Event)
    { /* Mouse handler for cursor */
        const wxPoint _MousePosition = _Event.GetPosition();
        /* Get layout positions */
        _LayoutPositions _Layout = g_LayoutPositions();

        Framework::Geometry::BoundingBox _BufferBB = _Layout.g_BBFromPackedArray(
            _Layout._BufferBB);

        const wxPoint _BufferPosition = _BufferBB.gStarting();
        const wxSize _BufferSize = _BufferBB.gSize();
        /* Check if cursor can be placed */
        if (Framework::Geometry::cBounds(
            _MousePosition, _BufferPosition, _BufferSize
        ) == false)
        { /* Skip event if failed */
            _Event.Skip(); return; 
        }
        /* Check in what line and char the position is located */
        const std::array<size_t, 2> _CursorWantedAt = g_BufferLineDataAtGivenPosition(
            _MousePosition);
        /* Modify cursor positions */
        cr_List._CursorAt = _CursorWantedAt; 
        /* Call handler */
        hdl_CursorGlobalChange(); 
    }

	void TextEditor::hdl_CursorGlobalChange()
	{ /* Handle cursor global changes */
		cr_List._BufferActivatedLine = cr_List._CursorAt[0];
		cr_List._LineIndexingActivatedLines = { cr_List._BufferActivatedLine };
		hdl_CursorGlobalChangeModifyBufferScroll(); 

        p_SmartRefreshObjects({ m_Global });
		hdl_ScrollbarsUpdatePositions();
	}

	const size_t TextEditor::g_CursorJumpingPosition(
		const std::string& _Line, const size_t& _At, m_CursorArrowsDirection _Direction) const
	{ /* Calculate & Return */
		if (_At == 0 || /* Check if operation can even be performed */
			_At == _Line.length()) return std::string::npos; 
		/* If iterator starts on a char then serach for space if iterator starts on space search for char*/
		const std::string _SpaceChar = " ";
		const char _IterationStartedAt = _Line.at(_At);

		bool _SearchForSpace = (
			_IterationStartedAt == _SpaceChar ? false : true
			);

		/* _Iterator @params */
		const size_t _IteratorMax = (_Line.length() - 1); /* Fixed size */
		const size_t _IteratorMin = 0; 

		size_t _IterationFound = std::string::npos;
		size_t _Iterator = _At;
		size_t _IterationFinal = std::string::npos;

		bool _SearchSwapped = false; 
		while (true)
		{ /* Perform search */
			_Iterator += (_Direction == m_Backward ? -1 : 1);

			if (_Iterator > _IteratorMax /* Stop if needed */
				|| _Iterator < _IteratorMin) break; 

			const std::string _CurrentChar = std::string(1, 
				_Line.at(_Iterator));

			if (_SearchForSpace ==
				true && _CurrentChar == _SpaceChar)
			{ /* Found wanted char: space */
				_IterationFound = _Iterator; 

				if (_SearchSwapped == true)
					break;
			}

			if (_SearchForSpace ==
				false && _CurrentChar != _SpaceChar)
			{ /* Found wanted char: other than space */
				_IterationFound = _Iterator; 

				if (_SearchSwapped == true)
					break;
			}

			if (_IterationFound 
				!= std::string::npos && _SearchSwapped == false)
			{ /* Second stage of iteration search */
				_SearchForSpace = not _SearchForSpace;
				_SearchSwapped = true; 
				_IterationFound = std::string::npos;
			}
			/* Handle iterator movement */
		}
		if (_IterationFound ==
			std::string::npos)
		{ /* Return eaither start or end of a line */
			return (_Direction ==
				m_Backward ? 0 : _Line.length()
				);
		}

		if (_Direction == m_Backward)
		{ /* Swap char on dir */
			_IterationFound++;
		}

		return _IterationFound;
	}

	const std::string TextEditor::g_CursorTabulationInStr(
		const size_t& _Count) const 
	{ /* Add up & Return */
		std::string _Tabulation; 

		for (size_t _Iterator = 0; 
			_Iterator < _Count; _Iterator++
			) {
			_Tabulation += " ";
		}
		/* Return */
		return _Tabulation;
	}

	const size_t TextEditor::g_CursorTabulationCount(
		const std::string& _Line) const 
	{ /* Calculate & Return */
		const size_t _TabulationLength = cr_List._LayoutCursorTabLength;

		size_t _Count = 0; 
		for (size_t _Iterator = 0;
			_Iterator < _Line.length(); _Iterator++)
		{ /* Calculate tabulation */
			if (_Line.at(_Iterator) 
				!= ' ') break;
			_Count++; 
		} /* Return */
		const size_t _CountRest = (_Count % _TabulationLength);
		return (
			(_Count - _CountRest)
			);
	}

	const size_t TextEditor::g_CursorDeletionEraseCount(
		const bool& _IsCtrlDown) const
	{ /* Include tabulation */
		const size_t& _AtLine = cr_List._CursorAt[0];
		const size_t& _AtChar = cr_List._CursorAt[1];
		const size_t& _Tabulation = cr_List._LayoutCursorTabLength;

		/* Content deletion */
		if (_IsCtrlDown
			== true)
		{ /* return char position */
			return _AtChar;
		}
		const size_t _DefaultResult = 1;
		if (_AtChar < _Tabulation)
			return _DefaultResult; /* Nothing to check */

		size_t _Count = 0; 
		size_t _Iterator = (_AtChar - 1);

		while (true)
		{ /* Count whitespaces */
			if (_Iterator == -1)
				break;
			/* Check if a condition is met */
			if (std::string(1,
				cr_List._Buffer[_AtLine].at(_Iterator)
			) != " ")
				break;

			_Count++;
			_Iterator--; 
		}

		if ((_Count % _Tabulation)
			== 0 && _Count > 0) return _Tabulation;
		return _DefaultResult;
	}

	const bool TextEditor::g_CursorIsPlaced() const
	{ /* Check if placed */
		const size_t _AtLine = cr_List._CursorAt[0];
		const size_t _AtChar = cr_List._CursorAt[1];

		if (_AtLine == -1
			|| _AtChar == -1)
			return false; 
		return true; /* Placed */
	}

	Framework::Geometry::BoundingBox 
		TextEditor::g_CursorBB()
	{ /* Calculate & Return */
		Framework::Geometry::BoundingBox
			_BufferLineBB = g_BufferLineBBAtGivenLineAndChar(
				cr_List._CursorAt[0], cr_List._CursorAt[1]
			);
		/* Calculate */
			int _x, _y, _fx, _fy;

			_x = _BufferLineBB.gStarting().x; 
			_y = _BufferLineBB.gStarting().y; 
			_x -= cr_List._BufferScroll[0];

			_fx = _x;
			_fy = _y; 

			_fx += (cr_List._LayoutCursorSizeX);
			_fy += (_BufferLineBB.gSize().y);
		return {wxPoint { _x, _y }, wxPoint { _fx, _fy }};
	}

	Framework::Geometry::BoundingBox 
		TextEditor::g_CursorOutlineBB()
	{ /* Calculate & Return */
		Framework::Geometry::BoundingBox
			_CursorBB = g_CursorBB();

		/* Calculate */
			int _x, _y, _fx, _fy;

			_x = _CursorBB.gStarting().x; 
			_y = _CursorBB.gStarting().y; 
			_fx = _CursorBB.gFinal().x; 
			_fy = _CursorBB.gFinal().y; 

			_x -= (cr_List._LayoutCursorOutlineSize);
			_y -= (cr_List._LayoutCursorOutlineSize);
			_fx += (cr_List._LayoutCursorOutlineSize);
			_fy += (cr_List._LayoutCursorOutlineSize);
		return {wxPoint { _x, _y }, wxPoint { _fx, _fy }};
	}

	/* [=============================== _OnChar ===============================] */
	void TextEditor::init_OnChar()
	{ /* _OnChar Initializer */
		Bind(wxEVT_CHAR_HOOK, &TextEditor::hk_OnChar, this);
	}

	void TextEditor::hk_OnChar(
		wxKeyEvent& _Event)
	{ /* Main OnChar Event for this class */
		try {
			/* Call default hooks that don't require modulated chars */
			hk_CursorDelete(_Event);
			hk_CursorTypeTabulator(_Event);
			hk_CursorEnter(_Event);
			hk_CursorArrows(_Event);

			/* End of calls */
			const std::string _RealChar = g_OnCharRealChar(
				_Event.GetUnicodeKey(), _Event.m_shiftDown);

			if (_RealChar ==
				"UNDEFINED") return; 
			/* Call hooks that require modulated chars */
			hk_CursorType(_Event, _RealChar);
		} catch (const std::exception& _ex) { cr_List._Debug->log_TaggedStdException(
			__FUNCTION__, _ex 
		);}
	}

	const std::string TextEditor::g_OnCharRealChar(
		const int& _UnicodeKey, const bool& _ShiftIsDown) const
	{ /* Transform unicode key to real char & Return */
		const std::string _RealChars =
			"~1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./ ";
		const std::string _RealCharsWithShift =
			"~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>? ";

		for (size_t _Iterator = 0;
			_Iterator < _RealChars.length(); _Iterator++)
		{ /* Search */
			const std::string _CurrentChar = std::string(1,
				_RealChars.at(_Iterator));

			bool _CharMatching = false; 

			if (_CurrentChar == std::string(
				1, std::tolower(_UnicodeKey))
				) { 
				_CharMatching = true; 
			}

			if (_CharMatching &&
				_ShiftIsDown)
			{ /* Upper */
				return std::string(1, _RealCharsWithShift[
					_Iterator
				]); 
			}

			if (_CharMatching)
			{ /* Lower */
				return _CurrentChar; 
			}

		} /* Return invaild char */
		return "UNDEFINED";
	}
	/* [=============================== _SmartRefresh ===============================] */
	void TextEditor::init_SmartRefresh()
	{
		m_SmartRefreshUpdateTimer.SetOwner(this);
		m_SmartRefreshUpdateTimer.Start(cr_List._SmartRefreshUpdateTimerMs);
		Bind(wxEVT_TIMER, &TextEditor::hk_SmartRefreshUpdate, this, m_SmartRefreshUpdateTimer.GetId());
	}

	void TextEditor::hk_SmartRefreshUpdate(
		wxTimerEvent& _Event)
	{ /* _SmartRefresh update hook */
		try 
		{ /* Function calls */
			hdl_SmartRefreshUpdate();
		} catch (const std::exception& ex) { cr_List._Debug->log_TaggedStdException(
			__FUNCTION__, ex
		); }
	}

    void TextEditor::p_SmartRefreshObjects(
        const std::vector<m_SmartRefreshObjects>& _Objects)
    { /* Push and exit */
        for (const m_SmartRefreshObjects& _Object :
            _Objects)
        { /* Push */
            m_SmartRefreshBuffer.push_back(_Object);
        }
    }

	void TextEditor::hdl_SmartRefreshUpdate()
	{ /* Update performer */
		const size_t _RefreshObjectsCount = 4;

        std::array<bool, _RefreshObjectsCount> _RefreshObjects;
        /* Force false on all */
        for (size_t _Iterator = 0; _Iterator <
            _RefreshObjects.size(); _Iterator++)
        { /* Override to false by defautl */
            _RefreshObjects[_Iterator] = false; 
        }
		/* Collect data */
        bool _GlobalRefresh = false; 

        for (m_SmartRefreshObjects _Object : m_SmartRefreshBuffer)
        {
            if (_Object !=
                m_Global)
            { /* Regular object */
                _RefreshObjects[_Object] = true;
            } else { _GlobalRefresh = true; }
        }
        /* Apply if global */
        if (_GlobalRefresh
            == true)
        { /* Set */
            for (size_t _Iterator = 0;
                _Iterator < _RefreshObjects.size(); _Iterator++)
            { /* Override */
                _RefreshObjects[_Iterator] = true; 
            }
        }

        /* Collect refreshments */
        std::array<std::array<int, 4>, _RefreshObjectsCount> _Refreshments;
        _LayoutPositions _Layout = g_LayoutPositions();

        Framework::Geometry::BoundingBox _CursorOutlineBB = g_CursorOutlineBB();

        _Refreshments[0] = _Layout._BufferBB;
        _Refreshments[1] = _Layout._LineIndexingBB;
        _Refreshments[2] = _Layout.g_PackedArrayFromBB(_CursorOutlineBB);
		_Refreshments[3] = _Layout._InfoBB;

        /* Refresh */
        size_t _RefreshIterator = 0; 
        for (const bool& _ObjectState :
            _RefreshObjects)
        { /* Check if needed */
            if (_ObjectState ==
                true)
            { /* Perform refresh */
                RefreshRect(
                    _Layout.g_BBFromPackedArray(
                        _Refreshments[_RefreshIterator]
                    ).gRect()
                );
            } /* Increment iterator */
            _RefreshIterator++;
        } /* Clear buffer */
        m_SmartRefreshBuffer = {}; 
	}

	/* [============================== _Info ===============================] */
	void TextEditor::init_Info()
	{ /* _Info initializer */
		//hdl_InfoSetupElements();
	}

	void TextEditor::hdl_InfoSetupElements()
	{
		m_InfoElements = {};

		p_InfoElement(
			cr_List._LayoutInfoCharSystem, "@Character set currently in use.", m_InfoElement::m_FromRight
		);

		p_InfoElement(
			cr_List._LayoutInfoEndlineStandardCurrent, "@End-line system currently in use.", m_InfoElement::m_FromRight
		);

		p_InfoElement(
			std::to_string(cr_List._CursorAt[0]) + ":" + std::to_string(cr_List._CursorAt[1]), "@line:char", m_InfoElement::m_FromLeft
		);

		p_InfoElement(
			"SPACE", "@Current character for tabulation.", m_InfoElement::m_FromRight
		);
	}

	void TextEditor::hk_InfoRender(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render hook for info */
		try {
			hdl_InfoSetupElements();

			hk_InfoRenderBackground(_Canvas);
			hk_InfoRenderSeparator(_Canvas);
			hk_InfoRenderElements(_Canvas);
			hk_InfoRenderComment(_Canvas);
		} catch (const std::exception& _ex) { cr_List._Debug->log_TaggedStdException(
			__FUNCTION__, _ex
		); }
	}

	void TextEditor::hk_InfoRenderComment(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render Current comment */
		const size_t _NoCommentActive = -1; 

		if (m_InfoElementActivated
			== _NoCommentActive) return;
		m_InfoElement& _Element = m_InfoElements[m_InfoElementActivated];

		const wxPoint _TextPosition = g_InfoElementCommentCenteredTextPosition(
			_Element);
		const wxRect _BackgroundRect = g_InfoElementCommentBB(_Element).gRect();
		/* Set Clipping Region */
		_Canvas.SetClippingRegion(_BackgroundRect);
		/* Draw Background */
		_Canvas.SetBrush(cr_List._LayoutInfoBackgroundColor);
		_Canvas.SetPen(cr_List._LayoutInfoBackgroundColor);

		_Canvas.DrawRectangle(_BackgroundRect);
		/* Draw text */
		_Canvas.SetTextForeground(cr_List._LayoutInfoElementCommentColor);
		_Canvas.DrawText(_Element.m_Comment, _TextPosition);
		/* Finalize */
		_Canvas.DestroyClippingRegion();
	}

	void TextEditor::hk_InfoRenderBackground(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render background */
		/* Setup */
		_Canvas.SetBrush(cr_List._LayoutInfoBackgroundColor);
		_Canvas.SetPen(cr_List._LayoutInfoBackgroundColor);

		/* Draw */
		_LayoutPositions _Layout = g_LayoutPositions();
		
		_Canvas.DrawRectangle(
			_Layout.g_BBFromPackedArray(_Layout._InfoBB).gRect()
		);
	}

	void TextEditor::hk_InfoRenderSeparator(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render Separator */
		/* Setup */
		_Canvas.SetBrush(cr_List._LayoutInfoSeparatorColor);
		_Canvas.SetPen(cr_List._LayoutInfoSeparatorColor);

		/* Render */
		_Canvas.DrawRectangle(g_InfoSeparatorBB().gRect());
	}

	void TextEditor::p_InfoElement(
		const std::string& _Text, const std::string& _Comment,
		m_InfoElement::m_LayoutPositions _LayoutPosition)
	{ /* Push element to m_InfoElements*/
		/* Assemble */
		m_InfoElement _InfoElement; 
		_InfoElement.m_Layout = _LayoutPosition;
		_InfoElement.m_Comment = _Comment;
		_InfoElement.m_Text = _Text;
		_InfoElement.m_CountForCurrentLayout = g_InfoElementCurrentLayoutCount(_LayoutPosition);
		_InfoElement.m_Index = m_InfoElements.size();
		/* Push */
		m_InfoElements.push_back(_InfoElement);
	}

	const size_t TextEditor::g_InfoElementCurrentLayoutCount(
		m_InfoElement::m_LayoutPositions _CurrentLayout) const 
	{ /* Get & Return */
		size_t _CountForCurrentLayout = 0;
		for (m_InfoElement _Element : m_InfoElements)
		{ /* Get count for current layout */
			if (_Element.m_Layout == _CurrentLayout)
				_CountForCurrentLayout++;
		} 
		return _CountForCurrentLayout;
	}

	void TextEditor::hk_InfoRenderElements(
		wxAutoBufferedPaintDC& _Canvas)
	{ /* Render Elements */
		for (const m_InfoElement _Element : m_InfoElements)
		{ /* Render */
			hdl_InfoRenderElement(_Canvas, _Element);
		}
	}

	void TextEditor::hdl_InfoDeactivateElement()
	{
		if (m_InfoElementActivated == -1)
			return; 
		/* Deaactivate & Refresh */
		m_InfoElementActivated = -1;
		p_SmartRefreshObjects({ m_Info });
	}

	void TextEditor::hk_InfoActivateElement(
		wxMouseEvent& _Event)
	{ /* Activation of a element */
		const wxPoint _MousePosition = _Event.GetPosition();

		_LayoutPositions _Layout = g_LayoutPositions();
		Framework::Geometry::BoundingBox _InfoBB = _Layout.g_BBFromPackedArray(
			_Layout._InfoBB);

		/* Check if mouse is in BB box of _Info */
		if (Framework::Geometry::cBounds(
			_MousePosition, _InfoBB.gStarting(), _InfoBB.gSize()
		) == false)
		{ /* Deactivate element if active & return */
			hdl_InfoDeactivateElement(); return; 
		}

		/* Iterate throught elements and check whatever position is matching */
		for (
			const m_InfoElement& _Element : m_InfoElements
			)
		{ /* Check if mouse is in bounding box of element */
			Framework::Geometry::BoundingBox _ElementBoundingBox =
				g_InfoElementBB(_Element);

			if (Framework::Geometry::cBounds(
				_MousePosition, _ElementBoundingBox.gStarting(), _ElementBoundingBox.gSize()
			) == true)
			{ /* Activate elements & refresh & return */
				m_InfoElementActivated = _Element.m_Index;

				p_SmartRefreshObjects({ m_Info });
				/* Abort */
				return;
			}
		} 
		/* Deactivate element if active */
		hdl_InfoDeactivateElement();
	}

	void TextEditor::hdl_InfoRenderElement(
		wxAutoBufferedPaintDC& _Canvas, const m_InfoElement& _Element)
	{ /* Render singular element */
		/* Render colored background if active */
		if (_Element.m_Index ==
			m_InfoElementActivated)
		{ /* Render colored background */
			_Canvas.SetPen(cr_List._LayoutInfoElementActiveBackgroundColor);
			_Canvas.SetBrush(cr_List._LayoutInfoElementActiveBackgroundColor);

			const wxRect _ColoredBackgroundRect = g_InfoElementBB(
				_Element
			).gRect();

			_Canvas.DrawRectangle(_ColoredBackgroundRect);
		}

		/* Render Text */
		_Canvas.SetFont(cr_List._LayoutInfoFont);
		_Canvas.SetTextForeground(cr_List._LayoutInfoFontColor);

		_Canvas.DrawText(_Element.m_Text, g_InfoElementCenteredTextPosition(_Element));
	}

	const wxPoint TextEditor::g_InfoElementCenteredTextPosition(
		const m_InfoElement& _Element)
	{ /* Calculate & Return */
		Framework::Geometry::BoundingBox _ElementBB = g_InfoElementBB(_Element);
		const wxPoint _ElementBBStarting = _ElementBB.gStarting();

		/* Calculate */
			int _x, _y;

			_x = _ElementBBStarting.x;
			_y = _ElementBBStarting.y; 

			_x += (cr_List._LayoutInfoElementSpacingX / 2);
			_y += (cr_List._LayoutInfoSpacingY / 2);
		return wxPoint { _x, _y };
	}

	Framework::Geometry::BoundingBox TextEditor::g_InfoElementCommentBB(
		const m_InfoElement& _Element)
	{ /* Calculate & Return */
		int _CharSizeX, _CharSizeY;
		int _sx, _sy;
		int _x, _y; 
		int _fx, _fy;

		/* Get char sizes */
		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutInfoFont);

		const wxSize _CharSize = _MemoryCanvas.GetTextExtent("a");
		
		_CharSizeX = _CharSize.x;
		_CharSizeY = _CharSize.y; 
		/* Get sx & sy */
		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _InfoBB = _Layout.g_BBFromPackedArray(
			_Layout._InfoBB);
		Framework::Geometry::BoundingBox _ElementBB = g_InfoElementBB(_Element);

		size_t _HighestCount = 0; 
		m_InfoElement _ElementFound;

		for (const m_InfoElement _IterationElement : m_InfoElements)
		{
			if (_IterationElement.m_CountForCurrentLayout > _HighestCount
				&& _IterationElement.m_Layout == _Element.m_Layout)
			{
				_HighestCount = _IterationElement.m_CountForCurrentLayout;
				_ElementFound = _IterationElement;
			}
		}
		
		_sy = (_ElementBB.gSize().y);
		_sx = (_InfoBB.gSize().x);
		_sx -= std::abs((g_InfoElementPushX(_ElementFound)));
		_sx -= g_InfoElementBB(_ElementFound).gSize().x; 
		_sx -= _ElementBB.gSize().x;

		//cr_List._Debug->LogTaggedInt(__FUNCTION__, g_InfoElementPushX(_ElementFound));

		if (_Element.m_Layout ==
			m_InfoElement::m_LayoutPositions::m_FromLeft
			)
		{ /* Comment to right */
			_x = _InfoBB.gFinal().x - _sx;
		}
		else
		{ /* Comment to left */
			_x = _InfoBB.gStarting().x;
		}

		_y = _ElementBB.gStarting().y;
		/* Get _fx & _sx */
		_fx = (_x + _sx);
		_fy = (_y + _sy);

		/* Return */
		return { wxPoint { _x, _y}, wxPoint {_fx, _fy} };
	}

	const int TextEditor::g_InfoElementPushX(
		const m_InfoElement& _Element)
	{
		int _PushX = 0;
		wxMemoryDC _MemoryCanvas;
		_MemoryCanvas.SetFont(cr_List._LayoutInfoFont);

		const int _InfoCharSizeX = _MemoryCanvas.GetTextExtent("a").x;
		/* Iterate to determinate PushX */
		for (const m_InfoElement& _CurrentElement :
			m_InfoElements)
		{ /* Check if needed to loop */
			if (_Element.m_CountForCurrentLayout
				== 0) break;
			/* Check for layout & count */
			if (_CurrentElement.m_Layout == _Element.m_Layout &&
				_CurrentElement.m_CountForCurrentLayout < _Element.m_CountForCurrentLayout
				)
			{ /* Increment value */
				_PushX += ((_InfoCharSizeX * _CurrentElement.m_Text.length()) + cr_List._LayoutInfoElementSpacingX);
			}
		}
		/* Check for layout integrity */
		if (_Element.m_Layout ==
			m_InfoElement::m_LayoutPositions::m_FromRight)
		{ /* Reverse _PushX */
			_PushX = -_PushX;
		}

		return _PushX;
	}

	const wxPoint TextEditor::g_InfoElementCommentCenteredTextPosition(
		const m_InfoElement& _Element)
	{ /* Calculate & Return */
		Framework::Geometry::BoundingBox _CommentBB = g_InfoElementCommentBB(_Element);
		const wxPoint _CommentBBStarting = _CommentBB.gStarting();

		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutInfoFont);

		const int _CharSizeX = _MemoryCanvas.GetTextExtent("a").x;
		/* Calculate */
			int _x, _y;

			_y = _CommentBBStarting.y; 
			_x = ((_Element.m_Layout == m_InfoElement::m_FromRight) ?
				_CommentBBStarting.x :
				_CommentBB.gFinal().x
				);

			_y += cr_List._LayoutInfoSpacingY / 2; 

			_x += ((_Element.m_Layout == m_InfoElement::m_FromLeft) ?
				(-cr_List._LayoutInfoElementCommentSpacingX - (_CharSizeX * _Element.m_Comment.length())) :
				cr_List._LayoutInfoElementCommentSpacingX
				);

					
		return wxPoint { _x, _y };
	} 

	Framework::Geometry::BoundingBox TextEditor::g_InfoElementBB(
		const m_InfoElement& _Element)
	{ /* Calculate & Return */
		/* _Push X */
		int _PushX = g_InfoElementPushX(_Element);
		wxMemoryDC _MemoryCanvas; 
		_MemoryCanvas.SetFont(cr_List._LayoutInfoFont);

		const int _InfoCharSizeX = _MemoryCanvas.GetTextExtent("a").x;

		_LayoutPositions _Layout = g_LayoutPositions(); 

		Framework::Geometry::BoundingBox _InfoBB = _Layout.g_BBFromPackedArray(
			_Layout._InfoBB);
		const wxPoint _InfoBBStarting = _InfoBB.gStarting();
		const wxPoint _InfoBBFinal = _InfoBB.gFinal();

		bool _LayoutFromLeft = (_Element.m_Layout == m_InfoElement::m_LayoutPositions::m_FromLeft);
		/* Calculate */ 
			int _x, _y, _fx, _fy, _sx;

			_sx = (cr_List._LayoutInfoElementSpacingX + (_InfoCharSizeX * _Element.m_Text.length()));

			_x = (_LayoutFromLeft ? _InfoBBStarting.x : _InfoBBFinal.x - _sx) + _PushX;
			_y = (_InfoBBStarting.y + cr_List._LayoutInfoSeparatorY);
			
			_fx = (_x + _sx);
			_fy = _InfoBBFinal.y;
		/* Return */
		return { wxPoint { _x, _y }, wxPoint { _fx, _fy} };
	}

	Framework::Geometry::BoundingBox TextEditor::g_InfoSeparatorBB()
	{ /* Calculate & Return */
		_LayoutPositions _Layout = g_LayoutPositions();

		Framework::Geometry::BoundingBox _InfoBB = _Layout.g_BBFromPackedArray(
			_Layout._InfoBB);
		const wxPoint _InfoBBStarting = _InfoBB.gStarting();
		/* Calculate */
			int _x, _y, _fx, _fy;

			_x = _InfoBBStarting.x;
			_y = _InfoBBStarting.y;

			_fx = _x; 
			_fy = _y;

			_fy += cr_List._LayoutInfoSeparatorY;
			_fx += _InfoBB.gSize().x;
		/* Return */
		return { wxPoint { _x, _y }, wxPoint { _fx, _fy } };
	}

	/* [=============================== _MouseMotionHook ===============================] */
	void TextEditor::init_MouseMotionHook()
	{
		Bind(wxEVT_MOTION, &TextEditor::hk_MouseMotion, this);
		Bind(wxEVT_LEAVE_WINDOW, &TextEditor::hk_MouseMotion, this);
	}

	void TextEditor::hk_MouseMotion(
		wxMouseEvent& _Event)
	{ /* Main _MouseMotion hook for this class */
		try 
		{
			hk_InfoActivateElement(_Event);
		} catch (const std::exception& _ex) { cr_List._Debug->log_TaggedStdException(
			__FUNCTION__, _ex
		); }
	}

	/* [=============================== _OnExit ===============================] */
	void TextEditor::init_OnExit()
	{
		//GetParent()->Bind(wxEVT_CLOSE_WINDOW, &TextEditor::hk_OnExit, this);
	}

	void TextEditor::hk_OnExit(
		wxCloseEvent& _Event)
	{ 
		//wxLogMessage("destructor called");

		//delete this;
	}
