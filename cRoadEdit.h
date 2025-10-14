#pragma once
#include "cMain.h"

#define BTNCLOSE_RE		15000
#define BTNCREATE_RE	15001
#define BTNDELETE_RE	15002

#define INPUTCOORDX_RE	16000
#define INPUTCOORDZ_RE	16001
#define INPUTCOORDY_RE	16002
#define INPUTROT_RE		16003

#define LBROADS_RE		17000
#define LBROADFRAME_RE	17001


class cRoadEdit : public wxFrame
{
public:
	cRoadEdit(cMain* ref);

	class cAddRoad : public wxFrame
	{

	public:
		cAddRoad(cRoadEdit* roadRef)
			: wxFrame(roadRef, 21000, "Add new transition", wxDefaultPosition, wxSize(508, 250), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX) | wxFRAME_FLOAT_ON_PARENT), roadRef(roadRef)
		{
			roadRef->Disable();
			Centre();
			this->SetBackgroundColour(wxColor(170, 170, 255));

			wxStaticText* label_FromScene = new wxStaticText(this, wxID_ANY, "From Scene:", wxPoint(10, 10));
			LB_fromScene = new wxListBox(this, 30000, wxPoint(10, 30), wxSize(220, 50));
			LB_fromSceneView = new wxListBox(this, 30001, wxPoint(10, 80), wxSize(220, 70));

			wxStaticText* label_ToScene = new wxStaticText(this, wxID_ANY, "To Scene:", wxPoint(256, 10));
			LB_toScene = new wxListBox(this, 30002, wxPoint(256, 30), wxSize(220, 50));
			LB_toSceneView = new wxListBox(this, 30003, wxPoint(256, 80), wxSize(220, 70));

			d_btnConfirm = new wxButton(this, 30004, "Confirm", wxPoint(20, 160));
			d_btnCancel = new wxButton(this, 30005, "Cancel", wxPoint(256, 160));

			// read in scenes
			for (int32_t scene = 0; scene < roadRef->refToMain->dfedit->SETref->getSETsceneCount(); scene++) {

				wxString sceneName(roadRef->refToMain->dfedit->SETref->getSETSceneName(scene));
				LB_fromScene->AppendString(sceneName);
				LB_toScene->AppendString(sceneName);
			}
		}
		~cAddRoad() {
			delete label_FromScene;
			delete label_ToScene;

			delete LB_fromScene;
			delete LB_fromSceneView;
			delete LB_toScene;
			delete LB_toSceneView;
			delete d_btnConfirm;
			delete d_btnCancel;
		}



	private:
		cRoadEdit* roadRef;

		wxStaticText* label_FromScene = nullptr;
		wxStaticText* label_ToScene = nullptr;

		wxListBox* LB_fromScene = nullptr;
		wxListBox* LB_fromSceneView = nullptr;
		wxListBox* LB_toScene = nullptr;
		wxListBox* LB_toSceneView = nullptr;
		// buttons
		wxButton* d_btnConfirm = nullptr;
		wxButton* d_btnCancel = nullptr;

		void OnClose(wxCommandEvent& evt) {
			roadRef->Enable(true);
			Close();
		}
		void OnClose(wxCloseEvent& evt) {
			// make sure the parent will be reactivated before closing
			roadRef->Enable(true);
			evt.Skip();
		}

		void refreshViewData(int32_t scene, wxListBox* viewLB) {
			viewLB->Clear();
			for (int32_t view = 0; view < roadRef->refToMain->dfedit->SETref->getSETsceneViewCount(scene); view++) {
				viewLB->AppendString(roadRef->refToMain->dfedit->SETref->getSETsceneViewName(scene, view));
			}
		}

		void OnFromSceneChange(wxCommandEvent& evt) {
			refreshViewData(evt.GetSelection(), LB_fromSceneView);
		}

		void OnToSceneChange(wxCommandEvent& evt) {
			refreshViewData(evt.GetSelection(), LB_toSceneView);
		}

		void OnConfirm(wxCommandEvent& evt) {
			int32_t fromScene = LB_fromScene->GetSelection();
			int32_t toScene = LB_toScene->GetSelection();
			int32_t fromView = LB_fromSceneView->GetSelection();
			int32_t toView = LB_toSceneView->GetSelection();

			if (fromScene == -1 ||
				toScene == -1 ||
				fromView == -1 ||
				toView == -1) {
				wxMessageBox("You must select scenes and views first!", "No Selection", wxICON_INFORMATION | wxOK);
				return;
			}

			if (fromScene == toScene) {
				wxMessageBox("You can not connect a scene with it self!", "Same Scenes", wxICON_INFORMATION | wxOK);
				return;
			}

			roadRef->roadsLB->AppendString(roadRef->refToMain->dfedit->SETref->addSETroad(fromScene, fromView, toScene, toView));
			roadRef->refToMain->refreshTree();
			roadRef->refToMain->Refresh();
			Close();
		}

