#pragma warning(disable:4996)
#pragma once

#include "../../libs/DFfile/DFfile/DFlib.h";
#include <iostream>

#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/grid.h>
#include <wx/treectrl.h>

#define DFVERSION "version 0.01"
#define DFKEY 0x2518153584131BAC


class DFedit
{
public:

	~DFedit() {
		delete SETref;
		delete SHPref;
		delete PUPref;

		delete fileRef;
	}

	class SETedit {
	public:
		SETedit (DFedit* DFeditRef, DFset* setRef) : DFeditRef(DFeditRef), setRef(setRef) {
			findHighestIDs();
		}

		std::string printSETroads();
		void printSETscenes(wxTextCtrl* text);
		std::string printSETroad(int32_t road) {
			return setRef->transitionTable.getTransitionData(road);
		}
		bool checkFrameRegister() {
			if (setRef->scenes[0].rotationRegister[0].frameInfos[0].frameContainerLoc == setRef->scenes[0].rotationRegister[1].frameInfos[0].frameContainerLoc) return true;
			return false;
		}


		void printHighestIDs(wxTextCtrl* text) {
			text->AppendText("last scene ID:  " + std::to_string(highestSETids.scene) + '\n');
			text->AppendText("last view ID:   " + std::to_string(highestSETids.view) + '\n');
			text->AppendText("last Frame:     " + std::to_string(highestSETids.frame) + '\n');
			text->AppendText("last Road:      " + std::to_string(highestSETids.road) + '\n');
			text->AppendText("last Direction: " + std::to_string(highestSETids.direction) + '\n');
		}
		void changeSETsceneCoordinates(wxTextCtrl* console, int32_t scene, double x, double z, double y, bool autoTransition = true);
		void changeSETrotationFrameCount(int32_t viewToView, int32_t frameDifference, int32_t scene);

