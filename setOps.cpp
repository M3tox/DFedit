#include "DFedit.h"

std::string DFedit::SETedit::printSETroads() {
	return setRef->transitionTable.getTransitionData();
}

void DFedit::SETedit::printSETscenes(wxTextCtrl* text) {
	//std::streambuf* sbOld = std::cout.rdbuf();
	//std::cout.rdbuf(text);
	for (uint32_t scene = 0; scene < setRef->getSceneCount(); scene++) {
		text->AppendText(setRef->scenes[scene].getSceneInfo());
		//std::cout << setRef->scenes[scene].getSceneInfo() << std::endl;
		//std::cout << set->scenes[scene].getTurnData() << std::endl;
		//std::cout << set->scenes[scene].getViewData() << std::endl;
	}
	//std::cout.rdbuf(sbOld);
}


void DFedit::SETedit::patchRotationTables() {

	// patch scenes
	for (uint32_t scene = 0; scene < setRef->getSceneCount(); scene++) {
		setRef->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[0].frameContainerLoc = setRef->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameInfos[0].frameContainerLoc;
		//eraseContainerContent(set->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[0].frameContainerLoc);

		for (uint32_t frame = 1; frame < setRef->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameCount; frame++) {

			if (setRef->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[frame].motionInfo == 1) {
				// only modify if motion type 1
				// make sure the ref is not zero! (zombie scene)
				if (setRef->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[frame].frameContainerLoc == 0) {
					std::cout << setRef->currentFileName << '\n';
					std::cout << setRef->scenes[scene].sceneName << '\n';
					std::cout << "FRAME: " << frame << '\n';
					break;
				}
				DFeditRef->eraseContainerContent(setRef->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[frame].frameContainerLoc);
				//copyContainerContent(set->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameInfos[set->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameCount - frame].frameContainerLoc, set->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[frame].frameContainerLoc);

				setRef->scenes[scene].rotationRegister[DFset::RIGHTTURNS].frameInfos[frame].frameContainerLoc
					= setRef->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameInfos[setRef->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameCount - frame].frameContainerLoc;
			}

		}
		updateContainerSETframeregister(setRef->scenes[scene].locationViewLogic[DFset::RIGHTTURNS]);
	}

	// patch translational frames
	for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
		// TO
		// find correct view
		for (uint32_t scene = 0; scene < setRef->getSceneCount(); scene++) {

			// check for both directions
			for (uint8_t direction = 0; direction < 2; direction++) {
				if (setRef->transitionTable.transitions[road].frameRegister[direction].destination == setRef->scenes[scene].locationViews) {
					// if here, this is the correct scene we were looking for

					// find correct rotation
					for (int32_t turns = 0; turns < setRef->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameCount; turns++) {
						if (setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[setRef->transitionTable.transitions[road].frameRegister[0].frameCount - 1].axisX8
							== setRef->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameInfos[turns].axisX8) {
							// correct frame found! OVERWRITE
							setRef->transitionTable.transitions[road].frameRegister[direction].frameInfos[setRef->transitionTable.transitions[road].frameRegister[0].frameCount - 1].frameContainerLoc
								= setRef->scenes[scene].rotationRegister[DFset::LEFTTURNS].frameInfos[turns].frameContainerLoc;
							// frame not adressed anymore, clear up
							//eraseContainerContent(set->transitionTable.transitions[road].frameRegister[direction].frameInfos[set->transitionTable.transitions[road].frameRegister[0].frameCount - 1].frameContainerLoc);
							break;
						}
					}
				}
			}
		}
		// update container tables
		updateContainerSETframeregister(setRef->transitionTable.transitions[road].locationSceneA);
		updateContainerSETframeregister(setRef->transitionTable.transitions[road].locationSceneB);
	}


	memset(setRef->fileHeader.unknown2 + 471, 0, 268);
	memcpy(setRef->fileHeader.unknown2 + 471, "Modified with DFedit made by MeToX", 34);
	memcpy(setRef->fileHeader.unknown2 + 471 + 64, DFVERSION, 13);
	// put some random numbers
	memcpy(setRef->fileHeader.unknown2 + 471 + 256, "DFED", 4);
	*(uint64_t*)(setRef->fileHeader.unknown2 + 471 + 256 + 4) = 0x2518153500000000;
}

int32_t DFedit::SETedit::changeSETmap(const std::string& path) {
	// adapt bounds first
	// kinda redudant but we want to use the image function also for others
	std::ifstream bmpFile(path, std::ios::binary);
	if (!bmpFile.good()) {
		DFeditRef->additionalErrorInfo = path;
		return ERRFILENOTFOUND;
	}

	bmpFile.seekg(10);
	int32_t pixelpos;
	bmpFile.read((char*)&pixelpos, sizeof(pixelpos));
	bmpFile.ignore(4);

	// resolution (better to read from bmp instead)
	int16_t height;
	int16_t width;

	bmpFile.read((char*)&width, sizeof(width));
	bmpFile.ignore(2);
	bmpFile.read((char*)&height, sizeof(height));
	bmpFile.ignore(2);
	bmpFile.close();

	// If i dont store it in a double first we gonna have some very annoying differences
	double temp = setRef->setHeader.setDimensionsY;
	temp /= setRef->setHeader.mapHeight;
	temp *= height;
	setRef->setHeader.setDimensionsY = temp;

	temp = setRef->setHeader.setDimensionsX;
	temp /= setRef->setHeader.mapWidth;
	temp *= width;
	setRef->setHeader.setDimensionsX = temp;

	setRef->setHeader.setDimensionsYf = setRef->setHeader.setDimensionsYf / setRef->setHeader.mapHeight * height;
	setRef->setHeader.setDimensionsXf = setRef->setHeader.setDimensionsXf / setRef->setHeader.mapWidth * width;

	// now change the global resolution data for the map
	setRef->setHeader.mapWidth = width;
	setRef->setHeader.mapHeight = height;
	setRef->setHeader.setDimensionsY_2 = setRef->setHeader.setDimensionsY;

	return DFeditRef->changeImageContainer(setRef->setHeader.containermapDark, path);
}

