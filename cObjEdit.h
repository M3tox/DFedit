#pragma once
#include "cMain.h"

#define MODESEL		11000

#define BTNCLOSE	12000
#define BTNTRANS	12001
#define BTNCREATE	12002
#define BTNRENAME	12005
#define BTNDELETE	12008

#define CBSCENES	13000
#define CBVIEWS		13001
#define CBOBJS		13002

#define INPUTCOORDX	14000
#define INPUTCOORDZ	14001
#define INPUTCOORDY	14002

#define INPUTROT	14003

#define INPUTSTARTX	14004
#define INPUTSTARTY	14005
#define INPUTENDX	14006
#define INPUTENDY	14007

class cObjEdit : public wxFrame
{
public:
	cObjEdit(cMain* ref);

	// small window that allows editing the transition frame count

	class cEdittrans : public wxFrame
	{

	public:
		cEdittrans(cObjEdit* objRef) 
			: wxFrame(objRef, 15002, "Edit transitional frames", wxDefaultPosition, wxSize(256, 230), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX) | wxFRAME_FLOAT_ON_PARENT), objRef(objRef)
		{
			objRef->Disable();
			Centre();
			this->SetBackgroundColour(wxColor(170, 170, 255));

			transCountCurrtxt = new wxStaticText(this, wxID_ANY, "Available spaces:", wxPoint(10, 10));
			viewLocations = new wxListBox(this, 20002, wxPoint(10, 30), wxSize(220, 50));
			
			transCountCurrtxt = new wxStaticText(this, wxID_ANY, "Current frame count:", wxPoint(10, 84));
			transitionsCount = new wxSlider(this, wxID_ANY,2,0,8, wxPoint(10, 100), wxSize(220, 30), wxSL_VALUE_LABEL);
			d_btnConfirm = new wxButton(this, 20000, "Confirm", wxPoint(20, 150));
			d_btnCancel = new wxButton(this, 20001, "Cancel", wxPoint(145, 150));

			// Update combobox and slider
			objRef->refToMain->dfedit->SETref->fillSETtransitionFrameInfos(viewLocations, FramesBetweenViews, objRef->sceneCB->GetCurrentSelection());

			if (!viewLocations->IsEmpty()) {
				viewLocations->Select(0);
				transitionsCount->SetValue(FramesBetweenViews.front());
			}
				
		}
		~cEdittrans() {
			delete viewLocations;
			delete transCountCurrtxt;
			delete transitionsCount;
			delete d_btnConfirm;
			delete d_btnCancel;
		}

	private:
		cObjEdit* objRef;

		wxListBox* viewLocations = nullptr;
		wxStaticText* transCountCurrtxt = nullptr;
		std::vector<int32_t>FramesBetweenViews;
		wxSlider* transitionsCount = nullptr;
		// buttons
		wxButton* d_btnConfirm = nullptr;
		wxButton* d_btnCancel = nullptr;

		void OnClose(wxCommandEvent& evt) {
			objRef->Enable(true);
			Close();
		}
		void OnClose(wxCloseEvent& evt) {
			// make sure the parent will be reactivated before closing
			objRef->Enable(true);
			evt.Skip();
		}
		void OnViewSelectioNChange(wxCommandEvent& evt) {
			transitionsCount->SetValue(FramesBetweenViews.at(evt.GetSelection()));
		}

		void OnConfirm(wxCommandEvent& evt) {
			int32_t frameDiff = FramesBetweenViews.at(viewLocations->GetSelection());
			frameDiff = transitionsCount->GetValue() - frameDiff;
			objRef->refToMain->dfedit->SETref->changeSETrotationFrameCount(viewLocations->GetSelection(), frameDiff, objRef->sceneCB->GetCurrentSelection());
			objRef->Enable(true);
			objRef->Refresh();
			objRef->refToMain->refreshTree();
			Close();
		}

		wxDECLARE_EVENT_TABLE();
	};

