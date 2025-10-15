const char* DFedit::readScriptFile(const std::string& location, uint32_t container) {
	if (!fileRef)
		return "File not referenced!\n";

	uint8_t* newFile = nullptr;
	uint32_t newFileSize = 0;

	std::ifstream t(location, std::ios::binary);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	//std::string buffer(size, ' ');
	std::string buffer(size, 0);
	//buffer.reserve(size+1);
	//buffer.resize(size + 1);
	t.seekg(0);
	t.read(&buffer[0], size);
	t.close();
	

	if (!fileRef->scripter.TextScriptToBinary(newFile, newFileSize, buffer)) {
		std::ofstream binaryFile("LNGHALL/testScript.bin", std::ios::binary);
		binaryFile.write((char*)newFile, newFileSize);
		binaryFile.close();
	}

	delete[] newFile;
	return "";
}


bool DFedit::checkContainerBounds(uint32_t& containerID) {
	if (containerID < 0 || containerID >= fileRef->containers.size()) {
		std::cout << "ERROR: Container ID " << containerID << " out of bounds!\n" << std::endl;
		return false;
	}
	return true;
}

void DFedit::changeContainerContent(uint32_t containerID, uint8_t* newContent, uint32_t containerSize) {
	// sanity check
	if (checkContainerBounds(containerID)) {
		delete[] fileRef->containers[containerID].data;
		fileRef->containers[containerID].data = newContent;
		fileRef->containers[containerID].size = containerSize;
	}
	return;
}

void DFedit::changeContainerContent(DFfile::Container& containerOld, uint8_t* newContent, uint32_t containerSize) {

	delete[] containerOld.data;
	containerOld.data = newContent;
	containerOld.size = containerSize;
	
	return;
}

void DFedit::insertNewContainer(uint32_t& containerID, uint8_t* content, uint32_t containerSize) {
	if (checkContainerBounds(containerID)) {
		fileRef->containers.insert(fileRef->containers.begin() + containerID, DFfile::Container());

		fileRef->containers.at(containerID).id = containerID;
		fileRef->containers.at(containerID).data = content;
		fileRef->containers.at(containerID).size = containerSize;

		// update header information
		fileRef->fileHeader.containerCount = fileRef->containers.size();
		// update container IDs because of the shift
		for (int32_t cont = containerID + 1; cont < fileRef->containers.size(); cont++) {
			fileRef->containers.at(cont).id = cont;
		}
		fileRef->fileHeader.containerCount++;
	}

	return;
}

void DFedit::removeContainer(uint32_t& containerID) {
	if (checkContainerBounds(containerID)) {

		fileRef->containers.at(containerID).clearContent();
		fileRef->containers.erase(fileRef->containers.begin() + containerID);

		// update header information
		fileRef->fileHeader.containerCount = fileRef->containers.size();
		// update container IDs because of the shift
		for (int32_t cont = containerID; cont < fileRef->containers.size(); cont++) {
			fileRef->containers.at(cont).id = cont;
		}
		fileRef->fileHeader.containerCount--;
	}

	return;
}

void DFedit::eraseContainerContent(int32_t& containerID) {
	fileRef->containers[containerID].clearContent();
	fileRef->containers[containerID].data = new uint8_t[8]{ 0 };
	fileRef->containers[containerID].size = 8;
}

void DFedit::eraseContainerContent(DFfile::Container& containerRef) {
	containerRef.clearContent();
	containerRef.data = new uint8_t[8]{ 0 };
	memset(containerRef.data, 0, 8);
	containerRef.size = 8;
}

void DFedit::addContainer(std::vector<DFfile::Container>& containerRef, uint8_t* content, uint32_t containerSize) {
	containerRef.push_back(DFfile::Container());
	containerRef.back().id = containerRef.size() - 1;
	containerRef.back().size = containerSize;
	containerRef.back().data = content;
}

void DFedit::addContainer(std::vector<DFfile::Container>& containerRef, DFfile::Container& container) {
	containerRef.push_back(container);
	containerRef.back().id = containerRef.size() - 1;
}