int32_t DFedit::SETedit::addZimageContainer(DFfile::Container& container, const std::string& imageFile) {


	int32_t pixelBeg;
	int32_t width;
	int32_t height;

	std::ifstream zImage(imageFile, std::ios::binary);
	if (!zImage.good()) {
		DFeditRef->additionalErrorInfo = imageFile;
		return ERRFILENOTFOUND;
	}

	// jump to start of color palette
	zImage.seekg(10);
	zImage.read((char*)&pixelBeg, sizeof(pixelBeg));
	// skip sub header size
	zImage.ignore(4);

	zImage.read((char*)&width, sizeof(width));
	zImage.read((char*)&height, sizeof(height));

	zImage.seekg(pixelBeg);

	// read whole depth information into ram, so that I can read from it in the right order
	uint8_t* rawData = new uint8_t[height * width];
	for (int32_t h = 0; h < height; h++) {
		zImage.read((char*)rawData + ((height - (h + 1)) * width), width);
	}
	zImage.close();

	// give me some space to store the lookup table
	uint16_t* lookUpTable = new uint16_t[height];

	uint8_t* dataCurr = rawData;
	uint8_t pixelCounter = 0;

	//std::vector<uint8_t> compressedPixel;
	std::vector<uint8_t>* compressedPixel = new std::vector<uint8_t>[height];
	// initialize the compressed data length with the size of the 
	uint16_t totalDataLength{ (uint16_t)(height * 2) };

	for (int32_t h = 0; h < height; h++) {

		// set initial pixel per width
		uint8_t previousPixel = *dataCurr++;
		pixelCounter++;

		// start loop from second pixel per width
		for (int32_t w = 1; w < width; w++, pixelCounter++) {

			uint8_t pixel = *dataCurr++;
			if (pixel != previousPixel || pixelCounter == 0xFF) {

				compressedPixel[h].push_back(pixelCounter);
				compressedPixel[h].push_back(previousPixel);
				pixelCounter = 0;
			}
			previousPixel = pixel;
		}
		// Check what is left in this width and write it in
		if (pixelCounter) {
			compressedPixel[h].push_back(pixelCounter);
			compressedPixel[h].push_back(previousPixel);
			pixelCounter = 0;
		}

		// update lookupTable
		lookUpTable[h] = totalDataLength;
		totalDataLength += compressedPixel[h].size() + 1;
	}
	// we got everything compressed now, clear memory!
	delete[] rawData;

	// get me some ram for the new container
	uint8_t* newContainer = new uint8_t[totalDataLength + container.size];
	// merge regular image with additional Z image
	memcpy(newContainer, container.data, container.size);
	memcpy(newContainer + container.size, lookUpTable, lookUpTable[0]);
	dataCurr = newContainer + container.size + lookUpTable[0];

	for (int32_t h = 0; h < height; h++) {
		*dataCurr++ = compressedPixel[h].size() / 2;
		memcpy(dataCurr, compressedPixel[h].data(), compressedPixel[h].size());
		dataCurr += compressedPixel[h].size();
	}


	// new container constructed, delete the old one and replace it
	DFeditRef->changeContainerContent(container, newContainer, totalDataLength + container.size);

	delete[] compressedPixel;
	delete[] lookUpTable;

	return NOERRORS;
}

void DFedit::SETedit::updateContainerSETframeregister(int32_t& containerID) {

	// find correct scene
	for (uint32_t scene = 0; scene < setRef->getSceneCount(); scene++) {
		for (uint8_t turns = 0; turns < 2; turns++) {
			if (setRef->scenes[scene].locationViewLogic[turns] == containerID) {
				// when here, it found the correct container
				DFeditRef->eraseContainerContent(containerID);
				int32_t newContainerSize = 12 + (60 * setRef->scenes[scene].rotationRegister[turns].frameCount);
				setRef->containers[containerID].data = new uint8_t[newContainerSize];
				setRef->containers[containerID].size = newContainerSize;
				setRef->scenes[scene].rotationRegister[turns].writeTransitionContainer(setRef->containers[containerID].data);
				return;
			}
		}
	}
	// if not rotation table, might be translational
	for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
		if (setRef->transitionTable.transitions[road].locationSceneA == containerID) {

			DFeditRef->eraseContainerContent(containerID);
			int32_t newContainerSize = 12 + (60 * setRef->transitionTable.transitions[road].frameRegister[0].frameCount);
			setRef->containers[containerID].data = new uint8_t[newContainerSize];
			setRef->containers[containerID].size = newContainerSize;
			setRef->transitionTable.transitions[road].frameRegister[0].writeTransitionContainer(setRef->containers[containerID].data);
			return;
		}
		else if (setRef->transitionTable.transitions[road].locationSceneB == containerID) {

			DFeditRef->eraseContainerContent(containerID);
			int32_t newContainerSize = 12 + (60 * setRef->transitionTable.transitions[road].frameRegister[1].frameCount);
			setRef->containers[containerID].data = new uint8_t[newContainerSize];
			setRef->containers[containerID].size = newContainerSize;
			setRef->transitionTable.transitions[road].frameRegister[1].writeTransitionContainer(setRef->containers[containerID].data);
			return;
		}
	}
	std::cout << "ERROR: No frame register matches the given container ID!!\n";
}

void DFedit::addDFeditLabel() {
	memset(fileRef->fileHeader.unknown2 + 471, 0, 144);
	memcpy(fileRef->fileHeader.unknown2 + 471, "Modified with DFedit made by MeToX", 34);
	memcpy(fileRef->fileHeader.unknown2 + 471 + 64, DFVERSION, 13);
}

void DFedit::SETedit::addUEcoordinates(float x, float z, float y, float ratio) {
	// put some random numbers
	memcpy(DFeditRef->fileRef->fileHeader.unknown2 + 471 + 256, "DFED\04D\0", 8);
	*(float*)(DFeditRef->fileRef->fileHeader.unknown2 + 471 + 256 + 8) = x;
	*(float*)(DFeditRef->fileRef->fileHeader.unknown2 + 471 + 256 + 12) = z;
	*(float*)(DFeditRef->fileRef->fileHeader.unknown2 + 471 + 256 + 16) = y;
	*(float*)(DFeditRef->fileRef->fileHeader.unknown2 + 471 + 256 + 20) = ratio;
}

std::string DFedit::SETedit::addSETscene(double x, double z, double y) {

	std::string newSceneName("Scene");
	newSceneName.append(std::to_string(++highestSETids.scene));
	setRef->setHeader.sceneCount++;
	setRef->scenes.reserve(setRef->setHeader.sceneCount);
	setRef->scenes.emplace_back();

	double temp;
	setRef->scenes.back().unknownDWORD1 = 0;
	temp = setRef->setHeader.setDimensionsX;
	temp /= setRef->setHeader.setDimensionsXf;
	setRef->scenes.back().XaxisMap = temp * x;
	setRef->scenes.back().ZaxisMap = temp * z;
	setRef->scenes.back().YaxisMap = temp * y;
	// we set a pseudo value here, just to keep track to which scene the transition goes
	setRef->scenes.back().locationViews = ++highestSETids.direction;
	setRef->scenes.back().locationViewLogic[0] = 0;
	setRef->scenes.back().locationViewLogic[1] = 0;
	setRef->scenes.back().locationScript = 0;
	setRef->scenes.back().sceneName = newSceneName;

	setRef->scenes.back().containerSceneScript.data = new uint8_t[8]{ 0 };
	setRef->scenes.back().containerSceneScript.size = 8;

	setRef->scenes.back().sceneLocation[0] = x;
	setRef->scenes.back().sceneLocation[1] = z;
	setRef->scenes.back().sceneLocation[2] = y;

	setRef->scenes.back().unknownVal[0] = 0;
	setRef->scenes.back().unknownVal[1] = 0;

	setRef->scenes.back().SceneViewCount = 0;

	for (uint8_t dir = 0; dir < 2; dir++) {
		setRef->scenes.back().rotationRegister[dir].destination = 0;
		setRef->scenes.back().rotationRegister[dir].frameCount = 0;
		setRef->scenes.back().rotationRegister[dir].unknownInt = 0;
	}

	// now add at least two views
	setRef->scenes.back().sceneViews.reserve(2);
	setRef->scenes.back().SceneViewCount = 2;
	for (int32_t view = 0; view < 2; view++) {
		setRef->scenes.back().sceneViews.emplace_back();

		setRef->scenes.back().sceneViews.back().viewID = ++highestSETids.view;
		std::string newViewName("View");
		newViewName.append(std::to_string(highestSETids.view));
		setRef->scenes.back().sceneViews.back().viewName = newViewName;
		setRef->scenes.back().sceneViews.back().locationObjects = 0;

		setRef->scenes.back().sceneViews.back().rotation = 0.0f + (view * PI);
		setRef->scenes.back().sceneViews.back().rotation8 = 0 + (view * 128);
		setRef->scenes.back().sceneViews.back().unknownDB2 = 25.0f;	// I dont know what this does :(
		setRef->scenes.back().sceneViews.back().viewPairType = view + 1;	// double indicator
	}


	for (uint8_t frame = 0; frame < 2; frame++) {
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].insertFrame(frame);
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].viewID = frame;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].motionInfo = 1;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].axisX = 0.0f + (frame * PI);
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].axisX8 = 0 + (frame * 128);

		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].posX = x;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].posZ = z;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].posY = y + setRef->setHeader.coords[0];
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].posX16 = setRef->scenes.back().XaxisMap;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].posZ16 = setRef->scenes.back().ZaxisMap;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].posY16 = setRef->scenes.back().YaxisMap + setRef->setHeader.heightDifference;

		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.data = new uint8_t[8]{ 0 };
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.size = 8;

		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].frameContainerLoc = 0;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].transitionLog = 0;
		setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameCount++;

		// just copy the new frame from the right table and change the motion info.
		setRef->scenes.back().rotationRegister[setRef->LEFTTURNS].insertFrame(frame, setRef->scenes.back().rotationRegister[setRef->RIGHTTURNS].frameInfos[frame]);
		setRef->scenes.back().rotationRegister[setRef->LEFTTURNS].frameInfos[frame].motionInfo = 2;

		setRef->scenes.back().rotationRegister[setRef->LEFTTURNS].frameInfos[frame].containerFrame.data = new uint8_t[8]{ 0 };
		setRef->scenes.back().rotationRegister[setRef->LEFTTURNS].frameInfos[frame].containerFrame.size = 8;

		setRef->scenes.back().rotationRegister[setRef->LEFTTURNS].frameCount++;
	}


	return newSceneName;
}

