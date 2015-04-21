//
// Created by mitchell on 4/21/2015.
//

#include "CoreJSONTests.h"

int main(int argc, char** argv)
{
//	CFAllocatorRef allocator = setup_CoreJSONTests();

	CFETestInit();

	testGenerator();
	testSimpleStuff();
	testUTF8Strings();
	testLargeNumbers();
	testFloats();
	
	CFETestFinalize();

	CFETestReturn();
}