void DFedit::addEmptyContainer(std::vector<DFfile::Container>& containerRef) {
    containerRef.push_back(DFfile::Container());
	containerRef.back().id = containerRef.size() - 1;
	containerRef.back().size = 8;
	containerRef.back().data = new uint8_t[8]{ 0 };
}

void DFedit::addEmptyContainer(std::vector<DFfile::Container>& containerRef, int32_t count) {
	for (uint16_t c = 0; c < count; c++) {
		addEmptyContainer(containerRef);
	}
}

void DFedit::copyContainerContent(int32_t containerIDfrom, int32_t containerIDto) {
	fileRef->containers[containerIDto].clearContent();
	fileRef->containers[containerIDto].size = fileRef->containers[containerIDfrom].size;
	fileRef->containers[containerIDto].data = new uint8_t[fileRef->containers[containerIDto].size];
	memcpy(fileRef->containers[containerIDto].data, fileRef->containers[containerIDfrom].data, fileRef->containers[containerIDto].size);
}

void DFedit::changeColorPalette(const std::string& imageFile) {
	if (!fileRef)
		std::cout << "File not referenced!\n";
	// get current colorPalette
	// gilt nur für SETS!
	//memcpy(CP, fileBlock[0].blockdata + 0xF2, sizeof(ColorPalette) * 256);

	constexpr int32_t colorPaletteStart = 54;

	std::ifstream image(imageFile, std::ios::binary);
	int32_t dataPos;
	image.seekg(10);
	image.read((char*)&dataPos, 4);
	dataPos = (dataPos - colorPaletteStart) / 4;
	// check color count, only write what is available
	int32_t maxColors = fileRef->getColorCount();
	if (dataPos < maxColors)
		maxColors = dataPos;

	// jump to start of color palette
	image.seekg(colorPaletteStart);

	for (int16_t color = 0; color < maxColors; color++) {

		char input[4];
		image.read(input, 4);
		//static int16_t check[3]{ 0xFFFF,0xFFFF,0xFFFF };

		*(((uint8_t*)(fileRef->colors[color].RGB + 2)) + 1) = input[0];
		*(((uint8_t*)(fileRef->colors[color].RGB + 1)) + 1) = input[1];
		*(((uint8_t*)(fileRef->colors[color].RGB)) + 1) = input[2];
	}
	image.close();
}