std::string DFedit::SETedit::addSETviews(int32_t scene, double degreese, bool doubleFrame) {
	std::string out(addSETSingleView(scene, degreese));

	if (doubleFrame) {
		out.append(" and ");
		out.append(addSETSingleView(scene, degreese + 180.0f));

		int32_t viewCount = setRef->scenes[scene].SceneViewCount;
		setRef->scenes[scene].sceneViews[viewCount - 2].viewPairType = 1;
		setRef->scenes[scene].sceneViews[viewCount - 1].viewPairType = 2;
	}

	refitSETrotationFrames(scene);

	return out;
}

bool DFedit::SETedit::addSETobject(int32_t scene, int32_t view, const std::string& name) {
	// make sure object table is not missing
	setRef->scenes[scene].sceneViews[view].addNewObject(name);
	return true;
}

std::string DFedit::SETedit::addSETSingleView(int32_t scene, double degreese) {
	setRef->scenes[scene].SceneViewCount++;
	setRef->scenes[scene].sceneViews.reserve(setRef->scenes[scene].SceneViewCount);
	setRef->scenes[scene].sceneViews.emplace_back();

	setRef->scenes[scene].sceneViews.back().viewID = ++highestSETids.view;
	std::string newViewName("View");
	newViewName.append(std::to_string(highestSETids.view));
	setRef->scenes[scene].sceneViews.back().viewName = newViewName;
	setRef->scenes[scene].sceneViews.back().locationObjects = 0;

	setRef->scenes[scene].sceneViews.back().rotation = degreese / 180 * PI;
	double temp = 256;
	temp /= 360;
	setRef->scenes[scene].sceneViews.back().rotation8 = temp * degreese;
	setRef->scenes[scene].sceneViews.back().unknownDB2 = 25.0f;	// I dont know what this does :(
	setRef->scenes[scene].sceneViews.back().viewPairType = 0;	// single

	// insert new frame into rotation tables find where
	int32_t frameCount = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount;
	for (int32_t frame = 1; frame < frameCount + 1; frame++) {
		double extra = 0.0f;
		if (frameCount == frame) extra = 360.0f;
		if (setRef->scenes[scene].sceneViews.back().rotation < setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame % frameCount].axisX + extra) {
			// if here, we have oversteped the location and know where to insert the frame
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].insertFrame(frame, setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0]);
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].viewID = setRef->scenes[scene].SceneViewCount - 1;
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].motionInfo = 1;
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].axisX = setRef->scenes[scene].sceneViews.back().rotation;
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].axisX8 = setRef->scenes[scene].sceneViews.back().rotation8;

			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.data = new uint8_t[8]{ 0 };
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.size = 8;

			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].frameContainerLoc = 0;
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].transitionLog = 0;
			setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount++;

			// just copy the new frame from the right table and change the motion info.
			setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].insertFrame(frameCount - frame + 1, setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame]);
			setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame + 1].motionInfo = 2;

			setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame + 1].containerFrame.data = new uint8_t[8]{ 0 };
			setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame + 1].containerFrame.size = 8;

			setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameCount++;
			break;
		}
	}

	return newViewName;

}

void DFedit::SETedit::refitSETrotationFrames(int32_t scene) {

	// refit transition frames between views
	int32_t frameCount = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount;
	double temp, min, max;
	int32_t minPos = 0;

	min = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].axisX;
 	for (int32_t frame = 1; frame < frameCount + 1; frame++) {
		if (setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame % frameCount].viewID != -1) {
			max = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame % frameCount].axisX;

			if (max < min)
				max += 2 * PI;

			double n = (max - min) / (frame - minPos);
			for (int32_t tframe = minPos + 1; tframe < frame; tframe++) {
				double newRotation = min + (n * (tframe - minPos));
				if (newRotation >= 2 * PI) {
					newRotation -= 2 * PI;
				}
				setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[tframe % frameCount].axisX = newRotation;
				temp = 256;
				temp /= 2 * PI;
				setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[tframe % frameCount].axisX8 = temp * newRotation;
			}

			min = max;
			minPos = frame;
		}
	}

	// update other side as well and frame pair IDs
	setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].framePairID = ++highestSETids.frame;
	setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[0].framePairID = highestSETids.frame;
	for (int32_t frame = 1; frame < frameCount; frame++) {
		setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame].axisX = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].axisX;
		setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame].axisX8 = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].axisX8;

		setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].framePairID = ++highestSETids.frame;
		setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame].framePairID = highestSETids.frame;
	}
}