		wxDECLARE_EVENT_TABLE();
	};

	wxListBox* roadsLB = nullptr;

private:

	cMain* refToMain;
	cAddRoad* addRoad = nullptr;

	wxButton* m_btnClose = nullptr;
	wxButton* m_btnCreate = nullptr;
	wxButton* m_btnDelete = nullptr;

	
	wxListBox* roadFrameLB = nullptr;

	wxTextCtrl* inputX = nullptr;
	wxTextCtrl* inputZ = nullptr;
	wxTextCtrl* inputY = nullptr;
	wxTextCtrl* rotation = nullptr;

	void OnClose(wxCommandEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void OnCreateRoad(wxCommandEvent& evt);
	void OnDeleteRoad(wxCommandEvent& evt);

	void refreshData(wxShowEvent& event);
	void OnSRoadSelectionChange(wxCommandEvent& evt);
	void OnSRoadFrameSelectionChange(wxCommandEvent& evt);
	void refreshRoadLB();
	void refreshRoadFrameLB();

	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(cRoadEdit::cAddRoad, wxFrame)
EVT_BUTTON(30005, cRoadEdit::cAddRoad::OnClose)
EVT_BUTTON(30004, cRoadEdit::cAddRoad::OnConfirm)
EVT_CLOSE(cRoadEdit::cAddRoad::OnClose)
EVT_LISTBOX(30000, cRoadEdit::cAddRoad::OnFromSceneChange)
EVT_LISTBOX(30002, cRoadEdit::cAddRoad::OnToSceneChange)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(cRoadEdit, wxFrame)
EVT_SHOW(refreshData)
EVT_CLOSE(cRoadEdit::OnClose)
EVT_LISTBOX(LBROADS_RE, cRoadEdit::OnSRoadSelectionChange)
EVT_LISTBOX(LBROADFRAME_RE, cRoadEdit::OnSRoadFrameSelectionChange)
EVT_BUTTON(BTNCLOSE_RE, cRoadEdit::OnClose)
EVT_BUTTON(BTNCREATE_RE, cRoadEdit::OnCreateRoad)
EVT_BUTTON(BTNDELETE_RE, cRoadEdit::OnDeleteRoad)
wxEND_EVENT_TABLE()

cRoadEdit::cRoadEdit(cMain* ref)
	: wxFrame(ref, ROADEDITWIND, "DF Road editor", wxPoint(400, 400), wxSize(800, 600), wxDEFAULT_FRAME_STYLE), refToMain(ref) {

	this->SetBackgroundColour(wxColor(170, 170, 255));

	m_btnClose = new wxButton(this, BTNCLOSE_RE, "Close");
	m_btnCreate = new wxButton(this, BTNCREATE_RE, "New Road");
	m_btnDelete = new wxButton(this, BTNDELETE_RE, "Delete Road");

	wxStaticText* labelRoads = new wxStaticText(this, wxID_ANY, "Available connections:");
	roadsLB = new wxListBox(this, LBROADS_RE);
	wxStaticText* labelRoadFrames = new wxStaticText(this, wxID_ANY, "Frames:");
	roadFrameLB = new wxListBox(this, LBROADFRAME_RE);

	wxStaticText* labelX = new wxStaticText(this, wxID_ANY, "X axis:");
	wxStaticText* labelZ = new wxStaticText(this, wxID_ANY, "Z axis:");
	wxStaticText* labelY = new wxStaticText(this, wxID_ANY, "Y axis:");
	wxStaticText* labelrot = new wxStaticText(this, wxID_ANY, "Rotation:");

	inputX = new wxTextCtrl(this, INPUTCOORDX_RE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	inputZ = new wxTextCtrl(this, INPUTCOORDZ_RE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	inputY = new wxTextCtrl(this, INPUTCOORDY_RE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	rotation = new wxTextCtrl(this, INPUTROT_RE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);


	wxBoxSizer* hbBtns = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* vbCtrl = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vbsCoordLabels = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vbsCoords = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* hbScreen = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbsCoords = new wxBoxSizer(wxHORIZONTAL);

	vbsCoordLabels->Add(labelX, 1);
	vbsCoordLabels->Add(labelZ, 1);
	vbsCoordLabels->Add(labelY, 1);
	vbsCoordLabels->Add(labelrot, 1);

	vbsCoords->Add(inputX, 1, wxEXPAND);
	vbsCoords->Add(inputZ, 1, wxEXPAND);
	vbsCoords->Add(inputY, 1, wxEXPAND);
	vbsCoords->Add(rotation, 1, wxEXPAND);

	hbsCoords->Add(vbsCoordLabels, 1, wxEXPAND | wxALL, 2);
	hbsCoords->Add(vbsCoords, 1, wxEXPAND | wxALL, 2);

	hbBtns->Add(m_btnCreate, 1, wxEXPAND | wxALL, 2);
	hbBtns->Add(m_btnDelete, 1, wxEXPAND | wxALL, 2);

	vbCtrl->Add(hbBtns, 0, wxEXPAND | wxALL, 2);
	vbCtrl->Add(labelRoads);
	vbCtrl->Add(roadsLB, 1, wxEXPAND | wxALL, 2);
	vbCtrl->Add(labelRoadFrames);
	vbCtrl->Add(roadFrameLB, 1, wxEXPAND | wxALL, 2);
	vbCtrl->Add(hbsCoords);
	vbCtrl->Add(m_btnClose, 0, wxEXPAND | wxALL, 2);

	hbScreen->AddSpacer(512);
	hbScreen->AddStretchSpacer(1);
	//hbs->Add(imagePanel, 1, wxEXPAND);
	hbScreen->Add(vbCtrl, 0, wxEXPAND | wxALIGN_TOP);

	this->SetSizerAndFit(hbScreen);
}

void cRoadEdit::OnClose(wxCommandEvent& evt) {
	this->Hide();
	evt.Skip();
}

void cRoadEdit::OnClose(wxCloseEvent& evt) {
	this->Hide();
	evt.Veto();
}

void cRoadEdit::refreshData(wxShowEvent& event) {
	refreshRoadLB();
	refreshRoadFrameLB();
}

void cRoadEdit::refreshRoadLB() {
	roadsLB->Clear();

	int32_t rdCount = refToMain->dfedit->SETref->getSETRoadCount();
	for (int32_t road = 0; road < rdCount; road++) {
		roadsLB->AppendString(refToMain->dfedit->SETref->getSETRoadName(road));
	}

	if (rdCount) {
		roadsLB->SetSelection(0);
	}
}

void cRoadEdit::refreshRoadFrameLB() {
	roadFrameLB->Clear();

	int32_t sel = roadsLB->GetSelection();
	if (sel == -1) return;
	int32_t frameCount = refToMain->dfedit->SETref->getSETRoadFrameCount(sel);
	if (!frameCount) return;
	for (int32_t frame = 0; frame < frameCount; frame++) {
		wxString name("Frame ");
		name.append(std::to_string(frame+1));
		roadFrameLB->AppendString(name);


	}
	roadFrameLB->SetSelection(0);
}

void cRoadEdit::OnSRoadSelectionChange(wxCommandEvent& evt) {
	refreshRoadFrameLB();

	refToMain->m_console->AppendText(refToMain->dfedit->SETref->printSETroad(roadsLB->GetSelection()));

	evt.Skip();
}

void cRoadEdit::OnSRoadFrameSelectionChange(wxCommandEvent& evt) {
	int32_t road = roadsLB->GetSelection();
	int32_t frame = roadFrameLB->GetSelection();

	if (road == -1 || frame == -1) return;
	double x, z, y, rot;
	refToMain->dfedit->SETref->getSETroadFrameCoords(road, frame, x, z, y, rot);

	inputX->SetValue(std::to_string(x));
	inputZ->SetValue(std::to_string(z));
	inputY->SetValue(std::to_string(y));
	rotation->SetValue(std::to_string(rot*180/PI));

	evt.Skip();
}

void cRoadEdit::OnCreateRoad(wxCommandEvent& evt) {
	addRoad = new cAddRoad(this);
	addRoad->Show();
}

inline void cRoadEdit::OnDeleteRoad(wxCommandEvent& evt)
{
	int32_t sel = roadsLB->GetSelection();
	if (sel == -1) {
		wxMessageBox("You must select a road first!", "No Selection", wxICON_INFORMATION | wxOK);
		evt.Skip();
		return;
	}

	refToMain->dfedit->SETref->removeSETRoad(sel);
	refToMain->refreshTree();
	refToMain->Refresh();

	refreshRoadLB();
	
	evt.Skip();
}