private:

	cMain* refToMain;

	wxChoice* modeSelector = nullptr;

	wxComboBox* sceneCB = nullptr;
	wxComboBox* viewCB = nullptr;
	wxComboBox* objCB = nullptr;

	bool sceneChg = false;
	bool viewChg = false;
	bool objChg = false;

	wxButton* m_btnCreate = nullptr;
	wxButton* m_btnRename = nullptr;
	wxButton* m_btnDelete = nullptr;

	wxButton* m_btnTransFrames = nullptr;

	wxCheckBox* c_doubleFrame = nullptr;

	wxTextCtrl* inputX = nullptr;
	wxTextCtrl* inputZ = nullptr;
	wxTextCtrl* inputY = nullptr;
	wxTextCtrl* inputRot = nullptr;

	wxTextCtrl* inputStartposX = nullptr;
	wxTextCtrl* inputStartposY = nullptr;
	wxTextCtrl* inputEndposX = nullptr;
	wxTextCtrl* inputEndposY = nullptr;

	wxBitmap currFrame;
	// little dialouge to change transition frames
	wxFrame* modifyTransFrame = nullptr;

	void OnPaint(wxPaintEvent& evt);

	void OnClose(wxCommandEvent& evt);
	void OnClose(wxCloseEvent& evt);

	void OnModeSelect(wxCommandEvent& evt);

	void OnCreate(wxCommandEvent& evt);
	void OnRename(wxCommandEvent& evt);
	void OnDelete(wxCommandEvent& evt);

	void OnSceneCBChange(wxCommandEvent& evt);
	void OnViewCBChange(wxCommandEvent& evt);
	void OnObjCBChange(wxCommandEvent& evt);

	void OnEditTransitions(wxCommandEvent& evt);

	void updateCoordinates(wxCommandEvent& evt);

	// utillities
	void checkDFedit(wxShowEvent& event);
	bool validateNewCBstring(wxComboBox* cb, const wxString& newStr);
	bool validateSelection(wxComboBox* cb);
	static void getCirclePosition(double rotation, int32_t length, int32_t& x, int32_t& y);
	bool updateViewImage();

	void updateClickRegion(wxCommandEvent& evt);

	void refreshSceneCB();
	void refreshViewCB();
	void refreshObjCB();

	void refreshObjText();

	wxDECLARE_EVENT_TABLE();
};

// event table for edit transition dialouge
wxBEGIN_EVENT_TABLE(cObjEdit::cEdittrans, wxFrame)
EVT_BUTTON(20000, cObjEdit::cEdittrans::OnConfirm)
EVT_BUTTON(20001, cObjEdit::cEdittrans::OnClose)
EVT_CLOSE(cObjEdit::cEdittrans::OnClose)
EVT_LISTBOX(20002, cObjEdit::cEdittrans::OnViewSelectioNChange)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(cObjEdit, wxFrame)
EVT_SHOW(checkDFedit)
EVT_CHOICE(MODESEL, OnModeSelect)
EVT_PAINT(OnPaint)
EVT_BUTTON(BTNCLOSE, cObjEdit::OnClose)
EVT_CLOSE(cObjEdit::OnClose)

EVT_BUTTON(BTNTRANS, cObjEdit::OnEditTransitions)
EVT_BUTTON(BTNCREATE, cObjEdit::OnCreate)
EVT_BUTTON(BTNRENAME, cObjEdit::OnRename)
EVT_BUTTON(BTNDELETE, cObjEdit::OnDelete)

EVT_TEXT_ENTER(INPUTCOORDX, cObjEdit::updateCoordinates)
EVT_TEXT_ENTER(INPUTCOORDZ, cObjEdit::updateCoordinates)
EVT_TEXT_ENTER(INPUTCOORDY, cObjEdit::updateCoordinates)

EVT_TEXT_ENTER(INPUTSTARTX, cObjEdit::updateClickRegion)
EVT_TEXT_ENTER(INPUTSTARTY, cObjEdit::updateClickRegion)
EVT_TEXT_ENTER(INPUTENDX, cObjEdit::updateClickRegion)
EVT_TEXT_ENTER(INPUTENDY, cObjEdit::updateClickRegion)

EVT_COMBOBOX(CBSCENES,cObjEdit::OnSceneCBChange)
EVT_COMBOBOX(CBVIEWS,cObjEdit::OnViewCBChange)
EVT_COMBOBOX(CBOBJS,cObjEdit::OnObjCBChange)
wxEND_EVENT_TABLE()

