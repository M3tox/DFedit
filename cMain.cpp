#include "cMain.h"

#define OPENFILE		10001
#define SAVEFILE		10002
#define SAVEFILEAS		10003
#define EXITAPP			10004
#define CLEARCONSOLE	10005

#define APPLYCHG		10007
#define READRES			10008
#define DELETERES		10009

// print functions
#define PRINTOVERVIEW	10100

#define DEFAULTTABLE	11000
#define SETFRAMETABLE	11001
#define SETVIEWTABLE	11002
#define SETVIEWOBJTABLE	11003
#define SETSCENETABLE	11004
#define SETSCENEACT		11005

#define SETSCENEEDIT	12000
#define SETROADEDIT		12001
#define SETMAP			12002
#define SETROTTABLE		12003
#define SETUE			12004

wxBEGIN_EVENT_TABLE(cMain::cSETueCoords, wxFrame)
EVT_BUTTON(20000, cMain::cSETueCoords::OnConfirm)
EVT_BUTTON(20001, cMain::cSETueCoords::OnClose)
wxEND_EVENT_TABLE()


wxBEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_MENU(OPENFILE, cMain::OnMenuOpenFile)
EVT_MENU(SAVEFILE, cMain::OnMenuSaveFile)
EVT_MENU(SAVEFILEAS, cMain::OnMenuSaveFileAs)
EVT_MENU(EXITAPP, cMain::OnMenuExit)
EVT_MENU(READRES, cMain::OnReadNewRes)
EVT_MENU(DELETERES, cMain::OnDeleteAllRes)
EVT_MENU(PRINTOVERVIEW, cMain::OnPrintOverview)
EVT_MENU(SETSCENEEDIT, cMain::OnEditScenes)
EVT_MENU(SETROADEDIT, cMain::OnEditRoads)
EVT_MENU(SETMAP, cMain::OnChangeMap)
EVT_MENU(SETUE, cMain::OnSetUEref)

EVT_BUTTON(CLEARCONSOLE, cMain::OnConsoleButtonClicked)
EVT_BUTTON(APPLYCHG, cMain::OnApplyChanges)

EVT_PAINT(OnPaint)
EVT_TREE_SEL_CHANGED(10006, OnControlTree)
EVT_GRID_CELL_CHANGING(OnGridCellChange)
wxEND_EVENT_TABLE()


