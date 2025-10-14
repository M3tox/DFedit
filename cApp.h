#pragma once

#include "cMain.h"
#include "cScriptCtrl.h"
#include "cObjEdit.h"
#include "cRoadEdit.h"

class cApp : public wxApp
{
public:
	cApp();
	~cApp();

private:
	cMain* m_frame1 = nullptr;
	cScrCtrl* m_scrCtrl = nullptr;
	cObjEdit* m_objCtrl = nullptr;
	cRoadEdit* m_roadCtrl = nullptr;
	
public:
	virtual bool OnInit();
};
