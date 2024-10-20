#pragma once
#include <wx/wx.h>
#include "ProgramTitleBar.h"
#include "ProgramResizeIcon.h"
#include "ProgramSlider.h"
#include "../Framework/Framework.h"
#include "ProgramMainPanel.h"


class ProgramCenter : public wxFrame
{ public:
	ProgramCenter(
		const std::string& Name
	);

private:
	ProgramMainPanel* m_MainPanel; 

/*
	On Startup
*/
	const void lConfig(
		const std::vector<std::vector<std::string>>& _DATA_SET, const std::filesystem::path& _PATH
	);


	const void lWindowDefParams();
	const void lDebug();
	const void lSizeHook();
	const void lTitleBar();
	const void lResizeIcon();

	void kEVT(wxKeyEvent& _EVT);
/*
	Config system
*/
	const Framework::Config::File::cValue gConfigParam(const std::string& Name);

	static const std::vector<std::vector<std::string>> ConfigElements;

/*
	Debug
*/
	const void DebugLog();
/*
	Hooks
*/
	void DebugLogHook(wxTimerEvent& _EVT);
	void SizeHook(wxSizeEvent& _EVT);
/*
	Sizers
*/
	const void sTitleBar();
	const void sResizeIcon();
	void s_MainPanel(); /* debug purposes */

/*
	Instances
*/
	wxTimer DebugLogHookTimer;
	Framework::Config::File* Config;
	Framework::Debug* _DEBUG = new Framework::Debug();

	ProgramTitleBar* TitleBarInstance;
	ProgramResizeIcon* ResizeIconInstance;
};
