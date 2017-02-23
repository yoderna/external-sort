//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//        Project:    External Sort
//
//      File Name:    main.cpp
//
//         Author:    Nicholas Yoder
//
//    Description:    This file contains the entry point of the program and
//                    functions for performing external sort on a binary file.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <queue>
#include <fstream>
#include <string>

#include "FileInteger.h"

int makeTempFiles(std::ifstream&, int);
void mergeTempFiles(int, int, std::string&);
int fileLen(std::ifstream&);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Function Name:  main
//
//        Purpose:  Receives user input for the unsorted file name, the file to output the sorted
//                  values to, and the maximum number of integers from a file that should be allowed
//                  in memory simultaneously. Then, calls functions to perform the sort.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	std::string unsortedPath, sortedPath;
	int maxFileInts;

	std::cout << "Enter the name/path of the file to sort: ";

	std::getline(std::cin, unsortedPath);

	std::cout << "Enter the name of the sorted file to output: ";

	std::getline(std::cin, sortedPath);

	std::cout << "Enter the maximum number of ints from the\nfile to keep in memory simultaneously: ";

	std::cin >> maxFileInts;

	if (maxFileInts <= 1)
	{
		"Must allow more than one int in memory simultaneously.";
		exit(0);
	}

	// Open the file, and exit if it could not be opened
	std::ifstream inFile(unsortedPath, std::ios::in | std::ios::binary);

	if (!inFile.is_open())
	{
		std::cout << "Error opening input file." << std::endl;
		exit(0);
	}

	// Sort the file
	int numberOfFiles = makeTempFiles(inFile, maxFileInts);

	inFile.close();

	mergeTempFiles(numberOfFiles, maxFileInts, sortedPath);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Function Name:  makeTempFiles
//
//        Purpose:  Where k is the maximum number of ints from a file allowed in memory simultaneously,
//                  this function reads k ints from the unsorted file, sorts them, writes them out to a
//                  new file, and repeats until all ints in the unsorted file have been read.
//
//      Parameter:  unsortedFile is an ifstream object that has already opened the file to sort.
//
//      Parameter:  maxFileInts is the maximum number of integers from the file that are allowed in
//                  memory simultaneously. As a result, it is also the number of integers written to
//                  each temp file.
//
//        Returns:  The number of temp files created.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int makeTempFiles(std::ifstream& unsortedFile, int maxFileInts)
{
	int amountLeftToRead = fileLen(unsortedFile) / sizeof(int);

	int fileNumber = 0;

	while (amountLeftToRead > 0)
	{
		// If the number of integers left to read in the unsorted file is less than the maximum number
		// of integers that can be kept in memory simultaneously, then read only that amount
		int numToRead = (amountLeftToRead < maxFileInts) ? amountLeftToRead : maxFileInts;

		amountLeftToRead -= numToRead;

		// Read the specified number of ints into an array, and then sort the array
		int* sortedValues = new int[numToRead];

		unsortedFile.read((char*)sortedValues, numToRead * sizeof(int));

		std::sort(sortedValues, sortedValues + numToRead);

		// Write sorted data from the current iteration to a new temp file
		std::ofstream outFile(std::to_string(fileNumber), std::ios::out | std::ios::binary);

		outFile.write((char*)sortedValues, sizeof(int) * numToRead);

		outFile.close();

		fileNumber++;

		delete[] sortedValues;
	}

	return fileNumber;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Function Name:  mergeTempFiles
//
//        Purpose:  Continuously merges all temp files created until only one large sorted file remains.
//
//      Parameter:  totalNumberOfFiles is the number of files that need to be merged.
//
//      Parameter:  maxFileInts is the maximum number of integers from a file that are allowed in memory
//                  simultaneously. As a result, it is also the maximum number of files that are merged
//                  at one time.
//
//      Parameter:  sortedPath will be the path/name of the file once it is sorted.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void mergeTempFiles(int totalNumberOfFiles, int maxFileInts, std::string& sortedPath)
{
	// The next file to open and merge
	int currentFileNumToMerge = 0;

	while (currentFileNumToMerge < totalNumberOfFiles)
	{
		int numFilesRemaining = totalNumberOfFiles - currentFileNumToMerge;

		// The number of files opened is equal to the maximum number of integers allowed in memory
		// simultaneously unless that is greater than the number of files that remain to be merged.
		int numFilesToOpen = (numFilesRemaining < maxFileInts) ? numFilesRemaining : maxFileInts;

		// Open all files to merge data from and create a min heap of one integer from each file
		std::ifstream* filesToMerge = new std::ifstream[numFilesToOpen];

		std::vector<FileInteger*> fileData(numFilesToOpen);

		for (int i = 0; i < numFilesToOpen; i++, currentFileNumToMerge++)
		{
			FileInteger* fi = new FileInteger;

			filesToMerge[i].open(std::to_string(currentFileNumToMerge), std::ios::in | std::ios::binary);

			filesToMerge[i].read((char*)&fi->value, sizeof(int));

			fi->ptrFileReadFrom = &filesToMerge[i];

			fi->numLeftToRead = (fileLen(filesToMerge[i]) / sizeof(int)) - 1;

			fileData[i] = fi;
		}

		std::make_heap(fileData.begin(), fileData.end(), FileInteger());


		// While there are still integers left in the heap, remove the smallest integer and write it
		// to the output file. Then, read a new integer from the file it belonged to as long as there
		// is still data left to read from that file.
		std::ofstream outFile(std::to_string(totalNumberOfFiles), std::ios::out | std::ios::binary);

		while (fileData.size() > 0)
		{
			// Remove the smallest FileInteger from the heap
			FileInteger* smallest = fileData.front();

			std::pop_heap(fileData.begin(), fileData.end(), FileInteger());

			fileData.pop_back();

			// Write it to the output file
			outFile.write((char*)&smallest->value, sizeof(int));

			// If there are still ints left to read from the file it belongs to, insert the next into the heap
			if (smallest->numLeftToRead > 0)
			{
				// Create the next FileInteger
				FileInteger* nextInteger = new FileInteger;

				smallest->ptrFileReadFrom->read((char*)&nextInteger->value, sizeof(int));

				nextInteger->numLeftToRead = smallest->numLeftToRead - 1;

				nextInteger->ptrFileReadFrom = smallest->ptrFileReadFrom;

				// Insert it into the heap
				fileData.push_back(nextInteger);

				std::push_heap(fileData.begin(), fileData.end(), FileInteger());
			}

			delete smallest;
		}

		// Close and delete all files that were merged, and delete the array of ifstream objects
		int fileToDelete = currentFileNumToMerge - numFilesToOpen;

		for (int i = 0; i < numFilesToOpen; i++, fileToDelete++)
		{
			filesToMerge[i].close();
			remove(std::to_string(fileToDelete).c_str());
		}

		delete[] filesToMerge;

		if (currentFileNumToMerge != totalNumberOfFiles)
			totalNumberOfFiles++;
	}

	// Rename the sorted file as specified by sortedPath.
	rename(std::to_string(currentFileNumToMerge).c_str(), sortedPath.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Function Name:  fileLen
//
//        Purpose:  Determines the length in bytes of an input file.
//
//      Parameter:  file is an ifstream object with the file to determine the length of already open.
//
//        Returns:  The length of the file in bytes.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int fileLen(std::ifstream& file)
{
	int position = file.tellg();

	file.seekg(0, std::ios::beg);

	int start = file.tellg();

	file.seekg(0, std::ios::end);

	int end = file.tellg();

	file.clear();

	file.seekg(position);

	return end - start;
}