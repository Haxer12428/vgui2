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
	
	wxFont _font3 = wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	_font3.SetFaceName("Menlo");

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
	_CoreList._Buffer = Framework::File::Open("C:\\vgui2\\TextEditor.cpp").Read().gVector();

	_CoreList.m_BufferSpacingY = _CoreList.m_LineIndexingSpacingY;
	_CoreList._LayoutBufferFont = _font2;
	_CoreList._LayoutBufferBackgroundColor = _CoreList._LayoutBackgroundColor;
	_CoreList._LayoutBufferFontColor = wxColor(193, 198, 211);

	_CoreList._LayoutScrollbarsElementColorDeActivated = wxColor(77, 83, 98);
	_CoreList._LayoutScrollbarsBackgroundColor = _CoreList._LayoutBackgroundColor;
	_CoreList._LayoutBufferActivatedLineColor = wxColor(45, 49, 60);
	_CoreList._LayoutCursorOutlineColor = wxColor(41, 44, 52);
	_CoreList._LayoutCursorColor = wxColor(101, 140, 253);

	_CoreList._LayoutBufferChunkColor = wxColor(120, 120, 120);
	_CoreList._LayoutBufferChunkOutlineColor = wxColor(10, 10, 10);

	_CoreList._LayoutInfoFont = _font3;
	_CoreList._LayoutInfoSpacingY = 8;

	_CoreList._LayoutInfoBackgroundColor = wxColor(34, 37, 43);
	_CoreList._LayoutInfoFontColor = wxColor(159, 165, 180);
	_CoreList._LayoutInfoSeparatorColor = wxColor(24, 26, 31);
	_CoreList._LayoutInfoSeparatorY = 1;
	_CoreList._LayoutInfoElementSpacingX = 14;
	_CoreList._LayoutInfoSpacingY = 8;

	_CoreList._Buffer = Framework::Vector::g_WithProperTabulation(_CoreList._Buffer, _CoreList._LayoutCursorTabLength);
	_CoreList._LayoutInfoElementCommentColor = wxColor(108, 157, 215);
	_CoreList._LayoutInfoElementActiveBackgroundColor = wxColor(48, 51, 61);

	m_TextEditor = new TextEditor(_CoreList);

	Bind(wxEVT_SIZE, &ProgramMainPanel::hk_OnSize, this);
}

void ProgramMainPanel::hk_OnSize(
	wxSizeEvent& _Event
) {
	//m_TextEditor->cr_List._Size = GetSize();
	//m_TextEditor->SetSize(GetSize());
	//m_TextEditor->Refresh();
}
