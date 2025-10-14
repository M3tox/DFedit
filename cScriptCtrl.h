#pragma once
#include "cMain.h"

class cScrCtrl : public wxFrame
{
public:
	cScrCtrl(cMain* ref);

	wxTextCtrl* scriptWnd;
private:

	cMain* ref;
	bool fileNotSaved = false;

	void OnClose(wxCommandEvent& evt);
	void OnClose(wxCloseEvent& evt);

	void textChange(wxCommandEvent& evt);
	void OnSave(wxCommandEvent& evt);

	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(cScrCtrl, wxFrame)
EVT_BUTTON(11000, cScrCtrl::OnClose)
EVT_BUTTON(11002, cScrCtrl::OnSave)
EVT_TEXT(11001, textChange)
EVT_CLOSE(cScrCtrl::OnClose)
wxEND_EVENT_TABLE()

cScrCtrl::cScrCtrl(cMain* ref) 
	: wxFrame(ref, SCRIPTEDITWIND, "DF Script Editor", wxPoint(400, 400), wxSize(800, 600), wxDEFAULT_FRAME_STYLE), ref(ref) {

	this->SetBackgroundColour(wxColor(170, 170, 255));

	scriptWnd = new wxTextCtrl(this, 11001, "", wxPoint(0, 0), wxSize(400, 300), wxTE_MULTILINE);
	scriptWnd->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	wxButton* m_btnClose = new wxButton(this, 11000, "Close");
	wxButton* m_btnSave = new wxButton(this, 11002, "Save");
	
	wxBoxSizer* vbs = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* hbs = new wxBoxSizer(wxHORIZONTAL);

	hbs->Add(m_btnSave, 0, wxALIGN_CENTER | wxALL, 2);
	hbs->Add(m_btnClose, 0, wxALIGN_CENTER | wxALL, 2);

	vbs->Add(scriptWnd, 1, wxEXPAND | wxALL, 2);
	vbs->Add(hbs, 0, wxALIGN_CENTER | wxALL, 2);

	this->SetSizerAndFit(vbs);
}

void cScrCtrl::OnClose(wxCloseEvent& evt)
{
	if (evt.CanVeto() && fileNotSaved)
	{
		if (wxMessageBox("The changes in the script were not saved! Continue closing?",
			"Warning", wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			evt.Veto();
			return;
		}
	}
	this->Hide();
	fileNotSaved = false;
	evt.Veto();
}

void cScrCtrl::OnClose(wxCommandEvent& evt)
{
	if (fileNotSaved) {
		if (wxMessageBox("The changes in the script were not saved! Continue closing?",
			"Warning", wxICON_QUESTION | wxYES_NO) != wxYES)
		{
			return;
		}
	}

	this->Hide();
	fileNotSaved = false;
	evt.Skip();
}

void cScrCtrl::OnSave(wxCommandEvent& evt) {
	if (!ref->dfedit->SETref->changeSETscript(ref->dfedit->lastSelectedContainer, scriptWnd->GetValue().ToStdString())) {
		wxMessageBox("Unknown problem occured. Could not save Script!",
			"Error", wxOK);
	};
	fileNotSaved = false;
	evt.Skip();
}

void cScrCtrl::textChange(wxCommandEvent& evt) {
	fileNotSaved = true;
	evt.Skip();
}
