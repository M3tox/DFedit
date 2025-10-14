#pragma once
#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/grid.h>

#include "DFedit.h"

#define SCRIPTEDITWIND 15000
#define SCENEEDITWIND 15001
#define ROADEDITWIND 15002

class cMain : public wxFrame
{
public:
	cMain();
	~cMain();

	class cSETueCoords : public wxFrame
	{

	public:
		cSETueCoords(cMain* mainRef)
			: wxFrame(mainRef, 21001, "UE Reference", wxDefaultPosition, wxSize(256, 230), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX) | wxFRAME_FLOAT_ON_PARENT), mainRef(mainRef)
		{
			Centre();
			this->SetBackgroundColour(wxColor(170, 170, 255));

			textLabels[0] = new wxStaticText(this, wxID_ANY, "X axis:", wxPoint(10, 10));
			textLabels[1] = new wxStaticText(this, wxID_ANY, "Z axis:", wxPoint(10, 40));
			textLabels[2] = new wxStaticText(this, wxID_ANY, "Y axis:", wxPoint(10, 70));
			textLabels[3] = new wxStaticText(this, wxID_ANY, "Ratio:", wxPoint(10, 100));
			textLabels[4] = new wxStaticText(this, wxID_ANY, wxEmptyString, wxPoint(100, 125));

			for (uint8_t i = 0; i < 4; i++)
				inputs[i] = new wxTextCtrl(this, 500 + i, wxEmptyString, wxPoint(100, 10+(30 *i)));


			d_btnConfirm = new wxButton(this, 20000, "Confirm", wxPoint(20, 150));
			d_btnCancel = new wxButton(this, 20001, "Cancel", wxPoint(145, 150));

			float coords[4];
			std::string ratioStr;
			mainRef->dfedit->SETref->getUEcoordinates(coords[0], coords[1], coords[2], coords[3], ratioStr);
			for (uint8_t i = 0; i < 4; i++)
				inputs[i]->SetValue(std::to_string(coords[i]));

			textLabels[4]->SetLabelText("Default ratio: " + ratioStr);

		}
		~cSETueCoords() {
			for (uint8_t i = 0; i < 5; i++)
				delete textLabels[i];
			for (uint8_t i = 0; i < 4; i++)
				delete inputs[i];

			delete d_btnConfirm;
			delete d_btnCancel;
		}

	private:
		cMain* mainRef;

		wxStaticText* textLabels[5]{ nullptr };
		wxTextCtrl* inputs[4]{ nullptr };
		// buttons
		wxButton* d_btnConfirm = nullptr;
		wxButton* d_btnCancel = nullptr;

		void OnClose(wxCommandEvent& evt) {
			Close();
		}

		void OnConfirm(wxCommandEvent& evt) {

			float coordsF[4];
			double coords[4];
			uint8_t check{ 0 };
			for (uint8_t i = 0; i < 4; i++)
				check += inputs[i]->GetValue().ToDouble(&coords[i]);

			if (check != 4) {
				wxMessageBox("Incorrect format entered, please try again with X.XXX", "Bad format", wxICON_INFORMATION | wxOK);
				return;
			}
			// the little data loss here is okay. we dont need it too precice
			for (uint8_t i = 0; i < 4; i++)
				coordsF[i] = coords[i];

			mainRef->dfedit->SETref->addUEcoordinates(coordsF[0], coordsF[1], coordsF[2], coordsF[3]);

			Close();
		}

		wxDECLARE_EVENT_TABLE();
	};

	
	// reference to editbox in other window
	wxTextCtrl* scriptWindow;
	wxTextCtrl* m_console = nullptr;

	// used in child window as well
	wxBitmap m_imageBitmap;
	wxImage* m_mapImage = nullptr;

	float mapRatio = 0; // used for SET maps to draw objects on
	// used to calculate either the int or double val
	double toIntegerRatio = 0;
	double toDoubleRatio = 0;

	DFedit* dfedit;
	wxPanel* mainPanel = nullptr;

	void refreshTree();

private:

	wxString dfScriptName;

	wxSplitterWindow* splitter;
	
	cSETueCoords* ueWindow = nullptr;
	bool ShowListBox = true;

	wxMenuBar* m_MenuBar = nullptr;
	wxMenu* menuEditData = nullptr;

	wxPanel* leftPanel = nullptr;
	
	wxPanel* bottomPanel = nullptr;

	wxPanel* left = nullptr;
	wxPanel* right = nullptr;
	wxPanel* bottom = nullptr;

	wxButton* m_btnConsole = nullptr;
	wxButton* m_btnControl = nullptr;


	wxGrid* m_tableSETframes = nullptr;
	wxGrid* m_tableSETsceneHeader = nullptr;
	wxGrid* m_tableSETviews = nullptr;
	wxGrid* m_tableSETviewsObj = nullptr;
	wxGrid* m_tableColorPalette = nullptr;
	wxGrid* m_tableSETactors = nullptr;
	wxGrid* m_tableGeneral = nullptr;


	//wxListBox* m_list1 = nullptr;
	//wxListBox* m_list2 = nullptr;
	wxTreeCtrl* m_tree1 = nullptr;


	// knows if changes were made or not
	// used to help the user to save data before closing/changing
	bool changed = false;	

	wxMenu* menuFile = nullptr;
	// menu buttons
	void OnMenuOpenFile(wxCommandEvent& evt);
	void OnMenuSaveFile(wxCommandEvent& evt);
	void OnMenuSaveFileAs(wxCommandEvent& evt);
	void OnMenuExit(wxCommandEvent& evt);

	void OnReadNewRes(wxCommandEvent& evt);
	void OnDeleteAllRes(wxCommandEvent& evt);

	void OnPrintOverview(wxCommandEvent& evt);

	void OnEditScenes(wxCommandEvent& evt);
	void OnEditRoads(wxCommandEvent& evt);
	void OnChangeMap(wxCommandEvent& evt);
	void OnSetUEref(wxCommandEvent& evt);

	// table events
	void OnGridCellChange(wxGridEvent& evt);

	// regular buttons
	void OnConsoleButtonClicked(wxCommandEvent& evt);
	// updates data to containers
	void OnApplyChanges(wxCommandEvent& evt);

	// paint events
	void OnPaint(wxPaintEvent& event);

	// assistant functions
	void wxLoadImage(wxString fileName);
	void wxLoadImage(uint8_t* image, uint32_t imageSize);

	bool checkErrors(int32_t errCode);
	
	void OnControlTree(wxTreeEvent& evt);

	void displayTable(wxGrid* table);
	inline void formatTable(wxGrid* table);
	inline void formatSETviewTable(wxGrid* table);
	inline void formatSETviewObjTable(wxGrid* table);
	inline void formatSETframeTable(wxGrid* table);
	inline void formatSETsceneTable(wxGrid* table);
	inline void formatSETactorsTable(wxGrid* table);

	wxDECLARE_EVENT_TABLE();
};
