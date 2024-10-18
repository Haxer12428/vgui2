#pragma once
#include <wx/wx.h>
#include "ProgramSlider.h"
#include "ProgramBuffer.h"
#include <string>
#include <regex>
#include "../Framework/Framework.h"
#include "TextEditor/TextEditor.h"

class ProgramMainPanel : public wxPanel
{ public: 
	/*
		Program Main Panel Included:
			Tab selector (gui)
			Tab handler 
	*/
	ProgramMainPanel(
		wxFrame* _PARENT, Framework::Debug* _DEBUG,
		const wxPoint& _POSITION, const wxSize& _SIZE
	);

private:
};