bool DFedit::SETedit::refitSETtransitionFrames(int32_t road, int32_t fromScene = -1, int32_t toScene = -1) {
	// find destination scene
	// and get its position to start linear interpolation
	
	if (toScene == -1 || fromScene == -1) {
		//int32_t toScene = -1;
		//int32_t fromScene = -1;
		for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
			if (setRef->transitionTable.transitions[road].frameRegister[0].destination == setRef->scenes[scene].locationViews) {
				toScene = scene;
				if (fromScene != -1) break;
			}
			if (setRef->transitionTable.transitions[road].frameRegister[1].destination == setRef->scenes[scene].locationViews) {
				fromScene = scene;
				if (toScene != -1) break;
			}
		}

		// if still -1 scene was not found
		if (toScene == -1 || fromScene == -1) return false;
	}

	
	int32_t frameCount = setRef->transitionTable.transitions[road].frameRegister[0].frameCount;
	// before we do anything, we must make sure that the path is filled with sufficient amount frames
	// MAKE SURE THE SCENES HAVE AT LEAST ONE FRAME! We want their coordinates
	// we could also use the scene pos instead, but that would cause a different height!
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX = setRef->scenes[fromScene].sceneLocation[0];
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ = setRef->scenes[fromScene].sceneLocation[1];
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY = setRef->scenes[fromScene].sceneLocation[2] + setRef->setHeader.coords[0];

	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX16 = setRef->scenes[fromScene].XaxisMap;
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ16 = setRef->scenes[fromScene].ZaxisMap;
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY16 = setRef->scenes[fromScene].YaxisMap + setRef->setHeader.heightDifference;

	double xLength = setRef->scenes[toScene].sceneLocation[0] - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX;
	xLength *= xLength;
	double zLength = setRef->scenes[toScene].sceneLocation[1] - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ;
	zLength *= zLength;
	double yLength = setRef->scenes[toScene].sceneLocation[2] + setRef->setHeader.coords[0] - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY;
	yLength *= yLength;
	double newLength = xLength + zLength + yLength;
	newLength = sqrt(newLength);

	// the frame count is the spaces between plus one
	int32_t newFrameCount = ceil(newLength / setRef->setHeader.coords[2]) + 1;

	if (newFrameCount > frameCount) {
		// need more frames for both sides
		for (uint8_t dir2 = 0; dir2 < 2; dir2++) {

			// we want the last element in the back. save it and emplace it later
			auto lastE = setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back();
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.reserve(newFrameCount);
			for (int32_t i = 0; i < newFrameCount - frameCount - 1; i++) {
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.emplace_back();
				//setRef->transitionTable.transitions[road].frameRegister[dir2].addNewFrame(0,0,0,-1);
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().motionInfo = 0;
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().frameContainerLoc = 0;
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().transitionLog = 0;
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().viewID = -1;
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().containerFrame.data = new uint8_t[8]{ 0 };
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().containerFrame.size = 8;

				// also refresh frame count!!
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameCount++;
			}
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.emplace_back(lastE);
			// we change the image to an empty container, but image data stays in the location from where we copied it from
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().containerFrame.data = new uint8_t[8]{ 0 };
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().containerFrame.size = 8;
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameCount++;

			// also give new IDs to each frame, even the old ones
			for (int32_t i = 0; i < newFrameCount; i++) {
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos[i].framePairID = ++highestSETids.frame;
			}
		}
	}
	else if (newFrameCount < frameCount) {
		// need to delete frames
		for (uint8_t dir2 = 0; dir2 < 2; dir2++) {
			double drehung = setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().axisX;
			int16_t drehung8 = setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().axisX8;
			for (int32_t i = 0; i < frameCount - newFrameCount; i++) {
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().containerFrame.clearContent();
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.pop_back();
				setRef->transitionTable.transitions[road].frameRegister[dir2].frameCount--;
			}
			// apply rotation from last frane to the new last frame
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().axisX = drehung;
			setRef->transitionTable.transitions[road].frameRegister[dir2].frameInfos.back().axisX8 = drehung8;
		}
	}

	frameCount = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos.size();
	frameCount--;

	double rotDiff = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].axisX - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].axisX;
	
	// check if difference is not 0
	if(rotDiff != 0.0f) {

		// normalize if minus
		if (rotDiff < 0)
			rotDiff += 2 * PI;

		if (rotDiff > PI) {
			// left turn
			rotDiff = 2 * PI - rotDiff;
			rotDiff = -rotDiff;
		}
	}

	rotDiff /= frameCount;
	for (int32_t frame = 1; frame < frameCount; frame++) {
		// 1st frame stays the same
		double diff = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posX16 - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX16;
		diff /= frameCount;
		diff *= frame;
		diff += setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX16;
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posX16 = diff;

		diff = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posX - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX;
		diff /= frameCount;
		diff *= frame;
		diff += setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posX;
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posX = diff;

		diff = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posZ16 - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ16;
		diff /= frameCount;
		diff *= frame;
		diff += setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ16;
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posZ16 = diff;

		diff = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posZ - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ;
		diff /= frameCount;
		diff *= frame;
		diff += setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posZ;
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posZ = diff;

		diff = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posY16 - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY16;
		diff /= frameCount;
		diff *= frame;
		diff += setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY16;
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posY16 = diff;

		diff = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posY - setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY;
		diff /= frameCount;
		diff *= frame;
		diff += setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].posY;
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].posY = diff;

		// positions should be interpolated at this point. Now do the same with the rotations

		// normalize to degreese
		diff = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[0].axisX + (rotDiff * frame);
		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX = diff;
		if (setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX >= 2 * PI)
			setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX -= 2 * PI;
		else if (setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX < 0)
			setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX += 2 * PI;

		setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX8 = 256.0f / (2 * PI) * setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frame].axisX;

	}
	// last frame matches scene. just take those
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].posX = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posX;
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].posZ = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posZ;
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].posY = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posY;

	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].posX16 = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posX16;
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].posZ16 = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posZ16;
	setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount].posY16 = setRef->scenes[toScene].rotationRegister[0].frameInfos[0].posY16;

	// now all we have to do is patching the other direction as well
	for (int32_t frame = 0; frame < frameCount + 1; frame++) {
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].posX = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].posX;
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].posZ = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].posZ;
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].posY = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].posY;

		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].posX16 = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].posX16;
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].posZ16 = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].posZ16;
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].posY16 = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].posY16;

		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].axisX;
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX += PI;
		if (setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX >= 2 * PI) {
			setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX -= 2 * PI;
		}

		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX8 = setRef->transitionTable.transitions[road].frameRegister[0].frameInfos[frameCount - frame].axisX8;
		setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX8 += 128;
		if (setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX8 > 255) {
			setRef->transitionTable.transitions[road].frameRegister[1].frameInfos[frame].axisX8 -= 256;
		}
	}
	return true;
}

void DFedit::SETedit::changeSETsceneCoordinates(wxTextCtrl* console, int32_t scene, double x, double z, double y, bool autoTransition) {

	double temp = setRef->scenes[scene].XaxisMap;
	temp /= setRef->scenes[scene].sceneLocation[0];
	temp *= x;
	setRef->scenes[scene].XaxisMap = temp;

	temp = setRef->scenes[scene].ZaxisMap;
	temp /= setRef->scenes[scene].sceneLocation[1];
	temp *= z;
	setRef->scenes[scene].ZaxisMap = temp;

	temp = setRef->scenes[scene].YaxisMap;
	temp /= setRef->scenes[scene].sceneLocation[2];
	temp *= y;
	setRef->scenes[scene].YaxisMap = temp;

	setRef->scenes[scene].sceneLocation[0] = x;
	setRef->scenes[scene].sceneLocation[1] = z;
	setRef->scenes[scene].sceneLocation[2] = y;


	// update frame coordinates as well...
	for (uint8_t dir = 0; dir < 2; dir++) {
		for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].posX = x;
			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].posZ = z;
			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].posY = y + setRef->setHeader.coords[0];

			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].posX16 = setRef->scenes[scene].XaxisMap;
			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].posZ16 = setRef->scenes[scene].ZaxisMap;
			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].posY16 = setRef->scenes[scene].YaxisMap + setRef->setHeader.heightDifference;
		}
	}

	// automate altering the transition frames if user wants that
	if (autoTransition) {
		for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
			// check for both directions
			for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
				if (setRef->transitionTable.transitions[road].viewIDstart == setRef->scenes[scene].sceneViews[view].viewID) {
					refitSETtransitionFrames(road);
					break;
				}
				if (setRef->transitionTable.transitions[road].viewIDend == setRef->scenes[scene].sceneViews[view].viewID) {
					refitSETtransitionFrames(road);
					break;
				}
			}
		}
	}
}

