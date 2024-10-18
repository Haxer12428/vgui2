#include "ProgramMainPanel.h"

ProgramMainPanel::ProgramMainPanel(
	wxFrame* _PARENT, Framework::Debug* _DEBUG,
	const wxPoint& _POSITION, const wxSize& _SIZE
) :
	wxPanel(_PARENT, wxID_ANY, _POSITION, _SIZE)
{
	wxFont _font = wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	_font.SetFaceName("Menlo");

	wxFont _font2 = wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	_font2.SetFaceName("Menlo");

	//_DEBUG->LogTaggedInt("y", this->GetPosition().y);

	TextEditor::_CoreList _CoreList;
	_CoreList._Parent = this;
	_CoreList._Debug = _DEBUG;
	_CoreList._Position = wxPoint(0, 0);
	_CoreList._Size = this->GetSize();
	_CoreList._LayoutBackgroundColor = wxColor(41, 44, 52);
	_CoreList._LayoutLinesIndexingFont = _font;
	_CoreList._LayoutLineIndexingFontColorDeActivated = wxColor(77, 83, 100);
	_CoreList._LayoutLineIndexingFontColorActivated = wxColor(111, 116, 126);
	_CoreList._LayoutLineIndexingBackgroundSelectionColor = wxColor(63, 68, 81);
	_CoreList._LayoutLineIndexingBackgroundRegularColor = _CoreList._LayoutBackgroundColor;
	_CoreList._LineIndexingActivatedLines = { 0 };
	_CoreList._Buffer.push_back("123");
	_CoreList._Buffer.push_back("nigger");
	_CoreList._Buffer = Framework::File::Open("C:\\Users\\Survi\\source\\repos\\vgui2\\vgui2\\Program\\TextEditor\\TextEditor.cpp").Read().gVector();
	_CoreList.m_BufferSpacingY = _CoreList.m_LineIndexingSpacingY;
	_CoreList._LayoutBufferFont = _font2;
	_CoreList._LayoutBufferBackgroundColor = _CoreList._LayoutBackgroundColor;
	_CoreList._LayoutBufferFontColor = wxColor(193, 198, 211);

	

	_CoreList._LayoutScrollbarsElementColorDeActivated = wxColor(77, 83, 98);
	_CoreList._LayoutScrollbarsBackgroundColor = _CoreList._LayoutBackgroundColor;
	_CoreList._LayoutBufferActivatedLineColor = wxColor(45, 49, 60);
	_CoreList._LayoutCursorOutlineColor = wxColor(41, 44, 52);
	_CoreList._LayoutCursorColor = wxColor(101, 140, 253);

	TextEditor* _Editor = new TextEditor(_CoreList);
}
