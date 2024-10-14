#include "ProgramMainPanel.h"

ProgramMainPanel::ProgramMainPanel(
	wxFrame* _PARENT, Framework::Debug* _DEBUG, 
	const wxPoint& _POSITION, const wxSize& _SIZE
) : 
	wxPanel(_PARENT, wxID_ANY, _POSITION, _SIZE)
{
    

	_BUFFER = new ProgramBuffer(this, _DEBUG, wxPoint(0, 0), this->GetSize(), wxColor(45, 51, 53), wxColor(37, 41, 42), wxColor(255, 255, 255));
    _BUFFER->SetFocus();

    const std::vector<
        std::vector<std::string>
    > _SYNTAX =
    {
        {"int ", "\\a00FF00"},
        {"const ", "\\a00FF00"},
        {".", "\\aFF0000"},
        {"bool ", "\\aFF0000"},
        {"\"", "\\a00FFFF"},
        {"<", "\\a00FFFF"},
        {">", "\\a00FFFF"}
    };
	
   
    _BUFFER->pNextLine("");

    std::unordered_map<std::string, std::string> colors = {
        {"keyword", RED},      // Keywords in red
        {"string", GREEN},     // Strings in green
        {"comment", CYAN}      // Comments in cyan
    };

	for (const std::string& _STR : Framework::File::Open(
        "C:\\Users\\Survi\\source\\repos\\vgui2\\vgui2\\Program\\ProgramBuffer.h"
        ).Read().gVector())
	{

        
       _BUFFER->pNextLine(_STR);
	}
   // _BUFFER->pNextLine("\\aFF0000niggers\\aFFFF00nddd\\aDEFCLR");

}