		int32_t updateSETRessources() {

			std::string fileDir("img/");

			std::string first(fileDir + std::to_string(setRef->scenes[0].rotationRegister[0].frameInfos[0].frameContainerLoc) + ".bmp");

			// make sure file exists
			std::ifstream bmpFile(first, std::ios::binary);
			if (!bmpFile.good()) {
				// file for color palette missing
				return ERRINITIALCOLORS;
			}
			// update color palette based on first image
			DFeditRef->changeColorPalette(first);
			bmpFile.close();
			// update container data
			// scene rotation frames
			for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
				for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[0].frameCount; frame++) {

					std::string fileName(fileDir + std::to_string(setRef->scenes[scene].rotationRegister[0].frameInfos[frame].frameContainerLoc) + ".bmp");

					if (int32_t error = DFeditRef->changeImageContainer(setRef->scenes[scene].rotationRegister[0].frameInfos[frame].containerFrame, fileName))
						return error;
				}
			}
			// road frames
			for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
				for (uint8_t dir = 0; dir < 2; dir++) {
					for (int32_t frame = 0; frame < setRef->transitionTable.transitions[road].frameRegister[dir].frameCount; frame++) {
						std::string fileName(fileDir + std::to_string(setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].frameContainerLoc) + ".bmp");

						if (int32_t error = DFeditRef->changeImageContainer(setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].containerFrame, fileName))
							return error;
					}
				}
			}
      
			return 0;
		}

		std::string addSETscene(double x, double z, double y);

		std::string addSETviews(int32_t scene, double degreese, bool doubleFrame);

		bool addSETobject(int32_t scene, int32_t view, const std::string& name);

		bool removeSETObject(int32_t scene, int32_t view, int32_t obj) {
			// sanity checks
			if (!setRef->scenes[scene].sceneViews[view].objTable) return false;
			if (obj >= setRef->scenes[scene].sceneViews[view].objTable->objectCount) return false;

			setRef->scenes[scene].sceneViews[view].removeObject(obj);
			return true;
		}
		void removeSETviews(int32_t scene, int32_t viewID) {

			// first check if its a double or single frame
			if (setRef->scenes[scene].sceneViews[viewID].viewPairType) {
				// apparently yes. delete pair view first and corresponding frames

				int8_t offset;
				if (setRef->scenes[scene].sceneViews[viewID].viewPairType == 1) {
					offset = 1;
				}
				else {
					// type 2 expected
					offset = -1;
				}
				removeSETsingleView(scene, viewID + offset);

				// position will shift if we deleted an element before. adapt index
				if (offset == -1) viewID--;
			}

			removeSETsingleView(scene, viewID);

			refitSETrotationFrames(scene);
		}


		std::string addSETroad(int32_t sceneFrom, int32_t viewFrom, int32_t sceneTo, int32_t viewTo) {
			setRef->transitionTable.transitionCount++;
			setRef->transitionTable.transitions.reserve(setRef->transitionTable.transitionCount);
			setRef->transitionTable.transitions.emplace_back();

			// just add some psuedo numbers
			//setRef->transitionTable.transitions.back().locationTransitionInfo = ++highestSETids.direction;
			//setRef->transitionTable.transitions.back().locationSceneA = ++highestSETids.direction;
			//setRef->transitionTable.transitions.back().locationSceneB = ++highestSETids.direction;
			// seems always 1 and -1
			setRef->transitionTable.transitions.back().unknownShort1 = 1;
			setRef->transitionTable.transitions.back().unknownShort2 = -1;

			setRef->transitionTable.transitions.back().viewIDstart = setRef->scenes[sceneFrom].sceneViews[viewFrom].viewID;
			setRef->transitionTable.transitions.back().viewIDend = setRef->scenes[sceneTo].sceneViews[viewTo].viewID;

			setRef->transitionTable.transitions.back().xAxisStart = setRef->scenes[sceneFrom].sceneLocation[0];
			setRef->transitionTable.transitions.back().zAxisStart = setRef->scenes[sceneFrom].sceneLocation[1];
			setRef->transitionTable.transitions.back().yAxisStart = setRef->scenes[sceneFrom].sceneLocation[2];
			setRef->transitionTable.transitions.back().xAxisEnd = setRef->scenes[sceneTo].sceneLocation[0];
			setRef->transitionTable.transitions.back().zAxisEnd = setRef->scenes[sceneTo].sceneLocation[1];
			setRef->transitionTable.transitions.back().yAxisEnd = setRef->scenes[sceneTo].sceneLocation[2];

			std::string name("Road");
			name.append(std::to_string(++highestSETids.road));
			setRef->transitionTable.transitions.back().transitionName = name;

			// before we add the initial and final frame, lets update the rotation frames already
			// FROM

			for (uint8_t dir = 0; dir < 2; dir++) {
				setRef->transitionTable.transitions.back().frameRegister[dir].frameCount = 2;
				setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos.reserve(2);
				for (uint8_t frame = 0; frame < 2; frame++) {
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos.emplace_back();
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos[frame].motionInfo = 1 - frame;
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos[frame].transitionLog = 0;
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos[frame].viewID = -1;
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos[frame].frameContainerLoc = 0;
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos[frame].containerFrame.data = new uint8_t[8]{ 0 };
					setRef->transitionTable.transitions.back().frameRegister[dir].frameInfos[frame].containerFrame.size = 8;
				}
			}

			// start frame FROM
			setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[0].axisX = setRef->scenes[sceneFrom].sceneViews[viewFrom].rotation;
			setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[0].axisX8 = setRef->scenes[sceneFrom].sceneViews[viewFrom].rotation8;
			// end frame FROM
			setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[1].axisX = setRef->scenes[sceneTo].sceneViews[viewTo].rotation + PI;
			if (setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[1].axisX >= 2 * PI) {
				setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[1].axisX -= 2 * PI;
			}
			setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[1].axisX8 = setRef->scenes[sceneTo].sceneViews[viewTo].rotation8 + 128;
			if (setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[1].axisX8 >= 256) {
				setRef->transitionTable.transitions.back().frameRegister[0].frameInfos[1].axisX8 -= 256;
			}
			// other side should be calculated by the refit function

			refitSETtransitionFrames(setRef->transitionTable.transitionCount - 1, sceneFrom, sceneTo);
			return name;
		}
		void removeSETRoad(int32_t road) {
			// before deleting all corresponding data, remove the reference from the connected scenes (from its rotation tables)
			resetTransitionLog(setRef->transitionTable.transitions[road].viewIDstart);
			resetTransitionLog(setRef->transitionTable.transitions[road].viewIDend);

			// now the road is disconnected from the scenes and we can safely delete it
			for (int32_t frame = 0; frame < setRef->transitionTable.transitions[road].frameRegister[0].frameCount; frame++) {
				// delete all related frames, otherwise we have a memory leak
				setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].containerFrame.clearContent();
				setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].containerFrame.clearContent();
			}

			setRef->transitionTable.transitions[road].entries.clear();
			setRef->transitionTable.transitions[road].frameRegister[0].frameInfos.clear();
			setRef->transitionTable.transitions[road].frameRegister[1].frameInfos.clear();

			setRef->transitionTable.transitions.erase(setRef->transitionTable.transitions.begin() + road);
			setRef->transitionTable.transitionCount--;
			return;
		}

		void removeScene(int32_t scene) {
			// before the actual scene is deleted, remove all roads that connect to it.
			// leading roads to nowhere makes no sense and wouldnt work anyway

			std::vector<int32_t>roadID;
			for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
				for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
					if (setRef->transitionTable.transitions[road].viewIDstart == setRef->scenes[scene].sceneViews[view].viewID ||
						setRef->transitionTable.transitions[road].viewIDend == setRef->scenes[scene].sceneViews[view].viewID) {
						roadID.push_back(road);
					}
				}
			}

			for (int32_t road = 0; road < roadID.size(); road++) {
				removeSETRoad(roadID.at(road) - road);
			}


			// all connected roads should be removed. now lets delete all content related to the scene
			// frames
			for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[0].frameCount; frame++) {
				setRef->scenes[scene].rotationRegister[0].frameInfos[frame].containerFrame.clearContent();
				setRef->scenes[scene].rotationRegister[1].frameInfos[frame].containerFrame.clearContent();
			}

			for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
				if (setRef->scenes[scene].sceneViews[view].objTable) {
					for (int32_t obj = 0; obj < setRef->scenes[scene].sceneViews[view].objTable->objectCount; obj++) {
						setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].containerObjScript.clearContent();
					}
					setRef->scenes[scene].sceneViews[view].objTable->objectEntries.clear();

					delete setRef->scenes[scene].sceneViews[view].objTable;
				}
			}


			// total
			setRef->scenes[scene].rotationRegister[0].frameInfos.clear();
			setRef->scenes[scene].rotationRegister[1].frameInfos.clear();
			setRef->scenes[scene].sceneViews.clear();
			setRef->scenes[scene].containerSceneScript.clearContent();

			setRef->scenes.erase(setRef->scenes.begin() + scene);
			setRef->setHeader.sceneCount--;
		}

		int32_t changeSETmap(const std::string& path);

		std::string getSETSceneName(int32_t setID) {
			return setRef->scenes[setID].sceneName;
		}
		int32_t getSETsceneCount() {
			return setRef->getSceneCount();
		}
		void getSETscenesCoordinates(int32_t sceneID, int32_t& x, int32_t& z) {
			x = setRef->scenes[sceneID].XaxisMap;
			z = setRef->scenes[sceneID].ZaxisMap;
			return;
		}
		void getSETscenesCoordinates(int32_t sceneID, double& x, double& z, double& y) {
			x = setRef->scenes[sceneID].sceneLocation[0];
			z = setRef->scenes[sceneID].sceneLocation[1];
			y = setRef->scenes[sceneID].sceneLocation[2];
			return;
		}
		const char* strs[3]{ "motion","low res","hi res" };
		void fillSETsceneFrameData(wxGrid* table, int32_t scene, int32_t direction, int32_t frame) {
			// This function is causing a memory leak but its insignificant small.
			// choice editor cant be deleted without causing crashes at some point as well as its strings
			// should be fixed at some point

			int16_t viewCount = setRef->scenes[scene].SceneViewCount;
			// fill view strings plus None
			wxString* viewID = new wxString[viewCount + 1];
			viewID[0] = "None";
			for (int16_t view = 1; view < viewCount + 1; view++)
				viewID[view] = setRef->scenes[scene].sceneViews[view - 1].viewName;

			//static wxGridCellChoiceEditor* viewSel = nullptr;
			//if (viewSel) {
			//	viewSel->Destroy();
			//	delete viewSel;
			//}
			//	
			wxGridCellChoiceEditor* viewSel = new wxGridCellChoiceEditor(viewCount + 1, viewID);
			//viewSel->
			//table->SetCellValue(0, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].motionInfo));

			//table->Show();
			table->SetCellValue(0, 1, strs[setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].motionInfo]);
			table->SetCellValue(1, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].transitionLog));

			table->SetCellEditor(2, 1, viewSel);
			if (setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].viewID == -1)
				table->SetCellValue(2, 1, "None");
			else
				table->SetCellValue(2, 1, setRef->scenes[scene].sceneViews[setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].viewID].viewName);

			//table->SetCellValue(2, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].viewID));
			table->SetCellValue(3, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].frameContainerLoc));
			table->SetCellValue(4, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].framePairID));
			table->SetCellValue(5, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].posX));
			table->SetCellValue(6, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].posZ));
			table->SetCellValue(7, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].posY));
			table->SetCellValue(8, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].axisX * 180 / PI));
			table->SetCellValue(9, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].posX16));
			table->SetCellValue(10, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].posZ16));
			table->SetCellValue(11, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].posY16));
			table->SetCellValue(12, 1, std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[frame].axisX8));

			//delete[] viewID;
		}
		void fillSETroadFrameData(wxGrid* table, int32_t road, int32_t direction, int32_t frame) {
			// always clear table before filling in new content

			//table->Show();
			table->SetCellValue(0, 1, strs[setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].motionInfo]);
			table->SetCellValue(1, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].transitionLog));
			table->SetCellValue(2, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].viewID));
			table->SetCellValue(3, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].frameContainerLoc));
			table->SetCellValue(4, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].framePairID));
			table->SetCellValue(5, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].posX));
			table->SetCellValue(6, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].posZ));
			table->SetCellValue(7, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].posY));
			table->SetCellValue(8, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].axisX * 180 / PI));
			table->SetCellValue(9, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].posX16));
			table->SetCellValue(10, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].posZ16));
			table->SetCellValue(11, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].posY16));
			table->SetCellValue(12, 1, std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].axisX8));
		}
		void fillSETsceneViewData(wxGrid* table, int32_t scene, int32_t view) {

			table->SetCellValue(0, 1, setRef->scenes[scene].sceneViews[view].viewName);
			table->SetCellValue(1, 1, std::to_string(setRef->scenes[scene].sceneViews[view].viewID));
			table->SetCellValue(2, 1, std::to_string(setRef->scenes[scene].sceneViews[view].locationObjects));
			table->SetCellValue(3, 1, std::to_string(setRef->scenes[scene].sceneViews[view].rotation * 180 / PI));
			table->SetCellValue(4, 1, std::to_string(setRef->scenes[scene].sceneViews[view].rotation8));
			table->SetCellValue(5, 1, std::to_string(setRef->scenes[scene].sceneViews[view].unknownDB2));
			table->SetCellValue(6, 1, std::to_string(setRef->scenes[scene].sceneViews[view].viewPairType));
		}
		void fillSETsceneViewObjectData(wxGrid* table, int32_t scene, int32_t view, int32_t obj) {

			table->SetCellValue(0, 1, setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].identifier);
			table->SetCellValue(1, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].unknownInt));
			table->SetCellValue(2, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].rotation8));
			table->SetCellValue(3, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].unknownShort2));
			table->SetCellValue(4, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].startRegionX));
			table->SetCellValue(5, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].startRegionY));
			table->SetCellValue(6, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].endRegionX));
			table->SetCellValue(7, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].endRegionY));
			table->SetCellValue(8, 1, std::to_string(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].locationScript));
		}
		void fillSETsceneHeader(wxGrid* table, int32_t scene) {

			table->SetCellValue(0, 1, setRef->scenes[scene].sceneName);
			table->SetCellValue(1, 1, std::to_string(setRef->scenes[scene].unknownDWORD1));
			table->SetCellValue(2, 1, std::to_string(setRef->scenes[scene].XaxisMap));
			table->SetCellValue(3, 1, std::to_string(setRef->scenes[scene].ZaxisMap));
			table->SetCellValue(4, 1, std::to_string(setRef->scenes[scene].YaxisMap));
			table->SetCellValue(5, 1, std::to_string(setRef->scenes[scene].locationViews));
			table->SetCellValue(6, 1, std::to_string(setRef->scenes[scene].locationViewLogic[setRef->RIGHTTURNS]));
			table->SetCellValue(7, 1, std::to_string(setRef->scenes[scene].locationViewLogic[setRef->LEFTTURNS]));
			table->SetCellValue(8, 1, std::to_string(setRef->scenes[scene].locationScript));
			// gap
			table->SetCellValue(10, 1, std::to_string(setRef->scenes[scene].sceneLocation[0]));
			table->SetCellValue(11, 1, std::to_string(setRef->scenes[scene].sceneLocation[1]));
			table->SetCellValue(12, 1, std::to_string(setRef->scenes[scene].sceneLocation[2]));
			table->SetCellValue(13, 1, std::to_string(setRef->scenes[scene].unknownVal[0]));
			table->SetCellValue(14, 1, std::to_string(setRef->scenes[scene].unknownVal[1]));

		}
		void fillSETactorsTable(wxGrid* table, int32_t act) {
			table->SetCellValue(0, 1, setRef->actors.actors[act].identifier);
			table->SetCellValue(1, 1, std::to_string(setRef->actors.actors[act].unknownInt));
			table->SetCellValue(2, 1, std::to_string(setRef->actors.actors[act].rotation8));
			table->SetCellValue(3, 1, std::to_string(setRef->actors.actors[act].positionX));
			table->SetCellValue(4, 1, std::to_string(setRef->actors.actors[act].positionZ));
			table->SetCellValue(5, 1, std::to_string(setRef->actors.actors[act].positionY));
		}
		void fillSETtransitionFrameInfos(wxListBox* viewList, std::vector<int32_t>& frameCount, int32_t scene) {
			// reset before filling
			viewList->Clear();
			frameCount.clear();
			// we know how many already so why not
			frameCount.reserve(setRef->scenes[scene].SceneViewCount);
			int32_t counter = 0;

			// make sure first frame is a view
			if (setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].viewID == -1)
				return;
			// load initial frame, which is always a view
			std::string cbName(setRef->scenes[scene].sceneViews[setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].viewID].viewName);
			cbName.append(" to ");

			// iterate though the rest
			int32_t totalFrames = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount;
			for (int32_t frame = 1; frame <= totalFrames; frame++) {
				if (setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame % totalFrames].viewID == -1) {
					counter++;
				}
				else {
					std::string secName(setRef->scenes[scene].sceneViews[setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame % totalFrames].viewID].viewName);
					cbName.append(secName);
					viewList->Append(cbName);
					frameCount.emplace_back(counter);
					// build next name
					cbName.assign(secName);
					cbName.append(" to ");
					// reset counter
					counter = 0;
				}
			}


		}
		void fillSETtree(wxTreeCtrl* tree, wxTreeItemId& setID) {

			// read in scene data
			wxTreeItemId scenesCAT = tree->AppendItem(setID, "Scenes");

			for (int32_t scene = 0; scene < getSETsceneCount(); scene++) {
				wxTreeItemId sceneID = tree->AppendItem(scenesCAT, getSETSceneName(scene));

				wxTreeItemId views = tree->AppendItem(sceneID, "Views");
				wxTreeItemId turnIDs[2];
				turnIDs[0] = tree->AppendItem(sceneID, "right turns");
				turnIDs[1] = tree->AppendItem(sceneID, "left turns");
				tree->AppendItem(sceneID, "Scene Script");

				// get view data
				for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
					wxTreeItemId viewItem = tree->AppendItem(views, setRef->scenes[scene].sceneViews[view].viewName);
					// check if this view has an object table
					if (setRef->scenes[scene].sceneViews[view].locationObjects) {
						// if here, there is. apply all objects to view
						for (int32_t obj = 0; obj < setRef->scenes[scene].sceneViews[view].objTable->objectCount; obj++) {
							tree->AppendItem(viewItem, setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].identifier);
						}
					}
				}

				// get turn rotation data
				for (uint8_t direction = 0; direction < 2; direction++) {
					for (int32_t turn = 0; turn < setRef->scenes[scene].rotationRegister[direction].frameCount; turn++) {
						std::string fr("container ");
						fr.append(std::to_string(setRef->scenes[scene].rotationRegister[direction].frameInfos[turn].frameContainerLoc));
						tree->AppendItem(turnIDs[direction], fr);
					}
				}
			}

			wxTreeItemId roadCAT = tree->AppendItem(setID, "Roads");
			for (int32_t road = 0; road < getSETRoadCount(); road++) {
				wxTreeItemId roadID = tree->AppendItem(roadCAT, getSETRoadName(road));
				wxTreeItemId dirIDs[2];
				wxString sceneNames[2];
				int32_t scenes[2]{ -1,-1 };

				setRef->getScenesRoad(road, scenes[0], scenes[1]);
				for (uint8_t sc = 0; sc < 2; sc++) {
					if (scenes[sc] != -1)
						sceneNames[sc] = setRef->scenes[scenes[sc]].sceneName;
					else
						sceneNames[sc] = "UNKNOWN";
				}

				dirIDs[0] = tree->AppendItem(roadID, sceneNames[0] + " to " + sceneNames[1]);
				dirIDs[1] = tree->AppendItem(roadID, sceneNames[1] + " to " + sceneNames[0]);

				// sort frames to X TO X scene
				for (uint8_t direction = 0; direction < 2; direction++) {
					for (int32_t frame = 0; frame < setRef->transitionTable.transitions[road].frameRegister[direction].frameCount; frame++) {
						std::string fr("container ");
						fr.append(std::to_string(setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[frame].frameContainerLoc));
						tree->AppendItem(dirIDs[direction], fr);
					}
				}

				// get frame container location
				//for (int32_t rFrame = 0; rFrame < dfedit->getSETRoadFrameCount(road); rFrame++) {
				//	std::string fr("container ");
				//	m_tree1->AppendItem(roadID, fr);
				//}
			}

			wxTreeItemId actCAT = tree->AppendItem(setID, "Actors");
			for (int32_t act = 0; act < getActorsCount(); act++) {
				tree->AppendItem(actCAT, getActorsIdent(act));
			}
			tree->AppendItem(setID, "Color palette");
			tree->AppendItem(setID, "Set Script");
			// I wanted these to be expanded already:
			tree->Expand(setID);
			tree->Expand(scenesCAT);
			tree->Expand(roadCAT);
			tree->Expand(actCAT);
		}
		bool changeSETscript(DFfile::Container* container, const std::string& newScript) {
			container->clearContent();
			if (setRef->scripter.TextScriptToBinary(container->data, container->size, newScript)) return false;
			return true;
		}
		std::string getSETsceneViewObjectScript(int32_t scene, int32_t view, int32_t obj) {
			DFeditRef->lastSelectedContainer = &setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].containerObjScript;
			return setRef->scripter.binaryScriptToText(setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].containerObjScript.data);
		}
		int32_t getSETsceneViewCount(int32_t scene) {
			return setRef->scenes[scene].SceneViewCount;
		}
		std::string getSETsceneViewName(int32_t scene, int32_t viewID) {
			return setRef->scenes[scene].sceneViews[viewID].viewName;
		}
		double getSETviewRotation(int32_t scene, int32_t viewID) {
			// find the correct frame first
			for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[0].frameCount; frame++) {
				if (viewID == setRef->scenes[scene].rotationRegister[0].frameInfos[frame].viewID)
					return setRef->scenes[scene].rotationRegister[0].frameInfos[frame].axisX;
			}
			// not found. indice error with -1
			return -1.0f;
		}
		int32_t getSETsceneFrameCount(int32_t scene) {
			return setRef->scenes[scene].rotationRegister[0].frameCount;
		}
		int32_t getSETsceneFrameRotation(int32_t scene, int32_t frame, double& rotation) {
			rotation = setRef->scenes[scene].rotationRegister[0].frameInfos[frame].axisX;
			return setRef->scenes[scene].rotationRegister[0].frameInfos[frame].viewID;
		}
		std::string getSceneScript(int32_t scene) {
			DFeditRef->lastSelectedContainer = &setRef->scenes[scene].containerSceneScript;
			return setRef->scripter.binaryScriptToText(setRef->scenes[scene].containerSceneScript.data);
		}
		std::string getSETscript() {
			DFeditRef->lastSelectedContainer = &setRef->setHeader.containerMainScript;
			return setRef->scripter.binaryScriptToText(setRef->setHeader.containerMainScript.data);
		}
		float getSETmapRatio() {
			float width = setRef->setHeader.mapHeight;
			return width / setRef->setHeader.setDimensionsY;
		}
		void getCoordRatios(double& toInt, double& toDbl) {
			toInt = setRef->setHeader.setDimensionsY / setRef->setHeader.setDimensionsYf;
			toDbl = setRef->setHeader.setDimensionsYf / setRef->setHeader.setDimensionsY;
		}
		int32_t getSETRoadCount() {
			return setRef->transitionTable.transitionCount;
		}
		int32_t getSETRoadFrameCount(int32_t road) {
			return setRef->transitionTable.transitions[road].frameRegister[0].frameCount;
		}
		std::string getSETRoadName(int32_t road) {
			return setRef->transitionTable.transitions[road].transitionName;
		}
		void getSETroadCoords(int32_t roadID, int32_t& xStart, int32_t& zStart, int32_t& xEnd, int32_t& zEnd) {
			int32_t sceneFrom = -1;
			int32_t sceneTo = -1;
			setRef->getScenesRoad(roadID, sceneFrom, sceneTo);

			if (sceneTo != -1 && sceneFrom != -1) {
				xStart = setRef->scenes[sceneFrom].XaxisMap;
				zStart = setRef->scenes[sceneFrom].ZaxisMap;
				xEnd = setRef->scenes[sceneTo].XaxisMap;
				zEnd = setRef->scenes[sceneTo].ZaxisMap;
				return;
			}
			// if not found
			xStart = 0;
			zStart = 0;
			xEnd = 0;
			zEnd = 0;
		}
		void getSETroadFrameCoords(int32_t road, int32_t frame, int32_t& x, int32_t& z) {
			x = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posX16;
			z = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posZ16;
		}
		void getSETroadFrameCoords(int32_t road, int32_t frame, double& x, double& z, double& y, double& rotation) {
			x = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posX;
			z = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posZ;
			y = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posY;
			rotation = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX;
		}
		int32_t getActorsCount() {
			return setRef->actors.actorsCount;
		}
		std::string getActorsIdent(int32_t actor) {
			return setRef->actors.actors[actor].identifier;
		}
		void getActorsPosition(int32_t actor, int32_t& x, int32_t& z) {
			x = setRef->actors.actors[actor].positionX;
			z = setRef->actors.actors[actor].positionZ;
			return;
		}

		// fixes blurry images by only referencing to the hi res images
		void patchRotationTables();

		void clearResources();
		void clearContainers() {
			setRef->updateContainerInfo();
			setRef->clearSETcontainers();
		}

		void getSETmap(uint8_t*& data, uint32_t& imageSize) {
			setRef->colorCount = 256;
			setRef->getBMPimage(setRef->setHeader.containermapDark, data, imageSize);
			setRef->colorCount = 128;	// reset to 128 colors. only  for the map we need the full variety
			return;
		}
		bool getFrame(uint8_t*& data, uint32_t& imageSize, int32_t scene, int32_t viewID) {
			// start finding the container first
			// use left register because that one has the "sharp" frames
			for (int32_t i = 0; i < setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameCount; i++) {
				if (setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[i].viewID == viewID) {
					setRef->getBMPimage(setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[i].containerFrame, data, imageSize);
					return true;
				}
			}
			// container not found :(
			return false;
		}
		void getSETobjRegion(int32_t scene, int32_t view, int32_t obj, int16_t& startX, int16_t& startY, int16_t& endX, int16_t& endY) {
			// check if obj exists
			if (!setRef->scenes[scene].sceneViews[view].objTable) return;
			if (setRef->scenes[scene].sceneViews[view].objTable->objectCount <= obj) return;
			startX = setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].startRegionX;
			startY = setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].startRegionY;
			endX = setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].endRegionX;
			endY = setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].endRegionY;
		}
		void setSETobjRegion(int32_t scene, int32_t view, int32_t obj, int16_t startX, int16_t startY, int16_t endX, int16_t endY) {
			// check if obj exists
			setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].startRegionX = startX;
			setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].startRegionY = startY;
			setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].endRegionX = endX;
			setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].endRegionY = endY;
		}
		int32_t getSETobjectCount(int32_t scene, int32_t view) {
			if (setRef->scenes[scene].sceneViews[view].objTable)
				return setRef->scenes[scene].sceneViews[view].objTable->objectCount;
			return 0;
		}
		std::string getSETobjName(int32_t scene, int32_t view, int32_t obj) {
			return setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].identifier;
		}

		bool reconstructContainers(bool onlyOneRotationRegister = false);
		int32_t addZimageContainer(DFfile::Container& container, const std::string& imageFile);

		// UNREAL ENGINE SPECIFIC FUNCTIONS
		void addUEcoordinates(float x, float z, float y, float ratio = 0);
		void getUEcoordinates(float& x, float& z, float& y, float& ratio, std::string& defaultRatioOut) {
			//float defaultRatio = 18.937789 / setRef->setHeader.coords[0] * 9.3354;
			float defaultRatio = 17.0f / setRef->setHeader.coords[0] * 9.3354;

			if (memcmp(setRef->fileHeader.unknown2 + 471 + 256, "DFED\04D\0", 8)) {
				// no data set yet
				x = 0.0f;
				z = 0.0f;
				y = 0.0f;
				ratio = defaultRatio;
			}
			else {
				// data found. use what was saved before
				x = *(float*)(setRef->fileHeader.unknown2 + 471 + 256 + 8);
				z = *(float*)(setRef->fileHeader.unknown2 + 471 + 256 + 12);
				y = *(float*)(setRef->fileHeader.unknown2 + 471 + 256 + 16);
				ratio = *(float*)(setRef->fileHeader.unknown2 + 471 + 256 + 20);
			}
			defaultRatioOut.assign(std::to_string(defaultRatio));
			return;
		}

	private:
		DFedit* DFeditRef;
		DFset* setRef;

		struct HighestSETIDs {
			int32_t scene = 0;
			int32_t view = 0;
			int32_t frame = 0;
			int32_t road = 0;
			int32_t direction = 0;
		};

		HighestSETIDs highestSETids;

		void findHighestIDs() {


			for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
				std::string numb;
				for (int32_t ch = 0; ch < setRef->scenes[scene].sceneName.length(); ch++) {
					if (setRef->scenes[scene].sceneName.at(ch) >= '0' && setRef->scenes[scene].sceneName.at(ch) <= '9') {
						numb.push_back(setRef->scenes[scene].sceneName.at(ch));
					}
				}
				int32_t sceneID = 0;
				if (!numb.empty())
					sceneID = stoi(numb);
				if (sceneID > highestSETids.scene)
					highestSETids.scene = sceneID;



				numb.clear();
				for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
					for (int32_t ch = 0; ch < setRef->scenes[scene].sceneViews[view].viewName.length(); ch++) {
						if (setRef->scenes[scene].sceneViews[view].viewName.at(ch) >= '0' && setRef->scenes[scene].sceneViews[view].viewName.at(ch) <= '9') {
							numb.push_back(setRef->scenes[scene].sceneViews[view].viewName.at(ch));
						}
					}
					int32_t viewID = 0;
					if (!numb.empty())
						viewID = stoi(numb);
					if (viewID > highestSETids.view)
						highestSETids.view = viewID;
					numb.clear();
				}

				// one direction is enough because they share the frame ID
				for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount; frame++)
					if (setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].framePairID > highestSETids.frame)
						highestSETids.frame = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].framePairID;
			}


			for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
				// used to reference transitions to correct scenes
				if (setRef->transitionTable.transitions[road].locationSceneB > highestSETids.direction)
					highestSETids.direction = setRef->transitionTable.transitions[road].locationSceneB;

				std::string numb;
				for (int32_t ch = 0; ch < setRef->transitionTable.transitions[road].transitionName.length(); ch++) {
					if (setRef->transitionTable.transitions[road].transitionName.at(ch) >= '0' && setRef->transitionTable.transitions[road].transitionName.at(ch) <= '9') {
						numb.push_back(setRef->transitionTable.transitions[road].transitionName.at(ch));
					}
				}
				int32_t roadID = 0;
				if (!numb.empty())
					roadID = stoi(numb);
				if (roadID > highestSETids.road)
					highestSETids.road = roadID;

				for (uint8_t dir = 0; dir < 2; dir++)
					for (int32_t frame = 0; frame < setRef->transitionTable.transitions[road].frameRegister[dir].frameCount; frame++)
						if (setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].framePairID > highestSETids.frame)
							highestSETids.frame = setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].framePairID;
			}


		}

		void updateSETmainSceneRegister(DFfile::Container& container);
		void updateSETactorRegister(DFfile::Container& container);
		void updateContainerSETframeregister(int32_t& containerID);
		void updateSETviewRegister(DFfile::Container& container, int32_t scene);
		void refitSETrotationFrames(int32_t scene);
		bool refitSETtransitionFrames(int32_t fromScene, int32_t toScene, int32_t road);

		inline void resetTransitionLog(int32_t viewID) {
			for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
				for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
					if (viewID == setRef->scenes[scene].sceneViews[view].viewID) {

						// for both directions left and right
						for (uint8_t dir = 0; dir < 2; dir++) {
							for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
								if (setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].viewID == view) {
									setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].transitionLog = 0;
									break;
								}
							}
						}
						return;
					}
				}
			}
		}

		std::string addSETSingleView(int32_t scene, double degreese);

		void removeSETsingleView(int32_t scene, int32_t viewID) {
			// now the actual selected frame
			// delete frame
			for (uint8_t dir = 0; dir < 2; dir++) {
				for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
					if (setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].viewID == viewID) {
						setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].containerFrame.clearContent();
						setRef->scenes[scene].rotationRegister[dir].frameInfos.erase(setRef->scenes[scene].rotationRegister[dir].frameInfos.begin() + frame);
						setRef->scenes[scene].rotationRegister[dir].frameCount--;
						break;
					}
				}
			}
			// this will change the viewIndex in the frame register... update those that needs to be
			for (uint8_t dir = 0; dir < 2; dir++) {
				for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
					if (setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].viewID >= viewID) {
						setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].viewID--;
					}
				}
			}
			if (setRef->scenes[scene].sceneViews[viewID].objTable) {
				for (int32_t obj = 0; obj < setRef->scenes[scene].sceneViews[viewID].objTable->objectCount; obj++) {
					setRef->scenes[scene].sceneViews[viewID].objTable->objectEntries[obj].containerObjScript.clearContent();
				}
				delete setRef->scenes[scene].sceneViews[viewID].objTable;
			}
			setRef->scenes[scene].sceneViews.erase(setRef->scenes[scene].sceneViews.begin() + viewID);
			setRef->scenes[scene].SceneViewCount--;
		}

		void reconstructSceneData(std::vector<DFfile::Container>& containerRef, int32_t scene, bool onlyOneRotationRegister = false);
		void reconstructRoadData(std::vector<DFfile::Container>& containerRef, int32_t road);
	};

	class SHPedit {
	public:
		SHPedit(DFedit* DFeditRef, DFshp* shpRef) : DFeditRef(DFeditRef), shpRef(shpRef) {}

		bool addImage(const std::string& groupName, const std::string& imageFilePath, const std::string& newIdentifier, int16_t ypos, int16_t xpos) {

			// find group
			int32_t grpID = -1;
			for (int32_t grp = 0; grp < shpRef->objGroups.size(); grp++) {
				if (!groupName.compare(shpRef->objGroups.at(grp).name)) {
					grpID = grp;
					break;
				}
			}

			if (grpID == -1) return false;

			int32_t lastConainerID = shpRef->objGroups.at(grpID).entries.back().subEntries.back().location + 1;

			// header container
			shpRef->containers.insert(shpRef->containers.begin() + lastConainerID, DFfile::Container());
			shpRef->fileHeader.containerCount++;
			// actual image container
			shpRef->containers.insert(shpRef->containers.begin() + lastConainerID, DFfile::Container());
			shpRef->fileHeader.containerCount++;

			shpRef->containers.at(lastConainerID).size = shpRef->containers.at(lastConainerID - 2).size;
			shpRef->containers.at(lastConainerID).data = new uint8_t[shpRef->containers.at(lastConainerID).size];
			memcpy(shpRef->containers.at(lastConainerID).data, shpRef->containers.at(lastConainerID - 2).data, shpRef->containers.at(lastConainerID).size);

			shpRef->containers.at(lastConainerID + 1).data = new uint8_t[8]{ 0 };
			shpRef->containers.at(lastConainerID + 1).size = 8;
			DFeditRef->changeTransImageContainer(shpRef->containers.at(lastConainerID + 1), imageFilePath, ypos, xpos);

			// add new item
			shpRef->objGroups.at(grpID).entries.push_back(shpRef->objGroups.at(grpID).entries.back());
			shpRef->objGroups.at(grpID).entries.back().location = lastConainerID;
			shpRef->objGroups.at(grpID).entries.back().identifier = newIdentifier;
			shpRef->objGroups.at(grpID).entries.back().subEntries.back().location = lastConainerID + 1;
			*(int32_t*)(shpRef->containers.at(shpRef->objGroups.at(grpID).entries.back().location).data + 118) = lastConainerID + 1;

			// update count
			*(int32_t*)(shpRef->containers.at(shpRef->objGroups.at(grpID).location).data + 90) = shpRef->objGroups.at(grpID).entries.size();

			// update subIndex
			uint8_t* updatedGrpList = new uint8_t[shpRef->containers.at(shpRef->objGroups.at(grpID).location).size + 32];
			memcpy(updatedGrpList, shpRef->containers.at(shpRef->objGroups.at(grpID).location).data, shpRef->containers.at(shpRef->objGroups.at(grpID).location).size);

			int32_t pos = shpRef->containers.at(shpRef->objGroups.at(grpID).location).size;

			memcpy(updatedGrpList + pos, &shpRef->objGroups.at(grpID).entries.back().location, 4 * sizeof(int32_t));
			pos += 4 * sizeof(int32_t);
			*(updatedGrpList + pos++) = (uint8_t)shpRef->objGroups.at(grpID).entries.back().identifier.size();
			memset(updatedGrpList + pos, 0, 15);
			memcpy(updatedGrpList + pos, shpRef->objGroups.at(grpID).entries.back().identifier.c_str(), shpRef->objGroups.at(grpID).entries.back().identifier.size());
			shpRef->containers.at(shpRef->objGroups.at(grpID).location).clearContent();
			shpRef->containers.at(shpRef->objGroups.at(grpID).location).data = updatedGrpList;
			shpRef->containers.at(shpRef->objGroups.at(grpID).location).size += 32;

			// update locations
			for (int32_t grp = grpID + 1; grp < shpRef->objGroups.size(); grp++) {
				uint8_t* curr = shpRef->containers[0].data + 2364 + grp * 16;
				shpRef->objGroups.at(grp).location += 2;
				*(int32_t*)curr = shpRef->objGroups.at(grp).location;
				curr += 16;

				shpRef->objGroups.at(grp).scriptContainerLocation += 2;
				*(int32_t*)(shpRef->containers.at(shpRef->objGroups.at(grp).location).data + 38) = shpRef->objGroups.at(grp).scriptContainerLocation;

				for (int32_t grpEntries = 0; grpEntries < shpRef->objGroups.at(grp).entries.size(); grpEntries++) {
					shpRef->objGroups.at(grp).entries.at(grpEntries).location += 2;

					*(int32_t*)(shpRef->containers[shpRef->objGroups.at(grp).location].data + 94 + (grpEntries * 32)) = shpRef->objGroups.at(grp).entries.at(grpEntries).location;

					for (int32_t subEntries = 0; subEntries < shpRef->objGroups.at(grp).entries.at(grpEntries).subEntries.size(); subEntries++) {
						shpRef->objGroups.at(grp).entries.at(grpEntries).subEntries.at(subEntries).location += 2;
						*(int32_t*)(shpRef->containers[shpRef->objGroups.at(grp).entries.at(grpEntries).location].data + 118 + (44 * subEntries)) = shpRef->objGroups.at(grp).entries.at(grpEntries).subEntries.at(subEntries).location;
					}
				}
			}


			return true;
		}
	private:
		DFedit* DFeditRef;
		DFshp* shpRef;
	};

	class PUPedit {
	public:
		PUPedit(DFedit* DFeditRef, DFpup* pupRef) : DFeditRef(DFeditRef), pupRef(pupRef) {}

		std::string getScriptName(int32_t index) {
			return pupRef->pupScripts.at(index).scriptName;
		}

		std::string getScript(int32_t index) {
			DFeditRef->lastSelectedContainer = &pupRef->containers.at(pupRef->pupScripts.at(index).location);
			return pupRef->scripter.binaryScriptToText(pupRef->containers.at(pupRef->pupScripts.at(index).location).data);
		}

		void fillPUPtree(wxTreeCtrl* tree, wxTreeItemId& setID) {
			
			// read in scene data
			wxTreeItemId scriptsCAT = tree->AppendItem(setID, "Scripts");

			for (int32_t scr = 0; scr < pupRef->pupScripts.size(); scr++) {
				std::string scrName(pupRef->pupScripts.at(scr).scriptName);
				scrName.append(" at ");
				// show me the location as well in the name
				scrName.append(std::to_string(pupRef->pupScripts.at(scr).location));
				tree->AppendItem(scriptsCAT, scrName);
			}

			wxTreeItemId speechesCAT = tree->AppendItem(setID, "Speech");
			for (int32_t speech = 0; speech < pupRef->pupData.size(); speech++) {
				wxTreeItemId speechID = tree->AppendItem(speechesCAT, pupRef->pupData.at(speech).ident);

				std::string speechLabes[2]{ "Audio", "Animation" };

				for (uint8_t label = 0; label < 2; label++) {
					std::string labelName(speechLabes[label]);
					labelName.append(" at ");
					// show me the location as well in the name
					labelName.append(std::to_string(pupRef->pupData.at(speech).audioLocation+label));
					tree->AppendItem(speechID, labelName);
				}
			}

			tree->AppendItem(setID, "HUD");
			tree->AppendItem(setID, "Color palette");
			// I wanted these to be expanded already:
			tree->Expand(setID);
			tree->Expand(scriptsCAT);
			tree->Expand(speechesCAT);
			
		}
	private:
		DFedit* DFeditRef;
		DFpup* pupRef;
	};

	class AUDIOedit {
	public:
		AUDIOedit(DFedit* DFeditRef, DFaudio* audioRef) : DFeditRef(DFeditRef), audioRef(audioRef) {}
		
		int32_t reconstructAudioData(const std::string& waveFile) {
			WaveHeader wh(waveFile);
			AudioContainerHeader audioHeader(wh.blockAlign, wh.bitsPerSample, wh.hertz, wh.audioDataSize);

			audioRef->containers.erase(audioRef->containers.begin()+2, audioRef->containers.end());

			for (uint32_t i = 0; i < wh.audioDataSize; i++) {
				*(uint8_t*)(wh.uncompressedData + i) >>= 1;
			}

			uint32_t audioPartsCount = wh.audioDataSize / audioHeader.chunkSize / 150.0f;
			audioHeader.chunksPos.reserve(150);
			for (uint32_t i = 0; i < 150; i++)
				audioHeader.chunksPos.emplace_back(48 + (150 * 4) + (i * audioHeader.chunkSize));

			audioHeader.uncompressedSize = 150.0f * audioHeader.chunkSize;
			audioHeader.compressedDataSize = 150.0f * audioHeader.chunkSize;
			for (uint32_t i = 0; i < audioPartsCount; i++) {
				if (audioRef->containers.size() <= (i + 2))
					audioRef->containers.push_back(DFfile::Container());

				audioRef->containers.at(i + 2).clearContent();
				audioRef->containers.at(i + 2).size = audioHeader.getHeaderSize() + audioHeader.compressedDataSize;
				audioRef->containers.at(i + 2).data = new uint8_t[audioRef->containers.at(i + 2).size];

				audioHeader.getAudioHeader(audioRef->containers.at(i + 2).data);
				memcpy(audioRef->containers.at(i + 2).data + audioHeader.getHeaderSize(), wh.uncompressedData + (i * 150 * audioHeader.chunkSize), audioHeader.uncompressedSize);
			}

			// deal with remainder
			audioHeader.uncompressedSize = wh.audioDataSize - (audioPartsCount* audioHeader.uncompressedSize);
			uint32_t fillupByteCount = audioHeader.uncompressedSize % audioHeader.chunkSize;
			if (fillupByteCount) {
				fillupByteCount = audioHeader.chunkSize - fillupByteCount;
			}

			audioHeader.uncompressedSize += fillupByteCount;
			audioHeader.compressedDataSize = audioHeader.uncompressedSize;

			uint32_t chunkCount = audioHeader.uncompressedSize / audioHeader.chunkSize;

			audioHeader.chunksPos.clear();
			audioHeader.chunksPos.reserve(chunkCount);
			for (uint32_t i = 0; i < chunkCount; i++)
				audioHeader.chunksPos.emplace_back(48 + (chunkCount * 4) + (i * audioHeader.chunkSize));

			if (audioRef->containers.size() <= (audioPartsCount + 2))
				audioRef->containers.push_back(DFfile::Container());

			audioRef->containers.at(audioPartsCount + 2).clearContent();
			audioRef->containers.at(audioPartsCount + 2).size = audioHeader.getHeaderSize() + audioHeader.compressedDataSize;
			audioRef->containers.at(audioPartsCount + 2).data = new uint8_t[audioRef->containers.at(audioPartsCount + 2).size];
			void* test = audioRef->containers.at(audioPartsCount + 2).data;
			audioHeader.getAudioHeader(audioRef->containers.at(audioPartsCount + 2).data);
			memcpy(audioRef->containers.at(audioPartsCount + 2).data + audioHeader.getHeaderSize(), wh.uncompressedData + (audioPartsCount * 150 * audioHeader.chunkSize), audioHeader.uncompressedSize - fillupByteCount);
			memset(audioRef->containers.at(audioPartsCount + 2).data + audioHeader.getHeaderSize()+ audioHeader.uncompressedSize - fillupByteCount,0, fillupByteCount);

			// for some reason it wants an empty container in the end
			audioRef->containers.push_back(DFfile::Container());
			audioRef->containers.back().data = new uint8_t[8]{ 0 };
			audioRef->containers.back().size = 8;
			audioRef->fileHeader.containerCount = audioRef->containers.size();

			// update head data
			// CONTAINER 0
			// container count minus header
			*(int32_t*)(audioRef->containers.at(0).data + 32) = audioRef->containers.size()-1;
			// ident
			std::string newFileName;
			std::string::size_type pos = waveFile.rfind('/');
			std::string::size_type pos2 = waveFile.rfind('\\');
			if (pos == std::string::npos && pos2 == std::string::npos) {
				newFileName.assign(waveFile.substr(0, waveFile.length()-4));
			}
			else {
				newFileName.assign(waveFile.substr(pos+1, waveFile.length()-pos-4));
			}
			newFileName.append("trk");
			if (newFileName.length() > 15) {
				// error, file name too long
				return 110;
			}
			*(audioRef->containers.at(0).data + 36) = newFileName.size();
			memcpy(audioRef->containers.at(0).data + 37, newFileName.data(), newFileName.size());
			
			// CONTAINER 1
			const uint16_t soundContainerCount = audioRef->containers.size() - 3;
			uint8_t* newContainerData = new uint8_t[270 + 26 * soundContainerCount]{ 0 };
			memcpy(newContainerData, audioRef->containers.at(1).data, 270);
			*(uint16_t*)(newContainerData + 4) = soundContainerCount;
			*(uint32_t*)(newContainerData + 266) = soundContainerCount;

			
			for (uint16_t i = 0; i < soundContainerCount; i++) {
				// upper loop table
				*(uint16_t*)(newContainerData + 6 + (i*2)) = i+1;
				// lower string table
				*(uint32_t*)(newContainerData + 274 + (i * 26)) = i + 2;

				std::string partName(newFileName.substr(0, newFileName.length() - 4));
				partName.push_back('.');
				partName.append(std::to_string(i+1));
				*(newContainerData + 280 + (i * 26)) = partName.length();
				memcpy(newContainerData + 281 + (i * 26), partName.data(), partName.length());
			}

			audioRef->containers.at(1).clearContent();
			audioRef->containers.at(1).size = 270 + 26 * soundContainerCount;
			audioRef->containers.at(1).data = newContainerData;

			return NOERRORS;
		}

		int32_t swapAudioContainer(int32_t containerID, const std::string& path) {

			// STEP ONE: Read wave file with and varify its head data
			WaveHeader wh(path);
			
			// STEP TWO: Encode the data based on the wave file data
			int8_t* compressedData;

			AudioContainerHeader audioHeader(wh.blockAlign, wh.bitsPerSample, wh.hertz, wh.audioDataSize);
			audioEncoder(wh.uncompressedData, compressedData, audioHeader);

			// STEP THREE: Glue new header with data together and replace containers content with it.
			audioRef->containers.at(containerID).clearContent();
			audioRef->containers.at(containerID).size = audioHeader.getHeaderSize() + audioHeader.compressedDataSize;
			audioRef->containers.at(containerID).data = new uint8_t[audioRef->containers.at(containerID).size];

			audioHeader.getAudioHeader(audioRef->containers.at(containerID).data);
			memcpy(audioRef->containers.at(containerID).data + audioHeader.getHeaderSize(), compressedData, audioHeader.compressedDataSize);

			delete[] compressedData;

			return NOERRORS;
		}

	private:

		struct WaveHeader {
			WaveHeader(const std::string& path) {
				std::ifstream waveFile(path, std::ios::binary);
				if (!waveFile.good()) {
					// FILE NOT FOUND
					return;
					waveFile.close();
				}

				waveFile.read(fchars, 4);
				waveFile.read((char*)&fileSize, 4);
				waveFile.read(qchars, 8);
				if (memcmp(fchars, "RIFF", 4) || memcmp(qchars, "WAVEfmt ", 8)) {
					// "WRONG AUDIO FORMAT"
					return;
					waveFile.close();
				}
				waveFile.read((char*)&headSize, 4);
				waveFile.read((char*)&audioFormat, 2);
				if (audioFormat != 1) {
					// return error "MUST BE PCM FORMAT!"
					return;
					waveFile.close();
				}
				waveFile.read((char*)&monoOrStereo, 2);
				waveFile.read((char*)&hertz, 4);
				waveFile.read((char*)&byteRate, 4);
				waveFile.read((char*)&blockAlign, 2);
				waveFile.read((char*)&bitsPerSample, 2);
				char subInfoC[4];
				waveFile.read(subInfoC, 4);
				if (memcmp(subInfoC, "data", 4)) {
					int32_t subInfoSize;
					waveFile.read((char*)&subInfoSize, 4);
					waveFile.ignore(subInfoSize + 4);
				}

				waveFile.read((char*)&audioDataSize, 4);

				uncompressedData = new int8_t[audioDataSize];
				waveFile.read((char*)uncompressedData, audioDataSize);
				waveFile.close();
			}

			~WaveHeader() {
				delete[] uncompressedData;
			}

			char fchars[4];
			char qchars[8];
			int32_t fileSize;
			int32_t audioDataSize;
			int32_t headSize;
			int16_t audioFormat;    // must be 1 for PCM
			int16_t monoOrStereo;	// 1 = mono, 2 = stereo
			int32_t hertz;			// can be anything?
			int32_t byteRate;		// sample rate * numChannels* bitsPerSample / 8
			int16_t blockAlign;     // number of channels * bitsPerSample / 8
			int16_t bitsPerSample;  // bits per sample, 8 or 16 allowed

			int8_t* uncompressedData;

		};

		struct AudioContainerHeader {
			AudioContainerHeader(int16_t stereo, int16_t bitsPerSample, int32_t hertz, int32_t uncompressedSize)
				: stereo(stereo), bitsPerSample(bitsPerSample), hertz(hertz), uncompressedSize(uncompressedSize) {
				chunkSize = 1024.0f/22050.0f* hertz * (bitsPerSample/8);
			}

			void getAudioHeader(uint8_t* dest) {
				*(int16_t*)dest = 0;
				dest += 2;
				*(int16_t*)dest = version;	// version
				dest += 2;
				memset(dest, 0, 20);
				dest += 20;
				*(int16_t*)dest = stereo >> 1;
				dest += 2;
				*(int16_t*)dest = bitsPerSample/8;
				dest += 2;
				*(int32_t*)dest = hertz;
				dest += 4;
				*(int32_t*)dest = chunkSize;
				dest += 4;
				*(int32_t*)dest = uncompressedSize;
				dest += 4;
				*(int32_t*)dest = chunksPos.size();
				dest += 4;

				for (auto& c : chunksPos) {
					*(int32_t*)dest = c;
					dest += 4;
				}
				// file end
				*(int32_t*)dest = compressedDataSize + 48 + (chunksPos.size() * 4); // total size
				dest += 4;
			}

			uint32_t getHeaderSize() {
				return 48 + (chunksPos.size() * 4);
			}
			const int16_t version = 1;
			const bool stereo;
			const int16_t bitsPerSample;
			//const int16_t sampleSize;
			const int32_t hertz;
			int32_t uncompressedSize;
			int32_t compressedDataSize;

			std::vector<int32_t> chunksPos;
			int32_t chunkSize;
		};
		// encodes audio container data
		int32_t audioEncoder(int8_t* uncompressedData, int8_t* &encodedOutput, AudioContainerHeader& audioHeader) {
		
			static constexpr int8_t StepSizeTable[256]{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, //  0
				0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	0x01, 0x01, 0x01, 0x01, //  1
				0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,	0x02, 0x02, 0x02, 0x02, 0x02, 0x02,	//  2
				0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,	//  3
				0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	//  4
				0x05, 0x05, 0x05, 0x05,	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,	0x05, 0x05,	//  5
				0x06, 0x06,	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,	//  6
				0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,	0x07, 0x07,	//  7
				0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,	0xF8, 0xF8, 0xF8, 0xF8,	// -8
				0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9,	0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9,	// -7
				0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	// -6
				0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	// -5
				0xFC, 0xFC, 0xFC, 0xFC,	0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,	// -4
				0xFD, 0xFD,	0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD,	// -3
				0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,	0xFE, 0xFE,	// -2
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	0xFF, 0xFF, 0xFF, 0xFF	// -1
			};

			static constexpr int8_t IndexTable[256]{
				// 0     1     2     3     4     5     6     7    -8    -7    -6    -5    -4    -3    -2    -1
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD,	0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB,	0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9,	0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03,	0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01,	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD,	0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB,	0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9,	0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05,	0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03,	0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01,	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD,	0xFE, 0xFF,
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xF8, 0xF9, 0xFA, 0xFB,	0xFC, 0xFD, 0xFE, 0xFF
			};

			std::vector<int8_t>output;
			// we dont need that much, but it saves keep doing new memory allocations
			output.reserve(audioHeader.uncompressedSize);
			// remove offset
			for (uint32_t i = 0; i < audioHeader.uncompressedSize; i++) {
				//uncompressedData[i] -= 0x40;
				*(uint8_t*)(uncompressedData+i) >>= 1;
			}

			//int8_t m2buffer[65];
			//uint8_t m2buffsize = 0;
			uint8_t m3counter = 0;	// check how many in a row are the same
			uint8_t m2counter = 0;	// 
			int8_t prevByte = uncompressedData[0];
			output.push_back(prevByte);
			bool bNotEqual = false;
			bool bNoTable = false;
			//int8_t* uCurr = uncompressedData + 1;
			int8_t* test = output.data();
			//bool noMatchFound = false;
			//bool missingByte = false;
			//bool noMatchSecByte = false;
			// TEMPORARY
			audioHeader.chunksPos.resize(audioHeader.uncompressedSize / audioHeader.chunkSize);
			for (uint32_t i = 0; i < audioHeader.chunksPos.size(); i++)
				audioHeader.chunksPos[i] = 48 + (audioHeader.chunksPos.size()*4) + (i* audioHeader.chunkSize);
			for (uint32_t i = 1; i < audioHeader.uncompressedSize; i++) {

				// lazy mode
				output.emplace_back(uncompressedData[i]);
				// analyze

				prevByte = uncompressedData[i];
			}
      
			// TODO: Process rest mode3counter if given

			audioHeader.compressedDataSize = output.size();
			encodedOutput = new int8_t[audioHeader.compressedDataSize];
			memcpy(encodedOutput, output.data(), audioHeader.compressedDataSize);
			output.clear();

			return NOERRORS;
		}

		DFedit* DFeditRef;
		DFaudio* audioRef;
	};

	SETedit* SETref = nullptr;
	SHPedit* SHPref = nullptr;
	PUPedit* PUPref = nullptr;
	AUDIOedit* AUDIOref = nullptr;

	// help variable for script editor
	DFfile::Container* lastSelectedContainer = nullptr;

	std::string getDFeditErrorMsg(int32_t& errorCode);
	
	std::string getFileName() {
		return fileRef->currentFileName;
	}


	// general operations
	void swapImage(int32_t containerID, const std::string& file) {
		// todo: evaluate correct function(s)
		changeImageContainer(fileRef->containers[containerID], file);
	}

	void swapTransImage(int32_t containerID, const std::string& file, int16_t y = -1, int16_t x = -1) {
		changeTransImageContainer(fileRef->containers[containerID], file, y, x);
	}

	void updateSCRIPT(int32_t container, const std::string& textFile) {

		std::ifstream t(textFile, std::ios::binary);
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		//std::string buffer(size, ' ');
		std::string buffer(size, 0);
		//buffer.reserve(size+1);
		//buffer.resize(size + 1);
		t.seekg(0);
		t.read(&buffer[0], size);
		t.close();

		fileRef->scripter.TextScriptToBinary(fileRef->containers.at(container).data, fileRef->containers.at(container).size, buffer);
		addDFeditLabel();
		addDFeditKey();
	}

	void getColorPalette(wxGrid* table) {

		// fill color information
		for (uint8_t row = 0; row < 16; row++) {
			for (uint8_t col = 0; col < 16; col++) {
				table->SetCellBackgroundColour(row, col, wxColour(
					*(((uint8_t*)&fileRef->colors[col + (row * 16)].RGB[0]) + 1),
					*(((uint8_t*)&fileRef->colors[col + (row * 16)].RGB[1]) + 1),
					*(((uint8_t*)&fileRef->colors[col + (row * 16)].RGB[2]) + 1)
				));
			}
		}
	}


	void addDFeditLabel();
	void setDFeditMode(int32_t mode) {
		*(int32_t*)(fileRef->fileHeader.unknown2 + 471 + 476) = mode;
	}


	// MOV FILE OPERATIONS
	void printMOVframeInfo();
	void createNewStaticMOVfile(const std::string& newFileName);
	void changeImageWithPessedOKBtn(uint32_t conainerIDfrom, uint32_t containerIDto);


	// GENERAL CONTAINER OPERATIONS
	void printHeaderInfo();
	std::string printContainerInfo() {
		return fileRef->getContainerInfo();
	}

	void changeColorPalette(const std::string& imageFile);


	int32_t loadFile(const std::string& file) {
		DFfile* directfileRef;
		if (int32_t err = readDFfile(directfileRef, file)) {
			return err;
		}
		fileRef = directfileRef;
		if (DFset::getDFset(fileRef)) {
			SETref = new SETedit(this, (DFset*)fileRef);
		}
		else if (DFshp::getDFshp(fileRef)) {
			SHPref = new SHPedit(this, (DFshp*)fileRef);
		}
		else if (DFpup::getDFpup(fileRef)) {
			PUPref = new PUPedit(this, (DFpup*)fileRef);
		}
		else if (DFaudio::getDFaudio(fileRef)) {
			AUDIOref = new AUDIOedit(this, (DFaudio*)fileRef);
		}
		else {
			return ERRFILENOSUPPORT;
		}
		return NOERRORS;
	}

	void updateAllContainers(const std::string& newFileName) {
		// check what kind of file to create by suffix

		std::string suffix = newFileName.substr(newFileName.length() - 3, 3);

		if (!suffix.compare("MOV")) {
			updateMOV();

		}
		else if (!suffix.compare("SET")) {
			// TODO
		}
		else {
			std::cout << "file type not supported!\n";
		}
	}

	void writePNGimage(const std::string& path, int32_t containerID) {
		fileRef->writePNGimage(path, containerID);
		//fileRef->writeBMPimage(11, "NEW");
	}


	int32_t writeContainersToFiles(const std::string& whereTo) {
		if (fileRef) {
			fileRef->writeContainersToFiles(whereTo);
			return NOERRORS;
		}
		return ERRNOFILEMOUNTED;
	}
	// takes current containers in ram and writes them to a new file
	bool writeNewFile(const std::string& to);
	const char* readScriptFile(const std::string& location, uint32_t container);