void DFedit::SETedit::changeSETrotationFrameCount(int32_t viewToView, int32_t frameDifference, int32_t scene) {
	// dont do anything if no change was done
	if (frameDifference) {
		int32_t current = 0;

		// make sure first frame is a view
		if (setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].viewID == -1)
			return;

		// iterate though the rest
		int32_t frameCount = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount;
		for (int32_t frame = 0; frame < frameCount; frame++) {
			if (setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].viewID != -1) {
				if (current == viewToView) {

					frame++;
					//frameCount;
					if (frameDifference < 0) {
						// remove frames
						while (frameDifference++) {
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.clearContent();
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos.erase(setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos.begin() + frame);
							// also the other side...
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame].containerFrame.clearContent();
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos.erase(setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos.begin() + frameCount - frame +1);
							frameCount++;
						}

						// make sure the frame pair ID is continues.
						int32_t pairID = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].framePairID;
						for (int32_t frame3 = 1; frame3 < frameCount; frame3++) {
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame3].framePairID = ++pairID;
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame3].framePairID = pairID;
						}

					}
					else {

						//setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos.reserve(frameCount+ frameDifference);
						while (frameDifference--) {
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].insertFrame(frame, setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame - 1]);
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].viewID = -1;
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].motionInfo = 0;
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.data = new uint8_t[8]{ 0 };
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame].containerFrame.size = 8;

							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].insertFrame(frameCount - frame+ 1, setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[0]);
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame +1].viewID = -1;
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame +1].motionInfo = 0;
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame + 1].containerFrame.data = new uint8_t[8]{ 0 };
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame + 1].containerFrame.size = 8;
							frameCount++;
						}
						// give new frame pair IDs
						for (int32_t frame3 = 1; frame3 < frameCount; frame3++) {
							setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[frame3].framePairID = ++highestSETids.frame;
							setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frameCount - frame3].framePairID = highestSETids.frame;
						}
					}

					setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount = frameCount;
					setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameCount = frameCount;
					refitSETrotationFrames(scene);

					return;
				}
				current++;
			}
		}
	}
}


void DFedit::SETedit::updateSETmainSceneRegister(DFfile::Container& container) {

	// prepare container data
	container.size = setRef->scenes.size() * 42;

	delete[] container.data;
	container.data = new uint8_t[container.size];

	// read each scene in
	for (uint32_t scene = 0; scene < setRef->scenes.size(); scene++) {
		uint8_t* curr = container.data + (scene * 42);

		*(int32_t*)(curr) = setRef->scenes[scene].unknownDWORD1;
		curr += 4;
		*(int16_t*)(curr) = setRef->scenes[scene].XaxisMap;
		curr += 2;
		*(int16_t*)(curr) = setRef->scenes[scene].ZaxisMap;
		curr += 2;
		*(int16_t*)(curr) = setRef->scenes[scene].YaxisMap;
		curr += 2;
		*(int32_t*)(curr) = setRef->scenes[scene].locationViews;
		curr += 4;
		*(int32_t*)(curr) = setRef->scenes[scene].locationViewLogic[setRef->RIGHTTURNS];
		curr += 4;
		*(int32_t*)(curr) = setRef->scenes[scene].locationViewLogic[setRef->LEFTTURNS];
		curr += 4;
		*(int32_t*)(curr) = setRef->scenes[scene].locationScript;
		curr += 4;
		*curr++ = setRef->scenes[scene].sceneName.size();
		memset(curr, 0, 15);	// write unused data 0...
		memcpy(curr, setRef->scenes[scene].sceneName.c_str(), *(curr - 1));
	}

}

void DFedit::SETedit::updateSETactorRegister(DFfile::Container& container) {
	// prepare container data
	container.size = 8 + setRef->actors.actorsCount * 54;

	delete[] container.data;
	container.data = new uint8_t[container.size];

	uint8_t* curr = container.data;
	*(int32_t*)curr = setRef->actors.actorsCount;
	curr += 4;
	*(int32_t*)curr = setRef->actors.unknownInt;
	curr += 4;

	for (int32_t act = 0; act < setRef->actors.actorsCount; act++) {
		*(int32_t*)curr = setRef->actors.actors.at(act).unknownInt;
		curr += 4;
		*(int16_t*)curr = setRef->actors.actors.at(act).rotation8;
		curr += 2;
		*(int16_t*)curr = setRef->actors.actors.at(act).positionX;
		curr += 2;
		*(int16_t*)curr = setRef->actors.actors.at(act).positionZ;
		curr += 2;
		*(int16_t*)curr = setRef->actors.actors.at(act).positionY;
		curr += 2;
		*curr++ = setRef->actors.actors.at(act).identifier.length();
		memcpy(curr, setRef->actors.actors.at(act).identifier.c_str(), *(curr - 1));
		memset(curr + *(curr - 1), 0, 41 - *(curr - 1));
		curr += 41;
	}
}

void DFedit::SETedit::updateSETviewRegister(DFfile::Container& container, int32_t scene) {

	// prepare container data
	container.size = 52 + (setRef->scenes[scene].SceneViewCount * 46);

	delete[] container.data;
	container.data = new uint8_t[container.size];

	// make sure the locations are correct before writing the file with it!
	int32_t objLoc = container.id + 2;	// first object should be here, if there are any
	for (uint32_t i = 0; i < setRef->scenes[scene].SceneViewCount; i++) {
		if (setRef->scenes[scene].sceneViews[i].objTable) {
			setRef->scenes[scene].sceneViews[i].locationObjects = objLoc++;
			for (int32_t obj = 0; obj < setRef->scenes[scene].sceneViews[i].objTable->objectCount; obj++) {
				setRef->scenes[scene].sceneViews[i].objTable->objectEntries[obj].locationScript = objLoc++;
			}
		}
	}

	uint8_t* curr = container.data;
	curr += setRef->swapEndians(*(double*)curr, (uint8_t*)setRef->scenes[scene].sceneLocation);
	curr += setRef->swapEndians(*(double*)curr, (uint8_t*)(setRef->scenes[scene].sceneLocation + 1));
	curr += setRef->swapEndians(*(double*)curr, (uint8_t*)(setRef->scenes[scene].sceneLocation + 2));

	// first value is always zero, 
	*(int32_t*)(curr) = setRef->scenes[scene].unknownVal[0];
	curr += 4;
	// second value is location script
	*(int32_t*)(curr) = setRef->scenes[scene].locationScript;	// script location was updated and should be correct
	setRef->scenes[scene].unknownVal[1] = *(int32_t*)(curr);	// update main data as well
	curr += 4;

	// next line is the scene name again (already written in scene table but whatever...
	*curr++ = setRef->scenes[scene].sceneName.size();
	memset(curr, 0, 15);
	memcpy(curr, setRef->scenes[scene].sceneName.c_str(), *(curr - 1));
	curr += 15;

	*(int32_t*)(curr) = setRef->scenes[scene].SceneViewCount;
	curr += 4;

	// view entry size 46 byte
	for (uint32_t i = 0; i < setRef->scenes[scene].SceneViewCount; i++) {
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->scenes[scene].sceneViews[i].rotation);
		*(int16_t*)(curr) = setRef->scenes[scene].sceneViews[i].rotation8;
		curr += 2;
		*(int32_t*)(curr) = setRef->scenes[scene].sceneViews[i].viewPairType;
		curr += 4;
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->scenes[scene].sceneViews[i].unknownDB2);
		*(int32_t*)(curr) = setRef->scenes[scene].sceneViews[i].viewID;
		curr += 4;
		*(int32_t*)(curr) = setRef->scenes[scene].sceneViews[i].locationObjects;
		curr += 4;
		*curr++ = setRef->scenes[scene].sceneViews[i].viewName.length();
		memset(curr, 0, 15);
		memcpy(curr, setRef->scenes[scene].sceneViews[i].viewName.c_str(), *(curr - 1));
		curr += 15;
	}
}

