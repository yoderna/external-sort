/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//        Project:  External Sort
//
//	    File Name:  FileInteger.h
//
//         Author:  Nicholas Yoder
//
//    Description:  This file contains the FileInteger struct declaration, which represents an integer read
//                  from a file. It contains a pointer to the ifstream object it was read from,	the number
//                  of integers left to read from the file, and the value that this struct represents.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FILEINTEGER_H
#define FILEINTEGER_H

#include <fstream>

// Represents an integer read from a file
struct FileInteger
{
	// Pointer to the ifstream object that this integer was read from
	std::ifstream* ptrFileReadFrom;

	// The number of ints left to read from the file
	int numLeftToRead;

	// The value that was read from the file
	int value;

	// Operator () overload that compares two FileIntegers
	bool operator()(const FileInteger* a, const FileInteger* b)
	{
		return a->value > b->value;
	}
};

#endif