private:

	enum ErrorCodes {
		NOERRORS,
		// errors between 0 and 100 are internal DFfile errors with its own error handler
		ERRNOFILEMOUNTED = 100,
		ERRFILENOSUPPORT,
		ERRFILENOTFOUND,
		ERRINITIALCOLORS,
		ERRPLACEHOLDER4,
		ERRPLACEHOLDER5,

	};


	
	DFfile* fileRef = nullptr;


	std::string additionalErrorInfo;

	// find highest IDs for possible adding operations


	void addDFeditKey() {
		*(int64_t*)(fileRef->fileHeader.unknown2 + 471 + 480) = DFKEY;
	}

	inline bool checkContainerBounds(uint32_t& containerID);
	inline void changeContainerContent(uint32_t containerID, uint8_t* content, uint32_t containerSize);
	inline void changeContainerContent(DFfile::Container& containerOld, uint8_t* newContent, uint32_t containerSize);

	void insertNewContainer(uint32_t& containerID, uint8_t* content, uint32_t containerSize);
	void removeContainer(uint32_t& containerID);
	void copyContainerContent(int32_t containerIDfrom, int32_t containerIDto);
	void eraseContainerContent(int32_t& containerID);
	void eraseContainerContent(DFfile::Container& containerRef);

	void addContainer(std::vector<DFfile::Container>& containerRef, uint8_t* content, uint32_t containerSize);
	void addContainer(std::vector<DFfile::Container>& containerRef, DFfile::Container& container);

	void addEmptyContainer(std::vector<DFfile::Container>& containerRef);
	void addEmptyContainer(std::vector<DFfile::Container>& containerRef, int32_t count);

	void updateMOV();


	int32_t changeImageContainer(DFfile::Container& container, const std::string& imageFile);
	bool changeTransImageContainer(DFfile::Container& container, const std::string& imageFile, int16_t offsetX = -1, int16_t offsetY = -1);
	
};