int32_t DFedit::changeImageContainer(DFfile::Container& container, const std::string& imageFile) {

	std::ifstream bmpFile(imageFile, std::ios::binary);
	if (!bmpFile.good()) {
		additionalErrorInfo = imageFile;
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

	bmpFile.seekg(pixelpos);	// jump directly to actual raw pixel data

	const uint8_t parameter = 4;	// tells engine to read entire rows

	int32_t newContainerSize{ (height * (width + 1)) + 4 };
	

	uint8_t* newContainer = new uint8_t[newContainerSize];
	uint8_t* currLoc = newContainer;
	memcpy(currLoc, &height, sizeof(height));
	currLoc += 2;
	memcpy(currLoc, &width, sizeof(width));
	currLoc += 2;

	// images in bmp files are upside down, start writing on the bottom line and go up
	while (height--) {
		int32_t tempOffset = height * (width + 1);
		memcpy(currLoc + tempOffset, &parameter, sizeof(parameter));
		bmpFile.read((char*)currLoc + tempOffset + 1, width);
	}
	bmpFile.close();
	// update blockdata
	changeContainerContent(container, newContainer, newContainerSize);
	return NOERRORS;
}

bool DFedit::changeTransImageContainer(DFfile::Container& container, const std::string& imageFile, int16_t offsetY, int16_t offsetX) {


	std::ifstream pngImage(imageFile, std::ios::binary);
	if (!pngImage.good()) {
		return false;
	}

	uint32_t width, height;
	std::vector<uint8_t> image;
	//decode
	uint32_t error = lodepng::decode(image, width, height, imageFile);
	// not needed anymore
	pngImage.close();
	//if there's an error, display it
	if (error) return false;
	

	std::vector<uint8_t>* segments = new std::vector<uint8_t>[height];

	uint8_t RGBA[4];
	

	uint8_t* widthBuffer = new uint8_t[width];
	// start compressing row by row
	uint8_t chunkPixelCounter = 0;
	//uint8_t transparentPixelCounter = 0;

	uint8_t modeSelector = 3;

	// MODE 0: 00: copy x amount from previous row, amount in BYTEFLAG
	// MODE 1: 01: tells how many pixels are transparent, amount in BYTEFLAG
	// MODE 2: 10: tells how many elements to put from element X. amount in byteflag, which color next byte
	// MODE 3: 11: defines how many bytes to copy directly from file, amount in flag, elements follow up

	auto writeSegment = [&chunkPixelCounter, &modeSelector, widthBuffer](std::vector<uint8_t>& segment)
	{
		if (chunkPixelCounter) {
			uint8_t byteFlag = chunkPixelCounter << 2;;
			switch (modeSelector) {
			case 0:
				segment.push_back(byteFlag);
				chunkPixelCounter = 0;
				break;
			case 1:
				byteFlag |= 1;
				segment.push_back(byteFlag);
				chunkPixelCounter = 0;
				break;
			case 2:
				byteFlag |= 2;
				segment.push_back(byteFlag);
				segment.push_back(widthBuffer[0]);
				chunkPixelCounter = 0;
				break;
			case 3:
				byteFlag |= 3;
				segment.push_back(byteFlag);
				for (uint8_t i = 0; i < chunkPixelCounter; i++)
					segment.push_back(widthBuffer[i]);
				chunkPixelCounter = 0;
				break;
			}
		}
		// reset counter
		
		return;
	};

	for (uint32_t h = 0; h < height; h++) {
		for (uint32_t w = 0; w < width; w++) {
		
			// clear if all available 6 bits are full
			if (chunkPixelCounter >= 63) {
				writeSegment(segments[h]);
				modeSelector = 3;
			}
			memcpy(RGBA, image.data() + (h * width*4) + w*4, 4);

			// check if mode 1 applies
			if (RGBA[3] == 0) {
				// pixel transparent
				if (modeSelector != 1 || chunkPixelCounter >= 63) {
					writeSegment(segments[h]);
					// update mode
					modeSelector = 1;
				}

				chunkPixelCounter++;
				continue;
			}



			int16_t colorIndex = -1;
			// find color to check the other 2 modes
			for (int16_t color = 0; color < 256; color++) {
				uint8_t RGB[3];
				RGB[0] = *(((uint8_t*)&fileRef->colors[color].RGB[0])+1);
				RGB[1] = *(((uint8_t*)&fileRef->colors[color].RGB[1]) + 1);
				RGB[2] = *(((uint8_t*)&fileRef->colors[color].RGB[2]) + 1);
				if (!memcmp(RGB, RGBA, 3)) {
					colorIndex = color;
					break;
				}
			}
			// return false if color was not found
			if (colorIndex == -1)
				return false;

			// do we have anything in the counter?
			// if not, start filling data first


			widthBuffer[chunkPixelCounter++] = colorIndex;
			if (chunkPixelCounter > 2) {

				if (h > 1) {
					uint8_t RGBA_before[12];
					uint8_t RGBAs[12];

					memcpy(RGBA_before, image.data() + ((h - 1) * width * 4) + (w - 2) * 4, 12);
					memcpy(RGBAs, image.data() + (h * width * 4) + (w - 2) * 4, 12);
					if (!memcmp(RGBAs, RGBA_before, 12)) {
						// mode 0 conditions met
						bool skip = false;
						if (modeSelector == 2) {
							for (uint16_t i = 0; i < chunkPixelCounter; i++) {
								if (widthBuffer[i] != colorIndex) break;
								skip = true;
							}
						}

						//if (modeSelector == 3) {
						//	uint8_t RGBA_compare[12];
						//	memcpy(RGBA_compare, RGBAs + 8, 4);
						//	memcpy(RGBA_compare + 4, RGBAs + 8, 4);
						//	memcpy(RGBA_compare + 8, RGBAs + 8, 4);
						//	if (!memcmp(RGBA_compare, RGBAs, 12)) {
						//		skip = true;
						//	}
						//}
						
						if (!skip) {
							if (modeSelector != 0) {
								uint8_t tmp[3];
								tmp[2] = widthBuffer[--chunkPixelCounter];
								tmp[1] = widthBuffer[--chunkPixelCounter];
								tmp[0] = widthBuffer[--chunkPixelCounter];
								writeSegment(segments[h]);
								modeSelector = 0;
								widthBuffer[chunkPixelCounter++] = tmp[0];
								widthBuffer[chunkPixelCounter++] = tmp[1];
								widthBuffer[chunkPixelCounter++] = tmp[2];
							}
							continue;
						}

					}
				}

				if (chunkPixelCounter > 3) {
					
					constexpr uint8_t digitsToCheck{ 4 };
					uint8_t tmp[digitsToCheck]{ colorIndex,colorIndex ,colorIndex, colorIndex };
					if (!memcmp(tmp, widthBuffer + chunkPixelCounter - digitsToCheck, digitsToCheck)) {
						// mode 2 conditions met
						if (modeSelector == 0) {
							// if mode 0 is selected, check one last time entire buffer
							// before writing the segment data
							// if its all the same, it is considered to be mode 2
							for (uint16_t i = 0; i < chunkPixelCounter; i++) {
								if (widthBuffer[i] != colorIndex) break;
								modeSelector = 2;
							}
						}
					
						if (modeSelector != 2) {
							chunkPixelCounter -= digitsToCheck;
							writeSegment(segments[h]);
							modeSelector = 2;
							memset(widthBuffer, colorIndex, digitsToCheck);
							chunkPixelCounter += digitsToCheck;
						}
						continue;
					}
				}
			}
			// if here, mode 3 is expected
			if (modeSelector != 3) {

				// cut off last pixel and push it into next buffer
				chunkPixelCounter--;
				writeSegment(segments[h]);
				modeSelector = 3;
				widthBuffer[chunkPixelCounter++] = colorIndex;
			}
		}
		// push in rest if there is
		if (chunkPixelCounter)
			writeSegment(segments[h]);
		// rest written, now reset mode
		modeSelector = 3;
	}
	
	delete[] widthBuffer;

	// calculate full new image size
	uint32_t compressImageSize = 8;
	for (uint32_t i = 0; i < height; i++)
		compressImageSize += segments[i].size()+2;

	// write new image
	uint8_t* imageCompressed = new uint8_t[compressImageSize];

	*(int16_t*)imageCompressed = height;
	*(int16_t*)(imageCompressed + 2) = width;
	// if -1 is given, it means it will take the same position from the old image
	if (offsetY != -1)
		*(int16_t*)(imageCompressed + 4) = 384/2 - offsetY;
	else
		*(int16_t*)(imageCompressed + 4) = *(int16_t*)(container.data + 4);

	if (offsetX != -1)
		*(int16_t*)(imageCompressed + 6) = 512/2 - offsetX;
	else
		*(int16_t*)(imageCompressed + 6) = *(int16_t*)(container.data + 6);

	/*
0	1A
1	1D
2	29
3	33
4	28
5	2A
*/
	uint8_t* curr = imageCompressed + 8;
	for (uint32_t i = 0; i < height; i++) {
		const uint16_t segSize = segments[i].size();
		*(int16_t*)curr = segSize;
		curr += 2;
		memcpy(curr, segments[i].data(), segSize);
		curr += segSize;
	}

	container.clearContent();
	container.data = imageCompressed;
	container.size = compressImageSize;
	return true;
}




void DFedit::changeImageWithPessedOKBtn(uint32_t conainerIDfrom, uint32_t containerIDto) {
	// NOTE: THIS FUNCTION CURRENTLY ONLY WORKS IF IMAGE IS 512x384!!!
	if (!fileRef)
		std::cout << "File not referenced!\n";

	// definitions
	constexpr int16_t okBtnPosY = 339;
	constexpr int16_t okBtnPosX = 429;

	std::string okbuttonImage("MOV");
	okbuttonImage.push_back('/');
	okbuttonImage.append(fileRef->currentFileName.substr(0, fileRef->currentFileName.length() - 4));
	okbuttonImage.append("_OK.bmp");

	std::ifstream bmpFile(okbuttonImage, std::ios::binary);
	if (!bmpFile.good()) {
		std::cout << "Error: " << okbuttonImage << " not found! No OK button created?" << std::endl;
		return;
	}

	int32_t pixelPos;
	int32_t okBtnHeight;
	int32_t okBtnWidth;

	bmpFile.seekg(10);
	bmpFile.read((char*)&pixelPos, 4);
	bmpFile.seekg(18);
	bmpFile.read((char*)&okBtnWidth, 4);
	bmpFile.read((char*)&okBtnHeight, 4);

	bmpFile.seekg(pixelPos);
	uint8_t* buttonData = new uint8_t[okBtnWidth* okBtnHeight];
	for (uint8_t h = 0; h < okBtnHeight; h++) {
		bmpFile.read((char*)buttonData + ((okBtnHeight-h-1)* okBtnWidth), okBtnWidth);
	}
	bmpFile.close();

	const int16_t imageHeight = *(int16_t*)fileRef->containers[conainerIDfrom].data;
	const int16_t imageWidth = *(int16_t*)(fileRef->containers[conainerIDfrom].data+2);

	int32_t newContainerSize{ imageHeight-okBtnHeight+(okBtnHeight * (imageWidth + 1)) + 4 };

	uint8_t* currFrom = fileRef->containers[conainerIDfrom].data + 4+(okBtnPosY * (imageWidth+1));

	
	uint8_t* newContainer = new uint8_t[newContainerSize];
	*(int16_t*)newContainer = imageHeight;
	*(int16_t*)(newContainer + 2) = imageWidth;

	// write 28 for "ignore image width from previous"

	memset(newContainer + 4, 40, okBtnPosY);
	uint8_t* buffer = new uint8_t[imageWidth+1];
	// now write actual line pixel information with the changed OK button thingy
	uint8_t* newImageCurr = buttonData;
	for (uint8_t okLine = 0; okLine < okBtnHeight; okLine++) {
		memset(buffer, 0, imageWidth + 1);
		*buffer = *currFrom++;	// parameter to tell to read entire rows
		if (*buffer != 4) {
			std::cout << "ERROR: Main image not in eligble format!\n";
			return;
		}
		memcpy(buffer+1, currFrom, okBtnPosX);	// copy the first 430 pixels from the root image
		currFrom += okBtnPosX;

		memcpy(buffer+1+ okBtnPosX, newImageCurr, okBtnWidth);
		newImageCurr += okBtnWidth;
		currFrom += okBtnWidth;

		uint16_t restWidth = imageWidth - okBtnPosX - okBtnWidth;
		memcpy(buffer+1+ okBtnPosX+ okBtnWidth, currFrom, restWidth);	// copy the last 430 pixels from the root image
		currFrom += restWidth;

		memcpy(newContainer+ 4 + okBtnPosY+(okLine * (imageWidth + 1)), buffer,imageWidth+1);
	}
	delete[] buttonData;
	delete[] buffer;
	// fill up the rest with "ignore"
	memset(newContainer + 4 + okBtnPosY + okBtnHeight * (imageWidth + 1), 40, imageHeight - okBtnPosY - okBtnHeight);
	changeContainerContent(containerIDto, newContainer, newContainerSize);
}



bool DFedit::writeNewFile(const std::string& to) {
	if (!fileRef)
		//std::cout << "origin File not referenced!\n";
		return false;
	if (fileRef->fileHeader.type == 1) {
		//std::cout << "type 1 not supported\n";
		return false;
	}
	else if (fileRef->fileHeader.type == 2) {
		//std::cout << "type 2 not supported\n";
		return false;
	}
	else {
		// construct new file based on container sizes

		uint32_t dataSize{ 0 };
		uint32_t* containerPositions = new uint32_t[fileRef->fileHeader.containerCount];

		uint32_t headerAndRefTableSize = (fileRef->fileHeader.containerCount * sizeof(int32_t)) + 1024;
		uint32_t gapToContainerData = headerAndRefTableSize % 64;
		if (gapToContainerData) {
			gapToContainerData = 64 - gapToContainerData;
		}
		headerAndRefTableSize += gapToContainerData; // sart here


		// calculate size of data section (biggest section), also construct locations for header
		for (int32_t container = 0; container < fileRef->fileHeader.containerCount; container++) {

			// seems like files are splitten in little 64 byte sections, maybe better for the CD to read?
			uint32_t remainder = ((fileRef->containers[container].size + 8) % 64);
			if (remainder) {
				remainder = 64 - remainder;
			}
			containerPositions[container] = dataSize + headerAndRefTableSize;
			dataSize += fileRef->containers[container].size + 8 + remainder;

		}

		// refresh total file size before writing to file
		fileRef->fileHeader.fileSize = dataSize + headerAndRefTableSize;

		std::string newFileName(to);
		//newFileName.append(fileRef->currentFileName);
		std::ofstream fileToWrite(newFileName, std::ios::trunc | std::ios::binary);

		fileToWrite.write((char*)&fileRef->fileHeader, sizeof(fileRef->fileHeader));
		fileToWrite.write((char*)containerPositions, fileRef->fileHeader.containerCount * sizeof(int32_t));
		// fill gap between offset index and container data
		for (uint8_t i = 0; i < gapToContainerData; i++)
			fileToWrite << '\0';

		// now write all containers down to file
		for (int32_t bli = 0; bli < fileRef->fileHeader.containerCount; bli++) {


			fileToWrite.write((char*)&bli, sizeof(bli));
			fileToWrite.write((char*)&fileRef->containers[bli].size, sizeof(fileRef->containers[bli].size));
			fileToWrite.write((char*)fileRef->containers[bli].data, fileRef->containers[bli].size);

			uint32_t diffToNext;
			if (fileRef->fileHeader.containerCount - 1 == bli) {
				diffToNext = fileRef->fileHeader.fileSize - containerPositions[fileRef->fileHeader.containerCount - 1] - (fileRef->containers[fileRef->fileHeader.containerCount - 1].size + 8);
			}
			else
				diffToNext = containerPositions[bli + 1] - containerPositions[bli] - (fileRef->containers[bli].size + 8);
			// fill in empty gab to next container
			for (uint8_t i = 0; i < diffToNext; i++)
				fileToWrite << '\0';
		}
    
		fileToWrite.close();
		delete[] containerPositions;
	}

	return true;
}



// CREATE FUNCTIONS
void DFedit::printHeaderInfo() {
	DFset* setRef = DFset::getDFset(fileRef);
	if (setRef) {
		std::cout << setRef->getHeaderInfo() << std::endl;
		return;
	}
	DFmov* movRef = DFmov::getDFmov(fileRef);
	if (movRef) {
		std::cout << movRef->getHeaderInfo() << std::endl;
		return;
	}
	std::cout << "No function given for this file!\n";
}



std::string DFedit::getDFeditErrorMsg(int32_t& errorCode) {

	// errors codes below 100 come from the inner DFfile lib
	if (errorCode < 100) {
		return fileRef->getErrorMsg(errorCode);
	}
	constexpr int32_t errorCount{ 16 };
	if (errorCode > 100+errorCount)
		return "Unknown Error! Code " + std::to_string(errorCode);

	static std::string errorMsgs[errorCount]
	{
		/* 100*/  {"No file mounted!"},
		/* 101*/  {"File editing for this type not supported yet!"},
		/* 102*/  {"File not found: " + additionalErrorInfo},
		/* 103*/  {"Initial image for color palette missing!"},
		/* 104*/  {"Placeholder"},
		/* 105*/  {"Placeholder"},
		/* 106*/  {"Placeholder"},
		/* 107*/  {"Placeholder"},
		/* 108*/  {"Placeholder"},
		/* 109*/  {"Placeholder"},
		/* 110*/  {"Placeholder"},
		/* 111*/  {"Placeholder"},
		/* 112*/  {"Placeholder"},
		/* 113*/  {"Placeholder"},
		/* 114*/  {"Placeholder"},
		/* 115*/  {"Placeholder"}
	};

	return errorMsgs[errorCode-100];
}
