#include "pch.h"
#include "cMain.h"
#include <Windows.h>
#include <Shellapi.h>
#include <fstream>

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_BUTTON(101, OnInjectButton)
EVT_BUTTON(102, OnSelectButton)
EVT_CHECKBOX(202, OnAutoCloseCheckBox)
wxEND_EVENT_TABLE()

DWORD StartMinecraft()
{
    HINSTANCE result = ShellExecuteW(
        NULL,
        L"open",
        L"minecraft:",
        NULL,
        NULL,
        SW_SHOW
    );

    if ((INT_PTR)result <= 32)
    {
        return 0;
    }

    Sleep(3000);

    return GetProcId("Minecraft.Windows.exe");
}
cMain::cMain()
    : wxFrame(
        nullptr,
        wxID_ANY,
        "Strength Injector",
        wxDefaultPosition,
        wxSize(297.5, 162.5),
        wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN
    )
{
    if (!this->cfg.serializeConfig())
    {
        this->cfg.updateConfigFile();
    }
    wxIcon icon(icon_xpm);
    this->SetIcon(icon);
    this->SetBackgroundColour(wxColour(253, 253, 253, 253));
    this->mainPanel = new wxPanel(this, wxID_ANY);
    this->btn_Inject = new wxButton(
        this->mainPanel,
        101,
        "Launch",
        wxPoint(5, 9),
        wxSize(100, 48)
    );
    this->btn_Select = new wxButton(
        this->mainPanel,
        102,
        "Browse",
        wxPoint(5, 66),
        wxSize(60, 25)
    );
    this->txt_ProcName = new wxTextCtrl(
        this->mainPanel,
        wxID_ANY,
        Globals::PROC_NAME,
        wxPoint(110, 10),
        wxSize(165, 23)
    );
    this->txt_ProcName->Disable();
    this->check_AutoClose = new wxCheckBox(
        this->mainPanel,
        202,
        "Close after injection",
        wxPoint(115, 40),
        wxSize(165, 20)
    );
    this->txt_DllPath = new wxTextCtrl(
        this->mainPanel,
        wxID_ANY,
        (
            Globals::DLL_PATH.empty()
            ? Globals::NO_DLL_PATH_SELECTED_MSG
            : Globals::DLL_PATH
            ),
        wxPoint(70, 67),
        wxSize(205, 23)
    );
    this->CreateStatusBar(1);
    this->SetStatusText("Modified by @Streoids", 0);
    this->openDialog = new wxFileDialog(
        this,
        "Select the DLL",
        Globals::WORKING_DIR,
        "*.dll",
        "Dynamic link library (*.dll)|*.dll",
        wxFD_OPEN
    );
}
cMain::~cMain()
{
    this->openDialog->Destroy();
}
void cMain::updateGlobalVars()
{
    static const std::wstring EMPTY_WSTR = std::wstring{};
    auto pathStr = this->txt_DllPath->GetValue();
    Globals::DLL_PATH =
        (
            (pathStr == Globals::NO_DLL_PATH_SELECTED_MSG)
            ? EMPTY_WSTR
            : pathStr
            );
    Globals::PROC_NAME = this->txt_ProcName->GetValue();
}
void cMain::OnInjectButton(wxCommandEvent& evt)
{
    cMain::OnInjectButtonExecute(evt, *this);
}
void cMain::OnInjectButtonExecute(wxCommandEvent& evt, cMain& ref)
{
    DWORD procId = GetProcId("Minecraft.Windows.exe");
    if (procId == 0)
    {
        ref.SetStatusText("Launching Minecraft...", 0);
        procId = StartMinecraft();
        if (procId == 0)
        {
            ref.SetStatusText("Failed to start Minecraft", 0);
            return;
        }
    }
    wxString wxStrPath = ref.txt_DllPath->GetValue();
    std::wstring wStrPath = wxStrPath.ToStdWstring();
    std::ifstream test(wStrPath.c_str());
    if (!test)
    {
        ref.SetStatusText("Invalid dll path", 0);
        return;
    }
    ref.SetStatusText("Injecting DLL...", 0);
    SetAccessControl(wStrPath, L"S-1-15-2-1");
    performInjection(procId, wStrPath.c_str());
    ref.SetStatusText("Success!", 0);
    if (ref.check_AutoClose->IsChecked())
    {
        Sleep(1000);
        ref.Close(true);
    }
    ref.updateGlobalVars();
    ref.cfg.updateConfigFile();
    evt.Skip();
}
void cMain::OnSelectButton(wxCommandEvent& evt)
{
    if (this->openDialog->ShowModal() == wxID_OK)
    {
        this->txt_DllPath->SetLabel(this->openDialog->GetPath());
    }
    this->updateGlobalVars();
    this->cfg.updateConfigFile();

    evt.Skip();
}
void cMain::OnAutoCloseCheckBox(wxCommandEvent& evt)
{
    evt.Skip();
}