void DFedit::SETedit::reconstructSceneData(std::vector<DFfile::Container>& containerRef, int32_t scene, bool onlyOneRotationRegister) {

	// update location data for scene index later
	setRef->scenes[scene].locationViews = containerRef.size();
	setRef->scenes[scene].locationScript = containerRef.size() + 1;

	// now construct actual containers for this scene
	DFeditRef->addEmptyContainer(containerRef);
	updateSETviewRegister(containerRef.back(), scene);
	// scene script
	if (setRef->scenes[scene].containerSceneScript.size)
		DFeditRef->addContainer(containerRef, setRef->scenes[scene].containerSceneScript);
	else
		DFeditRef->addEmptyContainer(containerRef);

	// now go thogh the scene views and write possible object tables and their scripts
	for (int32_t view = 0; view < setRef->scenes[scene].SceneViewCount; view++) {
		if (setRef->scenes[scene].sceneViews[view].objTable) {
			// if here, it has an object reference
			int32_t objCount = setRef->scenes[scene].sceneViews[view].objTable->objectCount;
			int32_t tableSize = 8 + objCount * 36;
			uint8_t* objs = new uint8_t[tableSize];
			DFeditRef->addContainer(containerRef, objs, tableSize);

			uint8_t* curr = objs;
			// write table header:
			*(int32_t*)curr = objCount;
			curr += 4;
			*(int32_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->unknownInt;
			curr += 4;
			// iterate though the objects
			for (int32_t obj = 0; obj < objCount; obj++) {

				// location script should have been updated in scene function already and considered to be correct
				int32_t locScript = setRef->scenes[scene].sceneViews[view].objTable->objectEntries[obj].locationScript;

				*(int32_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).unknownInt;
				curr += 4;
				*(int16_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).rotation8;
				curr += 2;
				*(int16_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).unknownShort2;
				curr += 2;
				*(int16_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).startRegionX;
				curr += 2;
				*(int16_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).startRegionY;
				curr += 2;
				*(int16_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).endRegionX;
				curr += 2;
				*(int16_t*)curr = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).endRegionY;
				curr += 2;
				*(int32_t*)curr = locScript;
				curr += 4;
				*curr++ = setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).identifier.length();
				memset(curr, 0, 15);
				memcpy(curr, setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).identifier.c_str(), *(curr - 1));
				curr += 15;

				// every object has script, which may be used or not (empty), but its there.
				if (setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).containerObjScript.size)
					DFeditRef->addContainer(containerRef, setRef->scenes[scene].sceneViews[view].objTable->objectEntries.at(obj).containerObjScript);
				else
					DFeditRef->addEmptyContainer(containerRef);
			}
		}
	}
	// once again, update locations... 
	setRef->scenes[scene].locationViewLogic[setRef->RIGHTTURNS] = containerRef.size();
	setRef->scenes[scene].locationViewLogic[setRef->LEFTTURNS] = containerRef.size() + 1;
	// before writing the transition data we must make sure that the location references are correct
	int32_t lastLocation = containerRef.size() + 2;
	// if bool is true we only iterate once and fill in the other rotation differently
	for (uint8_t dir = 0; dir < 2 - onlyOneRotationRegister; dir++) {
		for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
			setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].frameContainerLoc = lastLocation++;
		}
	}
	if (onlyOneRotationRegister) {
		// if here, fill in the same frame references like the right rotation table has
		setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[0].frameContainerLoc = setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[0].frameContainerLoc;
		for (int32_t frame = 1; frame < setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameCount; frame++) {
			setRef->scenes[scene].rotationRegister[setRef->LEFTTURNS].frameInfos[frame].frameContainerLoc
				= setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameInfos[setRef->scenes[scene].rotationRegister[setRef->RIGHTTURNS].frameCount - frame].frameContainerLoc;
		}
	}



	// rotational information will be skipped and written when we know where the transition containers will be
	DFeditRef->addEmptyContainer(containerRef);
	DFeditRef->addEmptyContainer(containerRef);

	// but we can already tell it which road and direction it will be
	// when we write the container locations

	for (uint8_t dir = 0; dir < 2 - onlyOneRotationRegister; dir++) {
		for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
			// some containers were not defined (default size 0). those can be written as empty
			if (setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].containerFrame.size)
				DFeditRef->addContainer(containerRef, setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].containerFrame);
			else
				DFeditRef->addEmptyContainer(containerRef);
		}
	}
}

void DFedit::SETedit::reconstructRoadData(std::vector<DFfile::Container>& containerRef, int32_t road) {

	// update location references. We will need those later and so does the engine
	setRef->transitionTable.transitions[road].locationTransitionInfo = containerRef.size();
	setRef->transitionTable.transitions[road].locationSceneA = containerRef.size() + 1;
	setRef->transitionTable.transitions[road].locationSceneB = containerRef.size() + 2;

	// start with first container: transition/road info
	int32_t roadSize = 82 + (24 * setRef->transitionTable.transitions[road].entriesCount);
	uint8_t* roadData = new uint8_t[roadSize];
	uint8_t* currRoad = roadData;

	*(int32_t*)currRoad = setRef->transitionTable.transitions[road].unknownInt;
	currRoad += 4;
	*(int16_t*)currRoad = setRef->transitionTable.transitions[road].rotation8;
	currRoad += 2;
	*(int32_t*)currRoad = setRef->transitionTable.transitions[road].viewIDstart;
	currRoad += 4;
	*(int32_t*)currRoad = setRef->transitionTable.transitions[road].viewIDend;
	currRoad += 4;
	currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].zAxisStart);
	currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].yAxisStart);
	currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].xAxisStart);

	currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].xAxisEnd);
	currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].zAxisEnd);
	currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].yAxisEnd);

	*currRoad++ = setRef->transitionTable.transitions[road].transitionName.length();
	memcpy(currRoad, setRef->transitionTable.transitions[road].transitionName.c_str(), *(currRoad - 1));
	memset(currRoad + *(currRoad - 1), 0, 15 - *(currRoad - 1));

	currRoad += 15;
	*(int32_t*)currRoad = setRef->transitionTable.transitions[road].entriesCount;
	currRoad += 4;

	for (int32_t e = 0; e < setRef->transitionTable.transitions[road].entriesCount; e++) {
		currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].entries[e].xAxis);
		currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].entries[e].zAxis);
		currRoad += setRef->swapEndians(*(double*)currRoad, (uint8_t*)&setRef->transitionTable.transitions[road].entries[e].yAxis);
	}

	DFeditRef->addContainer(containerRef, roadData, roadSize);

	// transition registers A and B will be skipped for now because we need to read in the destination containers first
	DFeditRef->addEmptyContainer(containerRef);
	DFeditRef->addEmptyContainer(containerRef);

	// now just push in the frame data sequencially and update the position data
	for (uint8_t dir = 0; dir < 2; dir++) {
		for (int32_t frame = 0; frame < setRef->transitionTable.transitions[road].frameRegister[dir].frameCount; frame++) {
			setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].frameContainerLoc = containerRef.size();
			// if size is zero, empty containers must be expected
			if (setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].containerFrame.size)
				DFeditRef->addContainer(containerRef, setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].containerFrame);
			else
				DFeditRef->addEmptyContainer(containerRef);
		}
	}
}