cMain::cMain() 
	: wxFrame(nullptr, wxID_ANY, "DFedit", wxPoint(800, 400), wxSize(800, 600)) {

	// used for script editor window
	dfScriptName.assign("DF Script Editor | ");

	// add menu bar
	m_MenuBar = new wxMenuBar();
	this->SetMenuBar(m_MenuBar);
	// add items
	menuFile = new wxMenu();
	menuFile->Append(OPENFILE, "Open File");
	menuFile->Append(SAVEFILE, "Save")->Enable(false);
	menuFile->Append(SAVEFILEAS, "Save as...")->Enable(false);
	menuFile->AppendSeparator();
	menuFile->Append(EXITAPP, "Exit");
	m_MenuBar->Append(menuFile, "File");

	wxMenu* menuRes = new wxMenu();
	menuRes->Append(READRES, "Read new Resources...");
	menuRes->Append(DELETERES, "Delete all Resources");
	m_MenuBar->Append(menuRes, "Resources");

	wxMenu* menuPrintData = new wxMenu();
	menuPrintData->Append(PRINTOVERVIEW, "Container information");
	m_MenuBar->Append(menuPrintData, "Print Data");

	menuEditData = new wxMenu();
	menuEditData->Append(SETSCENEEDIT, "Scenes");
	menuEditData->Append(SETROADEDIT, "Roads");
	menuEditData->Append(SETMAP, "Change map");
	menuEditData->Append(SETUE, "Set UE Reference");
	menuEditData->AppendSeparator();
	menuEditData->AppendCheckItem(SETROTTABLE, "Use only one rotation table");
	m_MenuBar->Append(menuEditData, "Edit");

	// sub menu to create new files
  // TODO: Create functions must be created before menu items are created
	/*
	wxMenu* menuCreate = new wxMenu();
	menuCreate->Append(PRINTOVERVIEW, "Set...")->Enable(false);
	menuCreate->Append(PRINTOVERVIEW, "Movie...")->Enable(false);
	menuCreate->Append(PRINTOVERVIEW, "Puppet...")->Enable(false);
	m_MenuBar->Append(menuCreate, "Create new");
	*/

	// process screen splitters
	splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

	wxSplitterWindow* right_splitter = new wxSplitterWindow(splitter, wxID_ANY,	wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

	leftPanel = new wxPanel(splitter);
	mainPanel = new wxPanel(right_splitter);
	bottomPanel = new wxPanel(right_splitter);

	leftPanel->SetBackgroundColour(wxColor(170, 170, 255));
	mainPanel->SetBackgroundColour(wxColor(150, 150, 150));
	bottomPanel->SetBackgroundColour(wxColor(170, 170, 255));

	right_splitter->SetMinimumPaneSize(100);
	right_splitter->SplitHorizontally(mainPanel, bottomPanel);

	right_splitter->SetSashGravity(1);

	splitter->SetMinimumPaneSize(200);
	splitter->SplitVertically(leftPanel, right_splitter);

	m_btnConsole = new wxButton(bottomPanel, CLEARCONSOLE, "clear console", wxPoint(10, 10), wxSize(150, 30));
	m_btnControl = new wxButton(leftPanel, APPLYCHG, "Apply changes", wxPoint(10, 10), wxSize(100, 30));
	m_btnControl->Disable();
	
	m_console = new wxTextCtrl(bottomPanel, wxID_ANY, "", wxPoint(0, 0), wxSize(300, 30), wxTE_MULTILINE | wxTE_READONLY);
	m_console->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	m_tree1 = new wxTreeCtrl(leftPanel, 10006, wxPoint(0, 0), wxSize(300, 300));

	m_tableGeneral = new wxGrid(leftPanel, DEFAULTTABLE, wxPoint(10, 410), wxSize(50, 300));
	m_tableSETframes = new wxGrid(leftPanel, SETFRAMETABLE, wxPoint(10, 410), wxSize(50, 300));
	m_tableSETviews = new wxGrid(leftPanel, SETVIEWTABLE, wxPoint(10, 410), wxSize(50, 300));
	m_tableSETviewsObj = new wxGrid(leftPanel, SETVIEWOBJTABLE, wxPoint(10, 410), wxSize(50, 300));
	m_tableSETsceneHeader = new wxGrid(leftPanel, SETSCENETABLE, wxPoint(10, 410), wxSize(50, 300));
	m_tableSETactors = new wxGrid(leftPanel, SETSCENEACT, wxPoint(10, 410), wxSize(50, 300));
	m_tableColorPalette = new wxGrid(leftPanel, wxID_ANY, wxPoint(0, 0), wxSize(50, 300));

	m_tableGeneral->CreateGrid(2, 2);
	// format the table to a minimalistic design
	formatTable(m_tableGeneral);
	m_tableGeneral->SetCellValue(0, 0, "Items:");
	m_tableGeneral->SetReadOnly(0, 0);
	m_tableGeneral->SetReadOnly(0, 1);

	// init SET frames, 13 entries total
	m_tableSETframes->CreateGrid(13, 2);
	formatTable(m_tableSETframes);
	m_tableSETframes->Hide();
	formatSETframeTable(m_tableSETframes);

	// view entries
	m_tableSETviews->CreateGrid(7, 2);
	formatTable(m_tableSETviews);
	m_tableSETviews->Hide();
	formatSETviewTable(m_tableSETviews);

	// view objects
	m_tableSETviewsObj->CreateGrid(9, 2);
	formatTable(m_tableSETviewsObj);
	m_tableSETviewsObj->Hide();
	formatSETviewObjTable(m_tableSETviewsObj);

	// SCENE Header
	m_tableSETsceneHeader->CreateGrid(15, 2);
	formatTable(m_tableSETsceneHeader);
	m_tableSETsceneHeader->Hide();
	formatSETsceneTable(m_tableSETsceneHeader);

	// ACTOR Entries
	m_tableSETactors->CreateGrid(6, 2);
	formatTable(m_tableSETactors);
	m_tableSETactors->Hide();
	formatSETactorsTable(m_tableSETactors);

	// color palette
	m_tableColorPalette->CreateGrid(16, 16);
	for (uint8_t col = 0; col < 16; col++) {
		m_tableColorPalette->SetColSize(col, 18);
	}
	for (uint8_t rw = 0; rw < 16; rw++) {
		m_tableColorPalette->SetRowSize(rw, 18);
	}
	m_tableColorPalette->HideColLabels();
	m_tableColorPalette->HideRowLabels();
	m_tableColorPalette->DisableDragRowSize();
	m_tableColorPalette->DisableDragColSize();
	m_tableColorPalette->Hide();


	wxBoxSizer* consoleVbp = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* consoleHbp = new wxBoxSizer(wxHORIZONTAL);
	//s1->Add(m_console, 0, wxEXPAND | wxALL, 5);
	consoleVbp->Add(m_console, 1, wxEXPAND | wxALL, 2);
	consoleVbp->Add(m_btnConsole, 0, wxALIGN_RIGHT | wxALL, 2);
	consoleHbp->Add(consoleVbp, 1, wxEXPAND | wxALL, 2);

	//controlVlp = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* controlVlp = new wxBoxSizer(wxVERTICAL);
	//wxBoxSizer* controlHlp = new wxBoxSizer(wxHORIZONTAL);
	controlVlp->Add(m_tree1, 1, wxEXPAND, 0);
	controlVlp->Add(m_btnControl, 0, wxEXPAND | wxALL, 2);
	controlVlp->Add(m_tableGeneral, 0, wxEXPAND, 0);
	controlVlp->Add(m_tableSETframes, 0, wxEXPAND, 0);
	controlVlp->Add(m_tableSETsceneHeader, 0, wxEXPAND, 0);
	controlVlp->Add(m_tableSETviews, 0, wxEXPAND, 0);
	controlVlp->Add(m_tableSETviewsObj, 0, wxEXPAND, 0);
	controlVlp->Add(m_tableSETactors, 0, wxEXPAND, 0);
	controlVlp->Add(m_tableColorPalette, 0, wxEXPAND, 0);


	bottomPanel->SetSizerAndFit(consoleHbp);
	leftPanel->SetSizerAndFit(controlVlp);
	
}

void cMain::OnPaint(wxPaintEvent& WXUNUSED(event))
//------------------------------------------------------------------------
// update the main window content
{
	//wxImage* tempImage;  // the bridge between my image buffer and the bitmap to display

	//wxPaintDC dc(this);
	SetDoubleBuffered(true);
	wxClientDC dc(mainPanel);

	if (m_mapImage)
	{

		//tempImage = new wxImage(m_imageWidth, m_imageHeight, m_myImage, true); // lend my image buffer...
		//m_imageBitmap = wxBitmap(*tempImage, -1); // ...to get the corresponding bitmap
		//delete(tempImage);		// buffer not needed any more
		dc.DrawBitmap(m_imageBitmap, 0, 0);

		// draw roads
		dc.SetPen(wxPen(wxColor(0, 0, 0), 1)); // black line, 3 pixels thick
		dc.SetBrush(*wxYELLOW_BRUSH);
		for (int32_t road = 0; road < dfedit->SETref->getSETRoadCount(); road++) {
			// draw connection lines
			int32_t roadFrameCount = dfedit->SETref->getSETRoadFrameCount(road);
			for (int32_t roadFrame = 1; roadFrame < roadFrameCount; roadFrame++) {
				int32_t x_start;
				int32_t z_start;
				int32_t x_end;
				int32_t z_end;
				dfedit->SETref->getSETroadFrameCoords(road, roadFrame-1, x_start, z_start);
				dfedit->SETref->getSETroadFrameCoords(road, roadFrame, x_end, z_end);
				x_start *= mapRatio;
				z_start *= mapRatio;
				x_end *= mapRatio;
				z_end *= mapRatio;
				dc.DrawLine(x_start, z_start, x_end, z_end); // draw line across the rectangle

				// dont draw the last one, we use bigger circles for the scene positions
				if (roadFrame != dfedit->SETref->getSETRoadFrameCount(road)-1)
					dc.DrawCircle(wxPoint(x_end, z_end), 3);
			}

			// draw road names on map
			int32_t x_start;
			int32_t z_start;
			int32_t x_end;
			int32_t z_end;
			dfedit->SETref->getSETroadFrameCoords(road, 0, x_start, z_start);
			dfedit->SETref->getSETroadFrameCoords(road, roadFrameCount-1, x_end, z_end);
			x_start *= mapRatio;
			z_start *= mapRatio;
			x_end *= mapRatio;
			z_end *= mapRatio;
			// the minus 20 is just an offset to get it more centered
			dc.DrawText((dfedit->SETref->getSETRoadName(road)), x_start + ((x_end-x_start)/2) - 20, z_start + ((z_end - z_start) / 2));

		}
		dc.SetBrush(*wxCYAN_BRUSH);
		// draw scene data
		for (int32_t scene = 0; scene < dfedit->SETref->getSETsceneCount(); scene++) {
			int32_t posX;
			int32_t posZ;
			dfedit->SETref->getSETscenesCoordinates(scene, posX,posZ);

			posX *= mapRatio;
			posZ *= mapRatio;
			dc.DrawCircle(wxPoint(posX, posZ), 5);
			// offset text a bit away
			dc.DrawText((dfedit->SETref->getSETSceneName(scene)), posX-20, posZ-20);
		}

		// draw actors positions
		dc.SetBrush(*wxGREEN_BRUSH);
		for (int32_t actor = 0; actor < dfedit->SETref->getActorsCount(); actor++) {
			int32_t posX;
			int32_t posZ;
			dfedit->SETref->getActorsPosition(actor, posX, posZ);
			posX *= mapRatio;
			posZ *= mapRatio;

			dc.DrawRectangle(posX-4, posZ-4, 8 , 8);

			dc.DrawText((dfedit->SETref->getActorsIdent(actor)), posX+4, posZ-9);
		}

	}

}

void cMain::OnMenuExit(wxCommandEvent& evt)
{
	if (changed)
	{
		if (wxMessageBox(_("Current content has not been saved! Exit anyway?"), _("WARNING"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
		//else: proceed asking to the user the new file to open
	}
	Close();
	evt.Skip();
}

void cMain::OnReadNewRes(wxCommandEvent& evt)
{
	if (dfedit && dfedit->SETref) {
		if (!checkErrors(dfedit->SETref->updateSETRessources()))
			return;

		m_console->AppendText("New ressources read!\n");
	}
	else {
		m_console->AppendText("No Set loaded!\n");
	}

	return;
}

void cMain::OnDeleteAllRes(wxCommandEvent& evt)
{
	if (dfedit && dfedit->SETref) {
		dfedit->SETref->clearResources();
		m_console->AppendText("All resources deleted!\n");
	}
	else {
		m_console->AppendText("No Set loaded!\n");
	}
}

void cMain::OnPrintOverview(wxCommandEvent& evt)
{
	if (dfedit && dfedit->SETref) {
		if (!dfedit->SETref->reconstructContainers(menuEditData->FindItemByPosition(5)->IsChecked())) {
			m_console->AppendText("Error reconstructing the file!\n");
			return;
		}

		m_console->AppendText(dfedit->printContainerInfo());
		dfedit->SETref->clearContainers();
	}
	else {
		m_console->AppendText("No File loaded!\n");
	}

	
}

void cMain::OnGridCellChange(wxGridEvent& evt)
{
	// TODO: There is no error typing checking going on.

	int32_t tableID = evt.GetId();
	int32_t row = evt.GetRow();
	switch (tableID) {
	case SETFRAMETABLE:
		// update coords and rotation to both formats, depending what was changed
		
		if (row >= 0 && row < 5)
			return;

		double coordf;
		long coordi;
		if (row == 5 || row == 6 || row == 7) {
			// float coordinates was changed

			evt.GetString().ToDouble(&coordf);
			m_tableSETframes->GetCellValue(row + 4, 1).ToLong(&coordi);
			coordi = toIntegerRatio* coordf;
			m_tableSETframes->SetCellValue(row + 4, 1, std::to_string(coordi));

			return;
		}
		if (row == 8) {
			// float degrees was selected
			evt.GetString().ToDouble(&coordf);
			m_tableSETframes->GetCellValue(row + 4, 1).ToLong(&coordi);

			coordi = (256.0f / 360.0f) * coordf;
			m_tableSETframes->SetCellValue(row + 4, 1, std::to_string(coordi));
			return;
		}
		if (row == 9 || row == 10 || row == 11) {
			// integer coordinates was changed

			evt.GetString().ToLong(&coordi);
			m_tableSETframes->GetCellValue(row - 4, 1).ToDouble(&coordf);
			coordf = toDoubleRatio * coordi;
			m_tableSETframes->SetCellValue(row - 4, 1, std::to_string(coordf));
			return;
		}
		break;
	case SETSCENETABLE:
		if (row == 2 || row == 3 || row == 4) {
			// integer coordinates was changed
			evt.GetString().ToLong(&coordi);
			m_tableSETsceneHeader->GetCellValue(row +8, 1).ToDouble(&coordf);
			coordf = toDoubleRatio * coordi;
			m_tableSETsceneHeader->SetCellValue(row +8, 1, std::to_string(coordf));

			// view name was changed. apply change to tree item as well
			// USE THIS FOR APPLY BUTTON ONLY!
			//m_tree1->SetItemText(m_tree1->GetFocusedItem(), evt.GetString());
			return;
		}
		if (row == 10 || row == 11 || row == 12) {
			// float coordinate was changed
			evt.GetString().ToDouble(&coordf);
			m_tableSETsceneHeader->GetCellValue(row -8, 1).ToLong(&coordi);
			coordi = toIntegerRatio * coordf;
			m_tableSETsceneHeader->SetCellValue(row -8, 1, std::to_string(coordi));
			return;
		}
		break;
	}
	evt.Skip();

}

void cMain::wxLoadImage(wxString fileName) {
	// open image dialog box
	m_mapImage = new wxImage(fileName, wxBITMAP_TYPE_ANY, -1); // ANY => can load many image formats
	m_imageBitmap = wxBitmap(*m_mapImage, -1); // ...to get the corresponding bitmap

	//Refresh();

	delete m_mapImage;
}

void cMain::wxLoadImage(uint8_t* image, uint32_t imageSize) {
	wxMemoryInputStream s(image, imageSize);

	m_mapImage = new wxImage(s);
	m_imageBitmap = wxBitmap(*m_mapImage, -1);

	mainPanel->Refresh();
	//Refresh();
	delete m_mapImage;
	delete[] image;
}

void cMain::OnEditScenes(wxCommandEvent& evt)
{
	// only open up the window if a file was loaded
	if (!dfedit) {
		wxMessageBox("No file loaded yet!", "No file", wxICON_INFORMATION | wxOK);
		return;
	}

	this->GetWindowChild(SCENEEDITWIND)->Show();

}

void cMain::OnEditRoads(wxCommandEvent& evt)
{
	// check if a file was loaded
	if (!dfedit) {
		m_console->AppendText("Error: No file is loaded!\n");
		return;
	}

	// delete this when window is done
	this->GetWindowChild(ROADEDITWIND)->Show();
	return;
}

void cMain::OnChangeMap(wxCommandEvent& evt)
{
	// check if a file was loaded
	if (!dfedit) {
		m_console->AppendText("Error: No file is loaded!\n");
		return;
	}

	if (!dfedit->SETref) {
		m_console->AppendText("Error: No SET file is loaded!\n");
		return;
	}
	wxFileDialog openFileDialog(this, _("Open BMP file"), "", "", "Bitmap (*.BMP)|*.BMP", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	
	// proceed loading the file chosen by the user;
	// this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}

	
	if (!checkErrors(dfedit->SETref->changeSETmap(openFileDialog.GetPath().ToStdString())))
		return;
	// refresh map
	uint8_t* image;
	uint32_t imageSize;
	dfedit->SETref->getSETmap(image, imageSize);
	wxLoadImage(image, imageSize);

	Refresh();
	return;
}

void cMain::OnConsoleButtonClicked(wxCommandEvent& evt) {

	
	m_console->Clear();
	//wxLoadImage("lounge/262_depthMap.png");
	evt.Skip();
}

void cMain::OnApplyChanges(wxCommandEvent& evt)
{
	changed = true;
}

void cMain::OnMenuOpenFile(wxCommandEvent& evt) {

	if (changed)
	{
		if (wxMessageBox(_("Current content has not been saved! Proceed?"), _("WARNING"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
		//else: proceed asking to the user the new file to open
	}

	wxFileDialog openFileDialog(this, _("Open SET file"), "", "", "Set files (*.SET)|*.SET|Puppet files (*.PUP)|*.PUP|Cast files (*.CST)|*.CST", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...

	// proceed loading the file chosen by the user;
	// this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	
	if (openFileDialog.GetPath().empty()) return;

	delete dfedit;
	// reset main image, if there is any
	m_mapImage = nullptr;
	//dfedit = new DFedit(openFileDialog.GetPath().ToStdString(), m_console);
	dfedit = new DFedit();
	if (!checkErrors(dfedit->loadFile(openFileDialog.GetPath().ToStdString())))
		return;
	

	// enable saving options
	menuFile->FindItemByPosition(1)->Enable();
	menuFile->FindItemByPosition(2)->Enable();

	// TODO: VARIFY FILE BEFORE CONTINUEING!
	// makes sure scene editor is closed. it will load new stuff when reopened
	this->GetWindowChild(SCENEEDITWIND)->Hide();
	
	if (dfedit->SETref) {
		// will delete current containers
		dfedit->SETref->clearContainers();
		mapRatio = dfedit->SETref->getSETmapRatio();
		dfedit->SETref->getCoordRatios(toIntegerRatio, toDoubleRatio);
		//m_list1->Delete(0);
		//m_list1->Clear();
		uint8_t* image;
		uint32_t imageSize;
		dfedit->SETref->getSETmap(image, imageSize);
		wxLoadImage(image, imageSize);

		// update tree view
		//m_tree1->AppendItem(m_tree1, "test");
		refreshTree();

		menuEditData->FindItemByPosition(5)->Check(dfedit->SETref->checkFrameRegister());

		//dfedit->SETref->printHighestIDs(m_console);
		//m_btnControl->Enable();

		// init color palette
	}
	else if (dfedit->SHPref) {
		// TODO
		m_console->AppendText("ERROR: SHOP files not supported yet.\n");
		return;
	}
	else if (dfedit->PUPref) {
		refreshTree();
	}
	else {
		m_console->AppendText("ERROR: Frontend for this type not implemented yet\n");
		return;
	}

	// notify the user that the set has been loaded successfully
	m_console->AppendText(openFileDialog.GetFilename());
	m_console->AppendText(" loaded!\n");

	evt.Skip();
}

void cMain::OnMenuSaveFile(wxCommandEvent& evt)
{
	m_console->AppendText("Saving not implemented yet\n");
}

void cMain::OnMenuSaveFileAs(wxCommandEvent& evt)
{
	wxFileDialog
		saveFileDialog(this, _("Save SET file"), "", "",
			"SET files (*.SET)|*.SET", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...

	// save the current contents in the file;
	// this can be done with e.g. wxWidgets output streams:
	if (dfedit->SETref) {
		if (!dfedit->SETref->reconstructContainers(menuEditData->FindItemByPosition(5)->IsChecked())) {
			m_console->AppendText("Error reconstructing the file!\n");
			return;
		}
		dfedit->setDFeditMode(-1);	// -1 = no extraction with DFET
		if (!dfedit->writeNewFile(saveFileDialog.GetPath().ToStdString())) {
			m_console->AppendText("Error writing the file!\n");
			return;
		}
		// get back to default state
		// images and scripts will be deleted by delete functions and destructor
		dfedit->SETref->clearContainers();
	}
	else {
		m_console->AppendText("ERROR: File type not supported!\n");
		return;
	}


	m_console->AppendText("File \"" + saveFileDialog.GetFilename() + "\" saved!\n");
	changed = false;
}

void cMain::OnControlTree(wxTreeEvent& evt)
{
	wxTreeItemIdValue rootCookie;
	wxTreeItemId category;
	category = m_tree1->GetFirstChild(m_tree1->GetRootItem(), rootCookie);

	if (dfedit->SETref) {
		if (category.IsOk()) {
			// check scene selections

			wxTreeItemIdValue sceneIndexCookie;
			wxTreeItemId sceneIndexItem;
			sceneIndexItem = m_tree1->GetFirstChild(category, sceneIndexCookie);
			for (int32_t scene = 0; sceneIndexItem.IsOk(); scene++) {

				// check scene data
				wxTreeItemIdValue sceneCookie;
				wxTreeItemId sceneItem;
				sceneItem = m_tree1->GetFirstChild(sceneIndexItem, sceneCookie);

				if (sceneIndexItem.GetID() == evt.GetItem().GetID()) {
					// if here, Scene X was selected
					dfedit->SETref->fillSETsceneHeader(m_tableSETsceneHeader, scene);
					displayTable(m_tableSETsceneHeader);

					return;
				}

				// variables for rotation tables
				wxTreeItemIdValue turnCookie;
				wxTreeItemId turnItem = m_tree1->GetFirstChild(sceneItem, turnCookie);
				for (int32_t view = 0; turnItem.IsOk(); view++) {

					if (turnItem.GetID() == evt.GetItem().GetID()) {
						// view item was selected
						dfedit->SETref->fillSETsceneViewData(m_tableSETviews, scene, view);
						displayTable(m_tableSETviews);
						return;
					}

					// now check possible objects that are attached to the view
					wxTreeItemIdValue objCookie;
					wxTreeItemId objItem = m_tree1->GetFirstChild(turnItem, objCookie);
					for (int32_t obj = 0; objItem.IsOk(); obj++) {
						if (objItem.GetID() == evt.GetItem().GetID()) {
							// view OBJECT was selected
							dfedit->SETref->fillSETsceneViewObjectData(m_tableSETviewsObj, scene, view, obj);
							displayTable(m_tableSETviewsObj);
							// also view corresponding script
							this->GetWindowChild(SCRIPTEDITWIND)->Show();
							this->GetWindowChild(SCRIPTEDITWIND)->SetLabel(dfScriptName + m_tree1->GetItemText(turnItem) + " | " + m_tree1->GetItemText(objItem));
							if (!scriptWindow)
								m_console->AppendText("ERROR: instance of script window not loaded!\n");
							else
								scriptWindow->ChangeValue(dfedit->SETref->getSETsceneViewObjectScript(scene, view, obj));
							return;
						}

						objItem = m_tree1->GetNextChild(turnItem, objCookie);
					}

					turnItem = m_tree1->GetNextChild(sceneItem, turnCookie);
				}

				// get second element
				sceneItem = m_tree1->GetNextChild(sceneIndexItem, sceneCookie);
				for (uint8_t turn = 0; turn < 2; turn++) {
					// iterate though right and left rotation table
					turnItem = m_tree1->GetFirstChild(sceneItem, turnCookie);
					for (int32_t frame = 0; turnItem.IsOk(); frame++) {
						if (turnItem.GetID() == evt.GetItem().GetID()) {
							// this rotation frame was selected
							dfedit->SETref->fillSETsceneFrameData(m_tableSETframes, scene, turn, frame);

							displayTable(m_tableSETframes);
							return;
						}
						turnItem = m_tree1->GetNextChild(sceneItem, turnCookie);
					}
					// get third and last element
					sceneItem = m_tree1->GetNextChild(sceneIndexItem, sceneCookie);
				}

				// get last element: scene scripts
				if (sceneItem.GetID() == evt.GetItem().GetID()) {
					// if here, scene script was selected!
					this->GetWindowChild(SCRIPTEDITWIND)->Show();
					this->GetWindowChild(SCRIPTEDITWIND)->SetLabel(dfScriptName + dfedit->SETref->getSETSceneName(scene));
					if (!scriptWindow)
						m_console->AppendText("ERROR: instance of script window not loaded!\n");
					else
						scriptWindow->ChangeValue(dfedit->SETref->getSceneScript(scene));

					displayTable(m_tableGeneral);
					m_tableGeneral->SetCellValue(0, 1, "script type");
					return;
				}

				// next scene
				sceneIndexItem = m_tree1->GetNextChild(category, sceneIndexCookie);
			}
		}

		category = m_tree1->GetNextChild(m_tree1->GetRootItem(), rootCookie);
		if (category.IsOk()) {
			// check road selections
			wxTreeItemIdValue roadIndexCookie;
			wxTreeItemId roadItem = m_tree1->GetFirstChild(category, roadIndexCookie);
			for (int32_t road = 0; roadItem.IsOk(); road++) {
				//if (actItem.GetID() == evt.GetItem().GetID()) {
				//	// actor selected
				//	dfedit->fillSETactorsTable(m_tableSETactors, act);
				//	displayTable(m_tableSETactors);
				//	return;
				//}
				// to though 2 frame registers
				wxTreeItemIdValue frameRegCookie;
				wxTreeItemId frameRegItem = m_tree1->GetFirstChild(roadItem, frameRegCookie);
				for (uint8_t frameReg = 0; frameReg < 2; frameReg++) {
					wxTreeItemIdValue frameCookie;
					wxTreeItemId frameItem = m_tree1->GetFirstChild(frameRegItem, frameCookie);
					for (int32_t frame = 0; frameItem.IsOk(); frame++) {
						if (frameItem.GetID() == evt.GetItem().GetID()) {
							// if here, one of the road frames was selected
							dfedit->SETref->fillSETroadFrameData(m_tableSETframes, road, frameReg, frame);

							displayTable(m_tableSETframes);
							return;
						}
						frameItem = m_tree1->GetNextChild(frameRegItem, frameCookie);
					}
					frameRegItem = m_tree1->GetNextChild(roadItem, frameRegCookie);
				}


				roadItem = m_tree1->GetNextChild(category, roadIndexCookie);
			}


		}

		category = m_tree1->GetNextChild(m_tree1->GetRootItem(), rootCookie);
		if (category.IsOk()) {
			// check actors table
			wxTreeItemIdValue actCookie;
			wxTreeItemId actItem = m_tree1->GetFirstChild(category, actCookie);
			for (int32_t act = 0; actItem.IsOk(); act++) {
				if (actItem.GetID() == evt.GetItem().GetID()) {
					// actor selected
					dfedit->SETref->fillSETactorsTable(m_tableSETactors, act);
					displayTable(m_tableSETactors);
					return;
				}
				actItem = m_tree1->GetNextChild(category, actCookie);
			}
		}
		// color palette information

		category = m_tree1->GetNextChild(m_tree1->GetRootItem(), rootCookie);
		if (category.IsOk()) {
			if (category.GetID() == evt.GetItem().GetID()) {
				dfedit->getColorPalette(m_tableColorPalette);
				displayTable(m_tableColorPalette);
				return;
			}
		}
		// main script
		category = m_tree1->GetNextChild(m_tree1->GetRootItem(), rootCookie);
		if (category.IsOk()) {

			if (category.GetID() == evt.GetItem().GetID()) {
				// main script was selected

				//m_console->GetValue();
				this->GetWindowChild(SCRIPTEDITWIND)->Show();
				this->GetWindowChild(SCRIPTEDITWIND)->SetLabel(dfScriptName + "main SET Script");

				if (!scriptWindow)
					m_console->AppendText("ERROR: instance of script window was not loaded!\n");
				else
					scriptWindow->ChangeValue(dfedit->SETref->getSETscript());

				displayTable(m_tableGeneral);
				m_tableGeneral->SetCellValue(0, 1, "script type");
				return;
			}
		}
		// if here no match or fucntion was found. Table keeps empty
		displayTable(m_tableGeneral);

	} else if (dfedit->PUPref) {
		if (category.IsOk()) {
			// check script selections

			wxTreeItemIdValue scriptIndexCookie;
			wxTreeItemId scriptIndexItem = m_tree1->GetFirstChild(category, scriptIndexCookie);
			for (int32_t scr = 0; scriptIndexItem.IsOk(); scr++) {

				if (scriptIndexItem.GetID() == evt.GetItem().GetID()) {
					// if here, script was selected!
					this->GetWindowChild(SCRIPTEDITWIND)->Show();
					this->GetWindowChild(SCRIPTEDITWIND)->SetLabel(dfScriptName + dfedit->PUPref->getScriptName(scr));
					if (!scriptWindow)
						m_console->AppendText("ERROR: instance of script window not loaded!\n");
					else
						scriptWindow->ChangeValue(dfedit->PUPref->getScript(scr));

					return;
				}

				// get next element
				scriptIndexItem = m_tree1->GetNextChild(category, scriptIndexCookie);
			}

		}

		// TODO next items 
	}

	// just show amount of content if its standard table
	m_tableGeneral->SetCellValue(0, 1, std::to_string(m_tree1->GetChildrenCount(evt.GetItem())));
	return;
}

// this function will display the correct table out of the others
// not very elegant, but it works
void cMain::displayTable(wxGrid* table)
{
	if (table->IsShown())
		return;

	// enter here all tables that are processed by the tree view
	m_tableGeneral->Hide();
	m_tableSETframes->Hide();
	m_tableSETviews->Hide();
	m_tableSETviewsObj->Hide();
	m_tableSETsceneHeader->Hide();
	m_tableColorPalette->Hide();
	m_tableSETactors->Hide();

	table->Show();
	leftPanel->Fit();
	splitter->UpdateSize();
	return;
}

// formats the table to a minimalistic design
void cMain::formatTable(wxGrid* table)
{
	table->SetGridLineColour(wxColour(128, 128, 128));
	table->SetColSize(0, 150);
	table->SetColSize(1, 150);
	table->HideColLabels();
	table->HideRowLabels();
	table->DisableDragRowSize();
}

void cMain::formatSETviewTable(wxGrid* table) {
	table->SetCellValue(0, 0, "View Name:");
	table->SetReadOnly(0, 0);

	table->SetCellValue(1, 0, "View ID:");
	table->SetReadOnly(1, 0);

	table->SetCellValue(2, 0, "Objects location:");
	table->SetReadOnly(2, 0);


	table->SetCellValue(3, 0, "View direction:");
	table->SetReadOnly(3, 0);

	table->SetCellValue(4, 0, "rotation as 1 Byte:");
	table->SetReadOnly(4, 0);

	table->SetCellValue(5, 0, "unknown Double2:");
	table->SetReadOnly(5, 0);

	// 0 is single, 1 or 2 is a pair and means each one
	table->SetCellValue(6, 0, "View Pair type:");
	table->SetReadOnly(6, 0);
}

void cMain::formatSETviewObjTable(wxGrid* table) {
	table->SetCellValue(0, 0, "Object Name:");
	table->SetReadOnly(0, 0);

	table->SetCellValue(1, 0, "Unknown Integer:");
	table->SetReadOnly(1, 0);

	table->SetCellValue(2, 0, "Unknown Short 1:");
	table->SetReadOnly(2, 0);

	table->SetCellValue(3, 0, "Unknown Short 2:");
	table->SetReadOnly(3, 0);

	table->SetCellValue(4, 0, "Click region start X:");
	table->SetReadOnly(4, 0);

	table->SetCellValue(5, 0, "Click region start Y:");
	table->SetReadOnly(5, 0);

	table->SetCellValue(6, 0, "Click region end X:");
	table->SetReadOnly(6, 0);

	table->SetCellValue(7, 0, "Click region end Y:");
	table->SetReadOnly(7, 0);

	table->SetCellValue(8, 0, "Location of Script:");
	table->SetReadOnly(8, 0);

}

void cMain::formatSETframeTable(wxGrid* table)
{
	table->SetCellValue(0, 0, "Frame Type:");
	table->SetReadOnly(0, 0);

	const char* strs[3]{ "motion","low res","hi res" };
	wxArrayString dsfsd;
	dsfsd.reserve(3);
	dsfsd.Add(strs[0]);
	dsfsd.Add(strs[1]);
	dsfsd.Add(strs[2]);
	wxGridCellChoiceEditor* test = new wxGridCellChoiceEditor(dsfsd);

	//table->SetCellValue(0, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].motionInfo));
	table->SetCellEditor(0, 1, test);

	table->SetCellValue(1, 0, "transition logic:");
	table->SetReadOnly(1, 0);

	table->SetCellValue(2, 0, "index in View Table:");
	table->SetReadOnly(2, 0);

	table->SetCellValue(3, 0, "frame container:");
	table->SetReadOnly(3, 0);

	table->SetCellValue(4, 0, "frame-Pair ID:");
	table->SetReadOnly(4, 0);

	table->SetCellValue(5, 0, "Position X:");
	table->SetReadOnly(5, 0);

	table->SetCellValue(6, 0, "Position Z:");
	table->SetReadOnly(6, 0);

	table->SetCellValue(7, 0, "Position Y:");
	table->SetReadOnly(7, 0);

	table->SetCellValue(8, 0, "Horizontal rotation:");
	table->SetReadOnly(8, 0);

	table->SetCellValue(9, 0, "Position X (short)");
	table->SetReadOnly(9, 0);

	table->SetCellValue(10, 0, "Position Z (short)");
	table->SetReadOnly(11, 0);

	table->SetCellValue(11, 0, "Position Y (short)");
	table->SetReadOnly(11, 0);

	table->SetCellValue(12, 0, "Rotation as 1 Byte:");
	table->SetReadOnly(12, 0);
	// also set read only for value. It will be changed automatically depending in the rotation set
	table->SetReadOnly(12, 1);
}

inline void cMain::formatSETsceneTable(wxGrid* table)
{
	table->SetCellValue(0, 0, "Scene name:");
	table->SetReadOnly(0, 0);
	table->SetCellValue(1, 0, "unknown Integer1:");
	table->SetReadOnly(1, 0);
	table->SetCellValue(2, 0, "position X (short):");
	table->SetReadOnly(2, 0);
	table->SetCellValue(3, 0, "position Z (short):");
	table->SetReadOnly(3, 0);
	table->SetCellValue(4, 0, "position Y (short):");
	table->SetReadOnly(4, 0);
	table->SetCellValue(5, 0, "Location View table:");
	table->SetReadOnly(5, 0);
	table->SetCellValue(6, 0, "Location right turn register:");
	table->SetReadOnly(6, 0);
	table->SetCellValue(7, 0, "Location left turn register:");
	table->SetReadOnly(7, 0);
	table->SetCellValue(8, 0, "Location Scene Script:");
	table->SetReadOnly(8, 0);
	table->SetCellValue(9, 0, "FROM VIEW HEADER");
	table->SetReadOnly(9, 0);
	table->SetCellValue(10, 0, "position X:");
	table->SetReadOnly(10, 0);
	table->SetCellValue(11, 0, "position Z:");
	table->SetReadOnly(11, 0);
	table->SetCellValue(12, 0, "position Y:");
	table->SetReadOnly(12, 0);
	table->SetCellValue(13, 0, "unknown Integer2:");
	table->SetReadOnly(13, 0);
	// lock the last one. should be changed via top (if even changed...)
	table->SetCellValue(14, 0, "Location to Script (again):");
	table->SetReadOnly(14, 0);
	table->SetReadOnly(14, 1);
}

inline void cMain::formatSETactorsTable(wxGrid* table) {
	table->SetCellValue(0, 0, "Name (ident):");
	table->SetReadOnly(0, 0);

	table->SetCellValue(1, 0, "unknown Integer:");
	table->SetReadOnly(1, 0);

	table->SetCellValue(2, 0, "unknown Short:");
	table->SetReadOnly(2, 0);

	table->SetCellValue(3, 0, "Position on axis X:");
	table->SetReadOnly(3, 0);

	table->SetCellValue(4, 0, "Position on axis Z:");
	table->SetReadOnly(4, 0);

	table->SetCellValue(5, 0, "Position on axis Y:");
	table->SetReadOnly(5, 0);
}

void cMain::OnSetUEref(wxCommandEvent& evt) {
	if (!dfedit) {
		wxMessageBox("No file loaded yet!", "No file", wxICON_INFORMATION | wxOK);
		return;
	}
	ueWindow = new cSETueCoords(this);
	ueWindow->Show();
}

bool cMain::checkErrors(int32_t errCode) {
	if (errCode) {
		m_console->AppendText("ERROR: ");
		m_console->AppendText(dfedit->getDFeditErrorMsg(errCode));
		m_console->AppendText('\n');
		return false;
	}
	return true;
}
