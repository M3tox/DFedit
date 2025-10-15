#include "DFedit.h"

void DFedit::printMOVframeInfo() {
	DFmov* mov = DFmov::getDFmov(fileRef);
	if (!mov) {
		std::cout << "ERROR: NO MOVE FILE LOADED!\n";
		return;
	}

	std::cout << mov->getFrameData();
}

void DFedit::createNewStaticMOVfile(const std::string& newFileName) {
	fileRef = new DFmov(newFileName, 1);

	DFmov* movRef = DFmov::getDFmov(fileRef);


	// construct header file
	movRef->fileHeader.FourCC = 0x00010000;
	movRef->fileHeader.unknown[0] = 0;
	movRef->fileHeader.unknown[1] = 0;
	movRef->fileHeader.unknown[2] = 128;
	movRef->fileHeader.containerCount = 0; // functions that create new container will incr this
	movRef->fileHeader.type = 0;
	movRef->fileHeader.gapWhere = 0;

	memset(fileRef->fileHeader.unknown2, 0, 983);
	memcpy(fileRef->fileHeader.unknown2 + 471, "Created with DFedit made by MeToX", 34);
	memcpy(fileRef->fileHeader.unknown2 + 471 + 64, DFVERSION, 13);
	// put some random numbers
	memcpy(fileRef->fileHeader.unknown2 + 471 + 256, "DFED", 4);
	*(uint64_t*)(fileRef->fileHeader.unknown2 + 471 + 256 + 4) = DFKEY;

	// process color palette
	memset(movRef->colors, 0, 256 * sizeof(DFfile::ColorPalette)); // empty color palette
	for (int16_t c = 0; c < movRef->getColorCount(); c++)
		movRef->colors[c].index = c;


	// main header file done, now movie header
	movRef->movHeader.version = 4;
	movRef->movHeader.unknown[0] = 1;
	movRef->movHeader.unknown[1] = 3;
	movRef->movHeader.unknown[2] = -1;
	// set the rest of these values to zero
	memset(movRef->movHeader.unknown + 3, 0, 4 * 15);
	movRef->movHeader.audioLocation = 2;
	movRef->movHeader.memoryAllocInfo = 1;
	movRef->movHeader.unknownRef2 = 3;
	// update container until color palette
	//memset(curr, 0, 2);


	movRef->movHeader.globalHeight = 384;	// applies for all frames (I Think)
	movRef->movHeader.globalWidth = 512;	// applies for all frames
	movRef->movHeader.unknwonInt = 0;		// seems important
	movRef->movHeader.frameCount = 2;
	movRef->movHeader.frames.reserve(movRef->movHeader.frameCount);

	movRef->movHeader.frames.emplace_back();
	movRef->movHeader.frames.at(0).frameCondition = 3;
	movRef->movHeader.frames.at(0).unknownWord1 = 0;
	movRef->movHeader.frames.at(0).unknownWord2 = 0;
	movRef->movHeader.frames.at(0).height = movRef->movHeader.globalHeight;
	movRef->movHeader.frames.at(0).width = movRef->movHeader.globalWidth;
	movRef->movHeader.frames.at(0).locationFrame = 5;
	movRef->movHeader.frames.at(0).locationClickRegion = 4;
	movRef->movHeader.frames.at(0).unknownWord3 = 0;
	movRef->movHeader.frames.at(0).frameContainerSize = 196996; // based on 512x384 + control bytes
	std::string frameName(movRef->currentFileName);
	frameName = frameName.substr(0, frameName.length() - 4);
	frameName.append(" 1");
	memcpy(movRef->movHeader.frames.at(0).frameName, frameName.c_str(), frameName.length());
	// information of click region container
	memset(movRef->movHeader.frames.at(0).frameHeadInformation, 0, 5 * 4);
	movRef->movHeader.frames.at(0).frameHeadInformation[0] = 6;
	movRef->movHeader.frames.at(0).frameHeadInformation[3] = 16;
	movRef->movHeader.frames.at(0).frameLogicSize = 1;
	movRef->movHeader.frames.at(0).frameLogic.reserve(1);
	movRef->movHeader.frames.at(0).frameLogic.emplace_back();
	movRef->movHeader.frames.at(0).frameLogic[0].unknownShort1 = 6;
	movRef->movHeader.frames.at(0).frameLogic[0].unknownInt1 = 2;
	movRef->movHeader.frames.at(0).frameLogic[0].unknownShort2 = 1;
	movRef->movHeader.frames.at(0).frameLogic[0].clickRegionStartX = 331;
	movRef->movHeader.frames.at(0).frameLogic[0].clickRegionStartY = 420;
	movRef->movHeader.frames.at(0).frameLogic[0].clickRegionEndX = 372;
	movRef->movHeader.frames.at(0).frameLogic[0].clickRegionEndY = 494;
	movRef->movHeader.frames.at(0).frameLogic[0].unknownInt2 = 0;
	movRef->movHeader.frames.at(0).frameLogic[0].unknownInt3 = -1;
	// rest of yet unresolved data
	uint8_t t[40]{ 0x01, 0x6B, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x8C, 0x68, 0xF0, 0x40, 0x00, 0x00, 0x01, 0x6B, 0xF1, 0x00, 0x00, 0xB1, 0x0F, 0x90, 0x00, 0xAD, 0x66, 0x78, 0xEC, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB1, 0x37, 0x90, 0xDA, 0x14 };
	memcpy(movRef->movHeader.frames.at(0).frameLogic[0].unknownInts, t, 40);

	// NEXT FRAME
	movRef->movHeader.frames.emplace_back();
	movRef->movHeader.frames.at(1).frameCondition = 0;
	movRef->movHeader.frames.at(1).unknownWord1 = 339;
	movRef->movHeader.frames.at(1).unknownWord2 = 400;
	movRef->movHeader.frames.at(1).height = movRef->movHeader.globalHeight;
	movRef->movHeader.frames.at(1).width = movRef->movHeader.globalWidth;
	movRef->movHeader.frames.at(1).locationFrame = 7;
	movRef->movHeader.frames.at(1).locationClickRegion = 6;
	movRef->movHeader.frames.at(1).unknownWord3 = 1;
	movRef->movHeader.frames.at(1).frameContainerSize = 12164; // based on 512x384 + control bytes minus transparency
	frameName.assign(movRef->currentFileName);
	frameName = frameName.substr(0, frameName.length() - 4);
	frameName.append(" 2");
	memcpy(movRef->movHeader.frames.at(1).frameName, frameName.c_str(), frameName.length());
	// information of click region container
	memset(movRef->movHeader.frames.at(1).frameHeadInformation, 0, 5*4);
	movRef->movHeader.frames.at(1).frameHeadInformation[0] = 0x00140001;
	movRef->movHeader.frames.at(1).frameHeadInformation[3] = 17;
	movRef->movHeader.frames.at(1).frameHeadInformation[4] = 1;
	movRef->movHeader.frames.at(1).frameLogicSize = 0;

	// add 8 empty containers
	addEmptyContainer(fileRef->containers, 8);
	fileRef->fileHeader.containerCount = 8;
	// add mem alloc info
	uint8_t* memContainer = new uint8_t[270];
	uint8_t meminfo[270]{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6B, 0x01, 0x48, 0xEC, 0x00, 0x00, 0xB0, 0x34, 0x02, 0x00, 0x2A, 0x00, 0x11, 0x00, 0x41, 0x00, 0x02, 0x00, 0x2A, 0x00, 0x11, 0x00, 0x41, 0x00, 0x02, 0x00, 0x2A, 0x00, 0x11, 0x00, 0x41, 0x00, 0x02, 0x00, 0x2A, 0x00, 0x11, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x42, 0x00, 0x9C, 0x8E, 0x10, 0xE0, 0xA8, 0x0D, 0x6B, 0x01, 0x30, 0xE3, 0x00, 0x42, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x68, 0x40, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x6B, 0x01, 0x30, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0C, 0x00, 0x7C, 0xB9, 0x0C, 0x00, 0xB4, 0xB7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x34, 0x0C, 0x00, 0x40, 0xE7, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x08, 0x00, 0x44, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x9E, 0x29, 0x00, 0x04, 0x3F, 0x29, 0x00, 0x08, 0x3F, 0x6B, 0x01, 0x68, 0xE8, 0x6B, 0x01, 0x0C, 0xEB, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x40, 0x16, 0x00, 0x00, 0x00, 0x00, 0x6B, 0x01, 0x10, 0xE3, 0x0C, 0x00, 0x40, 0xE7, 0x0C, 0x00, 0x40, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x60, 0xDD, 0x88, 0x00, 0x18, 0x72, 0x6B, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	memcpy(memContainer, meminfo, 270);
	changeContainerContent(movRef->movHeader.memoryAllocInfo, memContainer, 270);

	updateMOV();
	// add images
	frameName.assign("MOV/");
	frameName.append(movRef->currentFileName);
	frameName.assign(frameName.substr(0, frameName.length() - 4));
	frameName.append(".bmp");
	changeColorPalette(frameName);
	changeImageContainer(movRef->containers[5], frameName);
	changeImageWithPessedOKBtn(5, 7);

	return;
}

void DFedit::updateMOV() {
	// TODO: THIS SHOULD BE CALCULATED PREOR!

	DFmov* movRef = DFmov::getDFmov(fileRef);

	const int32_t containerSize{ 2168 +(movRef->movHeader.frameCount *44) };
	uint8_t* headerFileContainerData = new uint8_t[containerSize]{ 0 };

	uint8_t* curr = headerFileContainerData;

	curr += 2;
	*(int32_t*)curr = movRef->movHeader.version;
	curr += 4;
	//memset(curr, 0, 18);
	curr += 18;
	memcpy(curr, movRef->movHeader.unknown, 84);
	curr += 84;

	memcpy(curr, movRef->colors, 8 * 256);

	curr += 8 * 256;
	curr += 4; // another unknown integer? here, always 0


		// now copy everything into container...
	*(int16_t*)curr = movRef->movHeader.globalHeight;
	curr += 2;
	*(int16_t*)curr = movRef->movHeader.globalWidth;
	curr += 2;
	*(int16_t*)curr = movRef->movHeader.unknwonInt;
	curr += 4;
	*(int32_t*)curr = movRef->movHeader.frameCount;
	curr += 4;

	for (int32_t i = 0; i < movRef->movHeader.frameCount; i++) {

		*(int32_t*)curr = movRef->movHeader.frames[i].frameCondition;	// 1 = yes, 0 = No
		curr += 4;
		*(int16_t*)curr = movRef->movHeader.frames[i].unknownWord1;
		curr += 2;
		*(int16_t*)curr = movRef->movHeader.frames[i].unknownWord2;
		curr += 2;
		*(int16_t*)curr = movRef->movHeader.frames[i].height;
		curr += 2;
		*(int16_t*)curr = movRef->movHeader.frames[i].width;
		curr += 2;
		*(int32_t*)curr = movRef->movHeader.frames[i].locationFrame;
		curr += 4;
		*(int32_t*)curr = movRef->movHeader.frames[i].locationClickRegion;
		curr += 4;
		*(int16_t*)curr = movRef->movHeader.frames[i].unknownWord3;
		curr += 2;
		*(int32_t*)curr = movRef->movHeader.frames[i].frameContainerSize;
		curr += 4;
		std::string temp(movRef->movHeader.frames[i].frameName);
		*curr = temp.size();
		memcpy(curr + 1, temp.c_str(), *curr);
		curr += 16;
	}


	changeContainerContent(0, headerFileContainerData, containerSize);


	// write frameRegion containers
	for (uint32_t frame = 0; frame < movRef->movHeader.frameCount; frame++) {
		const int32_t regionContainerSize{ 1094+(movRef->movHeader.frames[frame].frameLogicSize*64) };
		uint8_t* regionContainer = new uint8_t[regionContainerSize]{ 0 };


		// read corresponding region information for this specific frame

		uint8_t* currRegion = regionContainer;
		memcpy(currRegion, movRef->movHeader.frames[frame].frameHeadInformation, 5 * 4);

		// jump to part where regional info is written
		currRegion += 1090;
		*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogicSize;
		currRegion += 4;
		for (int32_t fi = 0; fi < movRef->movHeader.frames[frame].frameLogicSize; fi++) {
			*(int16_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].unknownShort1;
			currRegion += 2;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].unknownInt1;
			currRegion += 4;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].unknownShort2;
			currRegion += 2;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].clickRegionStartX;
			currRegion += 2;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].clickRegionStartY;
			currRegion += 2;	  
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].clickRegionEndX;
			currRegion += 2;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].clickRegionEndY;
			currRegion += 2;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].unknownInt2;
			currRegion += 4;	   
			*(int32_t*)currRegion = movRef->movHeader.frames[frame].frameLogic[fi].unknownInt3;
			currRegion += 4;
			memcpy(currRegion, movRef->movHeader.frames[frame].frameLogic[fi].unknownInts, 4 * 10);
			currRegion += 40;
		}


		changeContainerContent(movRef->movHeader.frames[frame].locationClickRegion, regionContainer, regionContainerSize);
	}



	
}
