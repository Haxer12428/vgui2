#include "ProgramCenter.h"

ProgramCenter::ProgramCenter(
	const std::string& Name
) : wxFrame(nullptr, wxID_ANY, Name, wxPoint(100,100), wxDefaultSize, wxNO_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX)
{
	// Load hooks 
	try
	{
		lConfig(
			ProgramCenter::ConfigElements, "C:\\vgui2\\config.cfg"
		);

		lDebug();
		lWindowDefParams();
		lTitleBar();
		lResizeIcon();
		lSizeHook();

		const wxSize _FIXED_SIZE = (
			GetSize() - Framework::Transform::WxPointToWxSize2D(TitleBarInstance->gProgramCenterFixedPosition())
			);
		
		const wxPoint _FIXED_POS = wxPoint(
			120, TitleBarInstance->GetSize().y 
		);

		const wxSize _SIZE_ACC = (_FIXED_SIZE - Framework::Transform::WxPointToWxSize2D(_FIXED_POS) - wxSize(0, 60));

		ProgramMainPanel* _PANEL = new ProgramMainPanel(this, _DEBUG, _FIXED_POS, _SIZE_ACC);
		_PANEL->SetFocus();
		SetFocus();
	} catch (const std::exception& ex) {
		// Throw error into LogMessage
		wxLogMessage(
			((std::string)"PROGRAM_CENTER: EXCEPTION OCCURED WHEN LOADING HOOKS: " + ex.what()).c_str()
		); // Refuse to continue with throwing exception, ApplicationBootCenter will handle it; 
		throw ex;
	}

	// Finalize with showing the program 
	this->Show(
		true);
}

const void ProgramCenter::lConfig(
	const std::vector<std::vector<std::string>>& _DATA_SET, const std::filesystem::path& _PATH
)
{
	/*
		Config system is stored at ProgramCenter::Config as an instance 
		Path is given in param and set in ProgramCenter::ProgramCenter 

		[ qConfigSetup & qConfigLoad ] 
	*/

	this->Config = new Framework::Config::File(_PATH);
	
	const bool _SETUP_STATUS = Config->qSetup(
		_DATA_SET, _DEBUG
	); 

	if (_SETUP_STATUS
		== true)
		return; 

	// Case: Error -> Throw exception to stop constructor & tell what happened
	throw std::exception("lCONFIG_ERROR_qSETUP"); 
}

const Framework::Config::File::cValue ProgramCenter::gConfigParam(
	const std::string& Name)
{
	/*
		Settings: 
			Name  : argv[1]
			Fix   : true
			Debug : class_instance
		From: this->Config
	*/
	return Config->gValue(
		Name, true, _DEBUG);
}

const void ProgramCenter::lWindowDefParams()
{
	/*
		Extract params from ConfigSystem & Load them (casually param value strictly from this
			function might exist)
		[ qWindowSetup ]
	*/

	SetSize(gConfigParam("BaseWindowSizeX").gInt(), gConfigParam("BaseWindowSizeY").gInt());
	SetDoubleBuffered(gConfigParam("BaseWindowDoubleBuffered").gBool());
	SetBackgroundColour(wxColor( gConfigParam("BaseWindowBGCR").gInt(), gConfigParam("BaseWindowBGCG").gInt(), gConfigParam("BaseWindowBGCB").gInt() ));
}

const void ProgramCenter::lDebug()
{
	/*
		Setup timer: [time: config system: 'ProgramCenterDebugCheckMs'] 
	*/
	DebugLogHookTimer.SetOwner(this, 1);
	DebugLogHookTimer.Start(gConfigParam("ProgramCenterDebugCheckMs").gInt());

	// Bind 'function responsible for handling debug hook' to timer with given wxID 
	this->Bind(wxEVT_TIMER,
		&ProgramCenter::DebugLogHook, this, DebugLogHookTimer.GetId());
}

void ProgramCenter::DebugLogHook(
	wxTimerEvent& _EVT)
{
	/*
		Call 'this->LogDebug' function each time 
	*/
	this->DebugLog();
}

const void ProgramCenter::DebugLog()
{
	/*
		Log debug to wxLogMessage each time function is triggered, after: _DEBUG -> _GC 
	*/
	const std::vector<std::string> _LOGS = _DEBUG->Get();

	if (_LOGS.empty() == // Return if Empty 
		true) return;

	for ( // Perform a loop and log each _MESSAGE
		const std::string& _MESSAGE : _LOGS
		) wxLogMessage(_MESSAGE.c_str());
	// Perform _GARBAGE_COLLECTION of logs already printed
	_DEBUG->Clear();
}

const void ProgramCenter::lSizeHook()
{
	Bind(wxEVT_SIZE, &ProgramCenter::SizeHook, this);
}

const void ProgramCenter::lTitleBar()
{
	TitleBarInstance = new ProgramTitleBar(_DEBUG, Config, this, "vterminal-gui");
}

const void ProgramCenter::lResizeIcon()
{
	ResizeIconInstance = new ProgramResizeIcon(this, _DEBUG, Config,
		wxColor(gConfigParam("BaseWindowBGCR").gInt(), gConfigParam("BaseWindowBGCG").gInt(), gConfigParam("BaseWindowBGCB").gInt())
	);
}

const void ProgramCenter::sTitleBar()
{
	TitleBarInstance->exSizeHook();
}

const void ProgramCenter::sResizeIcon()
{
	ResizeIconInstance->sSelf();
}

void ProgramCenter::SizeHook(
	wxSizeEvent& _EVT
) {
	/*
		External sizers 
	*/
		sTitleBar(); 
		sResizeIcon();

	// Finalize with refresh to main window 
	Refresh();
}