void DFedit::SETedit::clearResources() {

	if (setRef) {
		bool onlyOneFrameRegister = checkFrameRegister();

		for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
			for (uint8_t dir = 0; dir < 2 - onlyOneFrameRegister; dir++) {
				for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
					setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].containerFrame.clearContent();
					setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].containerFrame.size = 0;
				}
			}
		}

		for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
			for (uint8_t dir = 0; dir < 2; dir++) {
				for (int32_t frame = 0; frame < setRef->transitionTable.transitions[road].frameRegister[dir].frameCount; frame++) {
					setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].containerFrame.clearContent();
					setRef->transitionTable.transitions[road].frameRegister[dir].frameInfos[frame].containerFrame.size = 0;
				}
			}
		}
	}

}

bool DFedit::SETedit::reconstructContainers(bool onlyOneRotationRegister) {

	// make sure main container root is empty

	if (setRef) {
		// at this point every reference should be in Memory!
		// delete old container structure except scripts and images

		// set header: write that when we know the new structure
		DFeditRef->addEmptyContainer(setRef->containers);
		// copy main script
		setRef->setHeader.mainScript = 1;
		DFeditRef->addContainer(setRef->containers, setRef->setHeader.containerMainScript);
		// write full scene register later. need refresh positions first
		setRef->setHeader.mainSceneRegister = 2;
		DFeditRef->addEmptyContainer(setRef->containers);

		// actor register always at umber three
		setRef->setHeader.actorRegister = 3;
		DFeditRef->addEmptyContainer(setRef->containers);
		if (setRef->actors.actorsCount)
			updateSETactorRegister(setRef->containers.back());


		//-  SCENE / ROAD ASSEMBLER -//
		// now go though all roads and write the corresponding scenes until everything is written
		// keep track of which scenes were already written
		for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
			reconstructSceneData(setRef->containers, scene, onlyOneRotationRegister);
		}
		for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
			reconstructRoadData(setRef->containers, road);
		}

		// at this point all scenes should be written. the locations are therefore known
		// now its time to write the road transition tables and update the destinations

		for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
			bool found[2]{ false };
			for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
				if (found[0] && found[1]) break;
				for (int32_t view = 0; view < setRef->scenes[scene].sceneViews.size(); view++) {
					// SCENE A
					if (setRef->transitionTable.transitions[road].viewIDend == setRef->scenes[scene].sceneViews[view].viewID) {
						setRef->transitionTable.transitions[road].frameRegister[0].destination = setRef->scenes[scene].locationViews;
						int32_t tableSize = 12 + setRef->transitionTable.transitions[road].frameRegister[0].frameCount * 60;
						uint8_t* roadData = new uint8_t[tableSize];
						DFeditRef->changeContainerContent(setRef->transitionTable.transitions[road].locationSceneA, roadData, tableSize);
						setRef->transitionTable.transitions[road].frameRegister[0].writeTransitionContainer(setRef->containers[setRef->transitionTable.transitions[road].locationSceneA].data);
						found[0] = true;
					}
					// SCENE B
					if (setRef->transitionTable.transitions[road].viewIDstart == setRef->scenes[scene].sceneViews[view].viewID) {
						setRef->transitionTable.transitions[road].frameRegister[1].destination = setRef->scenes[scene].locationViews;
						int32_t tableSize = 12 + setRef->transitionTable.transitions[road].frameRegister[1].frameCount * 60;
						uint8_t* roadData = new uint8_t[tableSize];
						DFeditRef->changeContainerContent(setRef->transitionTable.transitions[road].locationSceneB, roadData, tableSize);
						setRef->transitionTable.transitions[road].frameRegister[1].writeTransitionContainer(setRef->containers[setRef->transitionTable.transitions[road].locationSceneB].data);
						found[1] = true;
					}
					if (found[0] && found[1]) break;
				}
			}
			// if one of them were not found even after checking all the scenes something went wrong!
			if (!found[0] || !found[1]) return false;

		}


		// Now write the rotation data
		for (int32_t road = 0; road < setRef->transitionTable.transitionCount; road++) {
			for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
				for (int32_t view = 0; view < setRef->scenes[scene].sceneViews.size(); view++) {
					if (setRef->transitionTable.transitions[road].viewIDstart == setRef->scenes[scene].sceneViews[view].viewID) {
						// update both rotation registers accordingly
						for (uint8_t dir = 0; dir < 2; dir++) {
							for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
								// update the frame that matches the view, should be only one for each directions so break out
								if (setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].viewID == view) {
									setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].transitionLog = setRef->transitionTable.transitions[road].locationSceneA;
									break;
								}
							}
						}
					}
					if (setRef->transitionTable.transitions[road].viewIDend == setRef->scenes[scene].sceneViews[view].viewID) {
						// update both rotation registers accordingly
						for (uint8_t dir = 0; dir < 2; dir++) {
							for (int32_t frame = 0; frame < setRef->scenes[scene].rotationRegister[dir].frameCount; frame++) {
								// update the frame that matches the view, should be only one for each directions so break out
								if (setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].viewID == view) {
									setRef->scenes[scene].rotationRegister[dir].frameInfos[frame].transitionLog = setRef->transitionTable.transitions[road].locationSceneB;
									break;
								}
							}
						}
					}
				}
			}
		}

		for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
			for (uint8_t dir = 0; dir < 2; dir++) {
				// location info should be updated. Now you can write the rotation tables
				int32_t tableSize = 12 + setRef->scenes[scene].rotationRegister[dir].frameCount * 60;
				uint8_t* data = new uint8_t[tableSize];
				DFeditRef->changeContainerContent(setRef->scenes[scene].locationViewLogic[dir], data, tableSize);
				setRef->scenes[scene].rotationRegister[dir].writeTransitionContainer(setRef->containers[setRef->scenes[scene].locationViewLogic[dir]].data);
			}
		}


		// locations should be updated. scene index register can now be written
		updateSETmainSceneRegister(setRef->containers[setRef->setHeader.mainSceneRegister]);

		// write additional small scene register
		setRef->setHeader.sceneRegister = setRef->containers.size();
		int32_t containerSize = 12 + 16 * setRef->scenes.size();
		uint8_t* smallSceneRegisterData = new uint8_t[containerSize];
		uint8_t* curr = smallSceneRegisterData;
		*(int32_t*)curr = 0;	// seems always 0
		curr += 4;
		*(int32_t*)curr = 20;	// I dont know what this does, but I didnt notice a difference ingame
		curr += 4;
		*(int32_t*)curr = setRef->scenes.size();	// scene count
		curr += 4;
		for (int32_t scene = 0; scene < setRef->scenes.size(); scene++) {
			// 16 byte entries
			*(int32_t*)curr = setRef->scenes[scene].locationViews;
			curr += 4;
			*(int32_t*)curr = setRef->scenes[scene].locationViewLogic[setRef->RIGHTTURNS];
			curr += 4;
			*(int32_t*)curr = setRef->scenes[scene].locationViewLogic[setRef->LEFTTURNS];
			curr += 4;
			*(int16_t*)curr = 0;	// no idea what this does, usually one scene has 1
			curr += 2;
			*(int16_t*)curr = 2;	// seems always 2
		}
		DFeditRef->addContainer(setRef->containers, smallSceneRegisterData, containerSize);

		// write transition register
		setRef->setHeader.transitionRegister = setRef->containers.size();
		if (int32_t roadCount = setRef->transitionTable.transitionCount) {
			containerSize = 8 + 16 * roadCount;
			uint8_t* roadRegisterData = new uint8_t[containerSize];
			curr = roadRegisterData;

			*(int32_t*)curr = 0;
			curr += 4;
			*(int32_t*)curr = roadCount;
			curr += 4;
			for (int32_t road = 0; road < roadCount; road++) {
				*(int32_t*)curr = setRef->transitionTable.transitions[road].locationTransitionInfo;
				curr += 4;
				*(int32_t*)curr = setRef->transitionTable.transitions[road].locationSceneA;
				curr += 4;
				*(int32_t*)curr = setRef->transitionTable.transitions[road].locationSceneB;
				curr += 4;
				*(int16_t*)curr = setRef->transitionTable.transitions[road].unknownShort1;
				curr += 2;
				*(int16_t*)curr = setRef->transitionTable.transitions[road].unknownShort2;
				curr += 2;
			}

			DFeditRef->addContainer(setRef->containers, roadRegisterData, containerSize);
		}
		else {
			DFeditRef->addEmptyContainer(setRef->containers);	// it can be also empty if only one scene exists
		}

		// map 1 and 2 always at the end
		setRef->setHeader.mapLight = setRef->containers.size();
		DFeditRef->addContainer(setRef->containers, setRef->setHeader.containermapLight);
		setRef->setHeader.mapDark = setRef->containers.size();
		DFeditRef->addContainer(setRef->containers, setRef->setHeader.containermapDark);

		// at last... the giant set header
		containerSize = 2606;	// seems always the same size
		uint8_t* setHeaderData = new uint8_t[containerSize];
		curr = setHeaderData;
		// writing for TAOOT version 4.0
		*(int16_t*)curr = 0;
		curr += 2;
		*(int32_t*)(curr) = setRef->setHeader.version;
		curr += 4;
		memset(curr, 0, 18);
		curr += 18;
		*(int32_t*)curr = setRef->setHeader.mapLight;
		curr += 4;
		*(int32_t*)curr = setRef->setHeader.mapDark;
		curr += 4;
		*(int32_t*)curr = 0; // no idea
		curr += 4;
		*(int16_t*)curr = setRef->setHeader.mapHeight;
		curr += 2;
		*(int16_t*)curr = setRef->setHeader.mapWidth;
		curr += 2;
		*(int32_t*)curr = 0; // no idea
		curr += 4;
		*(int16_t*)curr = setRef->setHeader.setDimensionsY;
		curr += 2;
		*(int16_t*)curr = setRef->setHeader.setDimensionsX;
		curr += 2;
		memset(curr, 0, 16);
		curr += 16;
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.setDimensionsYf);
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.setDimensionsXf);

		// copy various register locations
		memcpy(curr, &setRef->setHeader.sceneRegister, 32);
		curr += 32;
		*curr++ = setRef->setHeader.setName.length();
		memset(curr, 0, 19);
		memcpy(curr, setRef->setHeader.setName.c_str(), *(curr - 1));
		curr += 19;

		*(int16_t*)curr = setRef->setHeader.viewPortWidth;
		curr += 2;
		*(int16_t*)curr = setRef->setHeader.viewPortHeight;
		curr += 2;
		// coopy coordinate/rotational data
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.coords[0]);
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.coords[1]);
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.coords[2]);
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.rotations[0]);
		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.rotations[1]);

		curr += setRef->swapEndians(*(float*)curr, (uint8_t*)&setRef->setHeader.unknownF0xB0);
		curr += setRef->swapEndians(*(float*)curr, (uint8_t*)&setRef->setHeader.unknownF0xB4);
		curr += setRef->swapEndians(*(float*)curr, (uint8_t*)&setRef->setHeader.unknownF0xB8);

		// put yet unknown dataset that has 3 entries
		// I think it has to do with memory
		for (uint8_t i = 0; i < 3; i++) {
			*(int16_t*)curr = setRef->setHeader.uEntries[i].count;
			curr += 2;
			*(int32_t*)curr = setRef->setHeader.uEntries[i].limit;
			curr += 4;
			*(int32_t*)curr = setRef->setHeader.uEntries[i].unknownInt[0];
			curr += 4;
			*(int32_t*)curr = setRef->setHeader.uEntries[i].unknownInt[1];
			curr += 4;
			*(int16_t*)curr = setRef->setHeader.uEntries[i].rotation8[0];
			curr += 2;
			*(int16_t*)curr = setRef->setHeader.uEntries[i].rotation8[1];
			curr += 2;
		}

		// the locations and most important data is fed. Now get the color palette

		memcpy(curr, setRef->colors, 256 * sizeof(DFfile::ColorPalette));
		curr += sizeof(DFfile::ColorPalette) * 256;
		*curr++ = setRef->setHeader.secondaryRefName.size();
		memset(curr, 0, 255);
		memcpy(curr, setRef->setHeader.secondaryRefName.c_str(), *(curr - 1));
		curr += 255;

		curr += setRef->swapEndians(*(double*)curr, (uint8_t*)&setRef->setHeader.unknownD0x9F2);
		memcpy(curr, &setRef->setHeader.unknownS0x9FA, 6);
		curr += 6;
		memcpy(curr, &setRef->setHeader.unknownI0xA00, 8);
		curr += 8;

		*(int16_t*)curr = setRef->setHeader.setDimensionsY_2;
		curr += 2;
		*(int32_t*)curr = setRef->setHeader.heightDifference;
		curr += 4;
		*curr++ = setRef->setHeader.defaultSceneName.length();
		memset(curr, 0, 15);
		memcpy(curr, setRef->setHeader.defaultSceneName.data(), *(curr - 1));
		curr += 15;
		*curr++ = setRef->setHeader.defaultViewName.length();;
		memset(curr, 0, 15);
		memcpy(curr, setRef->setHeader.defaultViewName.data(), *(curr - 1));
		// -- HEADER WRITTEN -- //
		DFeditRef->changeContainerContent(0, setHeaderData, containerSize);

		// refresh container count in file header and we are good
		setRef->fileHeader.containerCount = setRef->containers.size();
		// include dfedit version for later reference
		DFeditRef->addDFeditLabel();
		DFeditRef->addDFeditKey();

		return true;


	}
	else
		return false;
}
