#pragma once
#include <wx/wx.h>
#include "ProgramSlider.h"
#include "ProgramBuffer.h"
#include <string>
#include <regex>
#include <unordered_map>
#include "../Framework/Framework.h"
// Custom color format example: "\aHEXCODE"
#define RESET   "\\aRESET"  // You can define a reset sequence if needed
#define RED     "\\aFF0000" // Red color in hex
#define GREEN   "\\a00FF00" // Green color in hex
#define CYAN    "\\a00FFFF" // Cyan color in hex
#define _RESET  "\\aDEFCLR"


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
	ProgramBuffer* _BUFFER;
};