cObjEdit::cObjEdit(cMain* ref)
	: wxFrame(ref, SCENEEDITWIND, "DF Scene editor", wxPoint(400, 400), wxSize(800, 600), wxDEFAULT_FRAME_STYLE), refToMain(ref) {

	this->SetBackgroundColour(wxColor(170, 170, 255));

	wxString modeStrings[3]{
		"Scene mode",
		"View mode",
		"Object mode"
	};

	//imagePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(512, 264));

	(modeSelector = new wxChoice(this, MODESEL,wxDefaultPosition,wxDefaultSize, 3, modeStrings))->Select(0);

	sceneCB = new wxComboBox(this, CBSCENES);
	(viewCB = new wxComboBox(this, CBVIEWS))->Disable();
	(objCB = new wxComboBox(this, CBOBJS))->Disable();

	// for scenes
	wxStaticText* labelX = new wxStaticText(this, wxID_ANY, "X axis:");
	wxStaticText* labelZ = new wxStaticText(this, wxID_ANY, "Z axis:");
	wxStaticText* labelY = new wxStaticText(this, wxID_ANY, "Y axis:");
	// for views
	wxStaticText* labelrot = new wxStaticText(this, wxID_ANY, "Rotation:");

	// For objects
	wxStaticText* labelstartposX = new wxStaticText(this, wxID_ANY, "Start on X axis:");
	wxStaticText* labelstartposY = new wxStaticText(this, wxID_ANY, "Start on Y axis:");
	wxStaticText* labelendposX = new wxStaticText(this, wxID_ANY, "End on X axis:");
	wxStaticText* labelendposY = new wxStaticText(this, wxID_ANY, "End on Y axis:");

	// scene
	inputX = new wxTextCtrl(this, INPUTCOORDX,wxEmptyString,wxDefaultPosition,wxDefaultSize, wxTE_PROCESS_ENTER);
	inputZ = new wxTextCtrl(this, INPUTCOORDZ, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	inputY = new wxTextCtrl(this, INPUTCOORDY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	// For views
	(inputRot = new wxTextCtrl(this, INPUTROT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER))->Disable();
	c_doubleFrame = new wxCheckBox(this, wxID_ANY, "Use double views");
	// for objects
	(inputStartposX = new wxTextCtrl(this, INPUTSTARTX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER))->Disable();
	(inputStartposY = new wxTextCtrl(this, INPUTSTARTY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER))->Disable();
	(inputEndposX = new wxTextCtrl(this, INPUTENDX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER))->Disable();
	(inputEndposY = new wxTextCtrl(this, INPUTENDY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER))->Disable();
	
	m_btnTransFrames = new wxButton(this, BTNTRANS, "Modify transition frames");

	wxButton* m_btnClose = new wxButton(this, BTNCLOSE, "Close", wxDefaultPosition, wxSize(0, 30));

	// scene buttons
	m_btnCreate = new wxButton(this, BTNCREATE, "Create");
	m_btnRename = new wxButton(this, BTNRENAME, "Rename");
	m_btnDelete = new wxButton(this, BTNDELETE, "Delete");
	
	wxBoxSizer* vbs = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vbtnCtrl = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* hbs = new wxBoxSizer(wxHORIZONTAL);
	// scene coordinates
	wxBoxSizer* vbsCoordLabels = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vbsCoords = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* hbsCoords = new wxBoxSizer(wxHORIZONTAL);
	// view rotation
	wxBoxSizer* hbsRotation = new wxBoxSizer(wxHORIZONTAL);
	// screen position object
	wxBoxSizer* vbsObjPosLabels = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* vbsObjPos = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* hbsObjPos = new wxBoxSizer(wxHORIZONTAL);
	// frame control button and checkbox
	wxBoxSizer* hbsFrCtrl = new wxBoxSizer(wxHORIZONTAL);

	// Control buttons
	vbtnCtrl->Add(m_btnCreate, 1, wxEXPAND);
	vbtnCtrl->Add(m_btnRename, 1, wxEXPAND);
	vbtnCtrl->Add(m_btnDelete, 1, wxEXPAND);

	// coordinates
	vbsCoordLabels->Add(labelX, 1);
	vbsCoordLabels->Add(labelZ, 1);
	vbsCoordLabels->Add(labelY, 1);

	vbsCoords->Add(inputX, 1, wxEXPAND);
	vbsCoords->Add(inputZ, 1, wxEXPAND);
	vbsCoords->Add(inputY, 1, wxEXPAND);

	hbsCoords->Add(vbsCoordLabels, 1, wxEXPAND |wxALL, 2);
	hbsCoords->Add(vbsCoords, 1, wxEXPAND | wxALL, 2);

	// rotation
	hbsRotation->Add(labelrot, 1, wxEXPAND |wxALL, 2);
	hbsRotation->Add(inputRot, 1, wxEXPAND);

	vbsObjPosLabels->Add(labelstartposX, 1);
	vbsObjPosLabels->Add(labelstartposY, 1);
	vbsObjPosLabels->Add(labelendposX, 1);
	vbsObjPosLabels->Add(labelendposY, 1);

	vbsObjPos->Add(inputStartposX, 1, wxEXPAND);
	vbsObjPos->Add(inputStartposY, 1, wxEXPAND);
	vbsObjPos->Add(inputEndposX, 1, wxEXPAND);
	vbsObjPos->Add(inputEndposY, 1, wxEXPAND);

	hbsObjPos->Add(vbsObjPosLabels, 1, wxEXPAND | wxALL, 2);
	hbsObjPos->Add(vbsObjPos, 1, wxEXPAND | wxALL, 2);

	hbsFrCtrl->Add(c_doubleFrame, 1);
	hbsFrCtrl->Add(m_btnTransFrames, 1);
	


	vbs->Add(modeSelector, 0, wxEXPAND | wxALL, 2);
	vbs->Add(vbtnCtrl, 0, wxEXPAND | wxRIGHT | wxALL, 2);
	vbs->Add(sceneCB, 1, wxEXPAND | wxRIGHT | wxTOP | wxLEFT, 2);
	vbs->Add(hbsCoords, 0, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 2);
	vbs->Add(viewCB, 1, wxEXPAND | wxRIGHT | wxTOP | wxLEFT, 2);
	vbs->Add(hbsFrCtrl, 0, wxEXPAND | wxRIGHT | wxLEFT, 2);
	vbs->Add(hbsRotation, 0, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 2);
	vbs->Add(objCB, 1, wxEXPAND | wxRIGHT | wxBOTTOM | wxLEFT, 2);
	vbs->Add(hbsObjPos, 0, wxEXPAND);
	vbs->Add(m_btnClose, 0, wxALL | wxEXPAND, 2);
	hbs->AddSpacer(512);
	hbs->AddStretchSpacer(1);
	//hbs->Add(imagePanel, 1, wxEXPAND);
	hbs->Add(vbs, 0, wxEXPAND | wxALIGN_TOP);

	this->SetSizerAndFit(hbs);
}

inline void cObjEdit::checkDFedit(wxShowEvent& event)
{
	// dont do anything if window is up already
	if (!event.IsShown())
		return;


	// window is visible and DFedit is prepared
	// make sure mode 0 is selected
	modeSelector->SetSelection(0);

	viewCB->Disable();
	objCB->Disable();

	inputRot->Disable();
	inputStartposX->Disable();
	inputStartposY->Disable();
	inputEndposX->Disable();
	inputEndposY->Disable();
	m_btnTransFrames->Disable();

	// reset obj coords
	inputStartposX->Clear();
	inputStartposY->Clear();
	inputEndposX->Clear();
	inputEndposY->Clear();

	objChg = false;

	// load data
	refreshSceneCB();


	event.Skip();
	return;
}

inline void cObjEdit::OnModeSelect(wxCommandEvent& evt)
{
	uint8_t sel = evt.GetSelection();
	switch (sel) {
	case 0:
		// show only scenes
		viewCB->Disable();
		objCB->Disable();

		inputRot->Disable();
		inputStartposX->Disable();
		inputStartposY->Disable();
		inputEndposX->Disable();
		inputEndposY->Disable();
		m_btnTransFrames->Disable();

		// reset obj coords
		inputStartposX->Clear();
		inputStartposY->Clear();
		inputEndposX->Clear();
		inputEndposY->Clear();

		objChg = false;
		break;
	case 1:
		// show scenes and views
		viewCB->Enable();
		objCB->Disable();

		inputRot->Enable();
		inputStartposX->Disable();
		inputStartposY->Disable();
		inputEndposX->Disable();
		inputEndposY->Disable();
		m_btnTransFrames->Enable();

		// update view
		refreshViewCB();

		objChg = false;
		break;
	case 2:
		// show all: scenes, views and objects
		viewCB->Enable();
		objCB->Enable();

		inputRot->Enable();
		inputStartposX->Enable();
		inputStartposY->Enable();
		inputEndposX->Enable();
		inputEndposY->Enable();
		m_btnTransFrames->Enable();

		// update views and object if given
		refreshViewCB();
		refreshObjCB();
		break;
	}
	// refresh left panel
	Refresh();
}

bool cObjEdit::validateNewCBstring(wxComboBox* cb, const wxString& newStr) {

	if (newStr.empty()) {
		wxMessageBox("No name specified!", "Missing name", wxICON_INFORMATION | wxOK);
		return false;
	}

	// check if this name already exists
	for (int32_t item = 0; item < cb->GetCount(); item++) {
		if (!newStr.CmpNoCase(cb->GetString(item))) {
			wxMessageBox("This name alread exists. Please choose another one!", "Invalid name", wxICON_WARNING | wxOK);
			return false;
		}
	}
	cb->Append(newStr);
	return true;
}

bool cObjEdit::validateSelection(wxComboBox* cb) {
	int32_t t = cb->GetCurrentSelection();
	if (t == -1) {
		refToMain->m_console->AppendText("Error: No item selected!\n");
		wxMessageBox("You need to select something first!", "Operation failed!", wxICON_INFORMATION | wxOK);
		return false;
	}
	return true;
}


void cObjEdit::OnClose(wxCloseEvent& evt)
{
	this->Hide();
	evt.Veto();
}


void cObjEdit::OnClose(wxCommandEvent& evt)
{
	this->Hide();
	evt.Skip();
}

inline void cObjEdit::OnEditTransitions(wxCommandEvent& evt) {
	// construct a new window, previous one should have been deleted with close
	modifyTransFrame = new cEdittrans(this);
	modifyTransFrame->Show();

}

inline void cObjEdit::OnCreate(wxCommandEvent& evt)
{
	int32_t mode = modeSelector->GetCurrentSelection();

	wxString newItem;
	switch (mode) {
	case 0:
		{

		double x, z, y;
		int32_t error = 0;
		error += inputX->GetValue().ToDouble(&x);
		error += inputZ->GetValue().ToDouble(&z);
		error += inputY->GetValue().ToDouble(&y);

		if (error != 3) {
			wxMessageBox("The coordinates you typed are in an invalid format!", "Invalid coordinates", wxICON_ERROR | wxOK);
			break;
		}
		// Create scene
			std::string scene = refToMain->dfedit->SETref->addSETscene(x, z, y);
			sceneCB->AppendString(scene);
			sceneCB->Select(sceneCB->GetCount()-1);
			refToMain->m_console->AppendText(scene + " created!\n");
			// create 2 more views
			std::string view = refToMain->dfedit->SETref->addSETviews(sceneCB->GetCurrentSelection(), 90.0f, true);
			refToMain->m_console->AppendText(view + " created!\n");

			//refreshViewCB();
			Refresh();
			refToMain->refreshTree();
			break;
		}
	case 1:
		{

			// PROCESS DFEDIT!
			double in;
			if (!inputRot->GetValue().ToDouble(&in)) {
				refToMain->m_console->AppendText("Error: invalid input!");
				break;
			}
			if (c_doubleFrame->GetValue() && in >= 180.0f) {
				wxMessageBox("Double view is selected. Use only 180 degrees max!", "Invalid input", wxICON_WARNING | wxOK);
				break;
			}
			std::string view = refToMain->dfedit->SETref->addSETviews(sceneCB->GetCurrentSelection(), in, c_doubleFrame->GetValue());
			refreshViewCB();
			refToMain->m_console->AppendText(view + " created!\n");
			Refresh();
			refToMain->refreshTree();
			break;
		}
	case 2:
		std::string objName = objCB->GetValue().ToStdString();
		refToMain->dfedit->SETref->addSETobject(sceneCB->GetCurrentSelection(), viewCB->GetCurrentSelection(), objName);
		refreshObjCB();
		refToMain->m_console->AppendText(objName + " created!\n");
		Refresh();
		refToMain->refreshTree();
		break;
	}

	evt.Skip();
	return;
}


inline void cObjEdit::OnRename(wxCommandEvent& evt) {
	int32_t mode = modeSelector->GetCurrentSelection();
	switch (mode) {
	case 0:
		refToMain->m_console->AppendText("Sene renaming not implemented!\n");
		break;
	case 1:
		refToMain->m_console->AppendText("View renaming not implemented!\n");
		break;
	case 2:
		refToMain->m_console->AppendText("Object renaming not implemented!\n");
		break;
	}
	evt.Skip();
}

inline void cObjEdit::OnDelete(wxCommandEvent& evt) {
	int32_t mode = modeSelector->GetCurrentSelection();

	switch (mode) {
	case 0:

		refToMain->dfedit->SETref->removeScene(sceneCB->GetCurrentSelection());
		//sceneCB->Delete(sceneCB->GetCurrentSelection());
		refreshSceneCB();
		refToMain->refreshTree();
		refToMain->Refresh();
		break;
	case 1:

		refToMain->dfedit->SETref->removeSETviews(sceneCB->GetCurrentSelection(), viewCB->GetCurrentSelection());
			
		//viewCB->Delete(viewCB->GetCurrentSelection());
		refreshViewCB();
		Refresh();
		refToMain->refreshTree();
		refToMain->Refresh();
		break;
	case 2:
		if (!refToMain->dfedit->SETref->removeSETObject(sceneCB->GetCurrentSelection(), viewCB->GetCurrentSelection(), objCB->GetCurrentSelection())) {
			wxMessageBox("Could not remove selected object!", "Error", wxICON_ERROR | wxOK);
		}
		//viewCB->Delete(viewCB->GetCurrentSelection());
		refreshObjCB();
		Refresh();
		refToMain->refreshTree();
		refToMain->Refresh();
		break;
	}
	evt.Skip();
}

inline void cObjEdit::refreshSceneCB() {

	sceneCB->Clear();
	uint16_t strCount = refToMain->dfedit->SETref->getSETsceneCount();
	if (strCount == 0) return;

	for (int32_t scene = 0; scene < strCount; scene++) {
		sceneCB->Append(refToMain->dfedit->SETref->getSETSceneName(scene));
	}
	// set first scene to text box
	//sceneCB->SetValue(sceneCB->GetString(0));
	sceneCB->SetSelection(0);

	double posX;
	double posZ;
	double posY;
	refToMain->dfedit->SETref->getSETscenesCoordinates(0, posX, posZ, posY);
	inputX->ChangeValue(std::to_string(posX));
	inputZ->ChangeValue(std::to_string(posZ));
	inputY->ChangeValue(std::to_string(posY));
}

inline void cObjEdit::refreshViewCB() {
	viewCB->Clear();

	int32_t scene = sceneCB->GetCurrentSelection();
	uint16_t strCount = refToMain->dfedit->SETref->getSETsceneViewCount(scene);

	// dont show if no scene is given
	if (strCount == 0)	return;

	// reset obj coords
	inputStartposX->Clear();
	inputStartposY->Clear();
	inputEndposX->Clear();
	inputEndposY->Clear();

	for (int32_t view = 0; view < strCount; view++) {
		viewCB->Append(refToMain->dfedit->SETref->getSETsceneViewName(scene, view));
	}
	// set first scene to text box
	//viewCB->SetValue(dfedit->getSETsceneViewName(sceneSel, 0));
	viewCB->SetSelection(0);

	double view = refToMain->dfedit->SETref->getSETviewRotation(sceneCB->GetCurrentSelection(), 0);
	view = view * 180.0f / PI;	// calculate decimal to degrees
	inputRot->SetValue(std::to_string(view));
}

inline void cObjEdit::refreshObjCB()
{
	objCB->Clear();

	// dont do anything if nothing was selected
	int32_t scene = sceneCB->GetCurrentSelection();
	int32_t view = viewCB->GetCurrentSelection();

	if (scene == -1 || view == -1) return;

	// update image
	updateViewImage();

	int32_t objCount = refToMain->dfedit->SETref->getSETobjectCount(scene, view);
	for (int32_t obj = 0; obj < objCount; obj++) {
		objCB->Append(refToMain->dfedit->SETref->getSETobjName(scene,view,obj));
	}
	// if view has something, show me the first item
	if (objCount) {
		objCB->SetSelection(0);
		refreshObjText();
	}
		
}

inline void cObjEdit::refreshObjText() {
	int32_t scene = sceneCB->GetCurrentSelection();
	int32_t view = viewCB->GetCurrentSelection();
	int32_t obj = objCB->GetCurrentSelection();

	// sanity check
	if (scene == -1 || view == -1 || obj == -1) return;

	int16_t startX = 0;
	int16_t startY = 0;
	int16_t endX = 0;
	int16_t endY = 0;

	refToMain->dfedit->SETref->getSETobjRegion(scene, view, obj, startX, startY, endX, endY);
	// sometimes the coordinates are minus zero for some reason... normalize them!
	if (startX < 0)	startX = 0;
	if (startY < 0)	startY = 0;
	if (endX < 0)	endX = 0;
	if (endY < 0)	endY = 0;
	// fill the edit boxes
	inputStartposX->ChangeValue(std::to_string(startX));
	inputStartposY->ChangeValue(std::to_string(startY));
	inputEndposX->ChangeValue(std::to_string(endX));
	inputEndposY->ChangeValue(std::to_string(endY));
	objChg = true;
	Refresh();
}

inline void cObjEdit::OnSceneCBChange(wxCommandEvent& evt)
{
	int32_t sceneSel = evt.GetSelection();

	DFedit* dfedit = refToMain->dfedit;
	if (!dfedit) {
		wxMessageBox("No file loaded yet!", "No file", wxICON_INFORMATION | wxOK);
		return;
	}
		
	// update scene coordinates only during scene and view mode
	double posX;
	double posZ;
	double posY;
	dfedit->SETref->getSETscenesCoordinates(sceneSel, posX, posZ, posY);
	inputX->ChangeValue(std::to_string(posX));
	inputZ->ChangeValue(std::to_string(posZ));
	inputY->ChangeValue(std::to_string(posY));

	if (modeSelector->GetSelection() > 0) {
		// if here, one of the other modes was selected.
		// in this case
		// update views
		refreshViewCB();
	}

	Refresh();
	evt.Skip();
}

inline void cObjEdit::OnViewCBChange(wxCommandEvent& evt)
{
	int32_t viewSel = evt.GetSelection();

	if (!refToMain->dfedit) {
		wxMessageBox("No file loaded yet!", "No file", wxICON_INFORMATION | wxOK);
		return;
	}

	// reset obj coords
	inputStartposX->Clear();
	inputStartposY->Clear();
	inputEndposX->Clear();
	inputEndposY->Clear();

	if (modeSelector->GetSelection() > 0) {
		// update rotation in view mode only
	
		double view = refToMain->dfedit->SETref->getSETviewRotation(sceneCB->GetCurrentSelection(), viewSel);
		view = view*180.0f / PI;	// calculate decimal to degrees
		inputRot->SetValue(std::to_string(view));

		// update image for object mode
		updateViewImage();

		// also refresh objects if in correct mode
		if (modeSelector->GetSelection() == 2) {
			refreshObjCB();
		}
	}
	
	Refresh();
}

inline void cObjEdit::OnObjCBChange(wxCommandEvent& evt) {

	refreshObjText();
	evt.Skip();
}

void cObjEdit::OnPaint(wxPaintEvent& WXUNUSED(evt)) {
	// draw depending in the selected mode

	//wxClientDC dc(imagePanel);
	wxPaintDC dc(this);

	int32_t sceneSel = sceneCB->GetCurrentSelection();
	if (sceneSel == -1) {
		// draw nothing if no scene was selcted
		return;
	}
	if (modeSelector->GetSelection() == 0) {


		// check if map is loaded
		if (!refToMain->m_mapImage)
			return;

		dc.DrawBitmap(refToMain->m_imageBitmap, 0, 0);

		// check if a scene is selected
		int32_t posX;
		int32_t posZ;
		refToMain->dfedit->SETref->getSETscenesCoordinates(sceneSel, posX, posZ);
		posX *= refToMain->mapRatio;
		posZ *= refToMain->mapRatio;

		dc.SetPen(wxPen(wxColor(255, 0, 0), 1));
		// draw coordinational lines
		dc.DrawLine(0, posZ, refToMain->m_imageBitmap.GetWidth(), posZ);
		dc.DrawLine(posX, 0, posX, refToMain->m_imageBitmap.GetHeight());

		dc.SetBrush(*wxRED_BRUSH);
		dc.DrawCircle(wxPoint(posX, posZ), 6);
		return;
	}

	// dont draw if no scene was selected
	if (modeSelector->GetSelection() == 1) {
		//wxPaintDC dc(this);

		constexpr int32_t centralX = 256;
		constexpr int32_t centralY = 132;
		constexpr int32_t viewDirLength = 80;

		dc.DrawText("EAST", centralX-8, centralY-120);

		dc.SetPen(wxPen(wxColor(0, 0, 0), 1)); // black line, 3 pixels thick
		dc.SetBrush(*wxGREY_BRUSH);

		dc.DrawCircle(wxPoint(centralX, centralY), 40);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.DrawCircle(wxPoint(centralX, centralY), 35);

		
		for (int32_t view = 0; view < refToMain->dfedit->SETref->getSETsceneViewCount(sceneSel); view++) {
			double rotation = refToMain->dfedit->SETref->getSETviewRotation(sceneSel, view);
			if (rotation == -1.0) continue;	// -1 can only happen if it didnt find the view
			int32_t x;	// width
			int32_t y;	// height
			getCirclePosition(rotation, viewDirLength, x, y);
			y = -y;
			// draw selected line thicer and red. Reset line format after operation
			if (view == viewCB->GetCurrentSelection()) {
				dc.SetPen(wxPen(wxColor(255, 0, 0), 3));
				dc.DrawLine(centralX, centralY, centralX + x, centralY + y);
				dc.SetPen(wxPen(wxColor(0, 0, 0), 1));
			} else
				dc.DrawLine(centralX, centralY, centralX+x, centralY+ y);
			dc.DrawText(refToMain->dfedit->SETref->getSETsceneViewName(sceneSel,view), centralX + x, centralY + y);
		}
		// Should be Optional based on users preference: draw transition frames:
		for (int32_t frame = 0; frame < refToMain->dfedit->SETref->getSETsceneFrameCount(sceneSel); frame++) {
			double rotation;
			if (refToMain->dfedit->SETref->getSETsceneFrameRotation(sceneSel, frame, rotation) == -1) {
				// if here, there is no view ID reference
				int32_t x;	// width
				int32_t y;	// height
				getCirclePosition(rotation, 40, x, y);
				y = -y;
				dc.DrawLine(centralX, centralY, centralX + x, centralY + y);
			}
		}		
		return;
	}

	if (modeSelector->GetSelection() == 2) {
		// object mode selected

		// dont do anything if image was not loaded yet
		if (!currFrame.IsOk()) {
			dc.DrawText("No Image available!", 0, 0);
			return;
		}

		dc.DrawBitmap(currFrame, 0, 0);

		// draw click zone if an object was selected
		// if no object was selected, return and show only the image
		if (objCB->GetCurrentSelection() == -1 || viewCB->GetCurrentSelection() == -1 || objChg == false) return;

		double startX;
		double startY;
		double endX;
		double endY;
		if (!inputStartposX->GetValue().ToDouble(&startX)) return;
		if (!inputStartposY->GetValue().ToDouble(&startY)) return;
		if (!inputEndposX->GetValue().ToDouble(&endX)) return;
		if (!inputEndposY->GetValue().ToDouble(&endY)) return;
		// if all of them is zero then the object was most likely not found or the dimensions make no sense. Dont bother drawing that
		//if ((startX + startY + endX + endY) == 0) return;
		if ((startX + startY + endX + endY) == 0) return;

		dc.SetPen(wxPen(wxColor(255, 0, 0), 1));

		//dc.DrawLine(startY, startX, endY, startX);	// draw to right
		//dc.DrawLine(endY, startX, endY, endX);		// draw down
		//dc.DrawLine(endY, endX, startY, endX);		// draw to left
		//dc.DrawLine(startY, endX, startY, startX);		// draw to top

		dc.DrawLine(startY, startX, endY, startX);	// draw to right
		dc.DrawLine(endY, startX, endY, endX);		// draw down
		dc.DrawLine(endY, endX, startY, endX);		// draw to left
		dc.DrawLine(startY, endX, startY, startX);		// draw to top

		return;
	}
}

void cObjEdit::getCirclePosition(double rotation, int32_t length, int32_t& x, int32_t& y) {
	x = sin(rotation)* length;
	y = cos(rotation)* length;
}

bool cObjEdit::updateViewImage() {

	uint8_t* imagePtr;
	uint32_t imageSize;
	if (!refToMain->dfedit->SETref->getFrame(imagePtr, imageSize,sceneCB->GetCurrentSelection(), viewCB->GetCurrentSelection()))
		return false;	// content couldnt be loaded
	wxMemoryInputStream s(imagePtr, imageSize);

	// if size 566 we can expect an empty container
	if (imageSize > 566) {
		wxImage image(s);
		currFrame = wxBitmap(image, -1);
	}

	delete[] imagePtr;
	return true;
}

void cObjEdit::updateClickRegion(wxCommandEvent& evt) {

	int32_t scene = sceneCB->GetCurrentSelection();
	int32_t view = viewCB->GetCurrentSelection();
	int32_t obj = objCB->GetCurrentSelection();

	// sanity check
	if (scene == -1 || view == -1 || obj == -1) {
		wxMessageBox("Scene, view, or object not selected!", "Invalid Selection", wxICON_ERROR | wxOK);
		return;
	}

	long startX;
	long startY;
	long endX;
	long endY;

	int32_t error = 0;
	error += inputStartposX->GetValue().ToLong(&startX);
	error += inputStartposY->GetValue().ToLong(&startY);
	error += inputEndposX->GetValue().ToLong(&endX);
	error += inputEndposY->GetValue().ToLong(&endY);
	if (error != 4) {
		wxMessageBox("The region space coordinates you typed are in an invalid format!", "Invalid format", wxICON_ERROR | wxOK);
		return;
	}

	refToMain->dfedit->SETref->setSETobjRegion(scene, view, obj, startX, startY, endX, endY);

	Refresh();
	evt.Skip();
}

void cObjEdit::updateCoordinates(wxCommandEvent& evt) {

	double x;
	double z;
	double y; 

	int32_t error = 0;
	error += inputX->GetValue().ToDouble(&x);
	error += inputZ->GetValue().ToDouble(&z);
	error += inputY->GetValue().ToDouble(&y);

	if (error != 3) {
		wxMessageBox("The coordinates you typed are in an invalid format!", "Invalid coordinates", wxICON_ERROR | wxOK);
		return;
	}

	refToMain->dfedit->SETref->changeSETsceneCoordinates(refToMain->m_console,sceneCB->GetCurrentSelection() , x, z, y);
	Refresh();
	refToMain->refreshTree();
	refToMain->mainPanel->Refresh();
	refToMain->Refresh();
}
