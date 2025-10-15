#include "cApp.h"

wxIMPLEMENT_APP(cApp);

cApp::cApp() {

}

cApp::~cApp() {

}


bool cApp::OnInit() {
	wxInitAllImageHandlers();	// support all image formats

	// actual main window
	m_frame1 = new cMain();
	m_frame1->Show();

	// call this first so it can prepare the text winow
	m_scrCtrl = new cScrCtrl(m_frame1);
	m_objCtrl = new cObjEdit(m_frame1);
	m_roadCtrl = new cRoadEdit(m_frame1);

	m_frame1->scriptWindow = m_scrCtrl->scriptWnd;

	return true;
}
