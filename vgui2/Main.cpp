#include "Program/ProgramCenter.h"

class ApplicationBootCenter
{ public:
	ApplicationBootCenter()
	{
		try
		{ // Run main element: 'Program Center'
			wxWindow* Window = new ProgramCenter("vgui2-experimental");
		}
		catch (
			const std::exception& ex) 
		{ wxLogMessage("APPLICATION_BOOT_CENTER : EXPECTING_ABORT"); 
			throw ex;
		};
	}
};

class ApplicationCenter : public
	wxApp
{ public:
	bool OnInit()
	{
		try {
			ApplicationBootCenter();
		} catch (
			const std::exception& 
			) {
			wxLogMessage("APPLICATION_CENTER : ABORT"); return false; 
		}

		return true; 
	}

};

wxIMPLEMENT_APP(ApplicationCenter);

