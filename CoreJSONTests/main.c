//
// Created by mitchell on 4/21/2015.
//

#include <CoreJSON/CoreJSON-Prefix.pch>
#include "CoreJSONTests.h"

int main(int argc, char** argv)
{
	CFETestInit();

	testGenerator();
	testSimpleStuff();
	testUTF8Strings();
	testLargeNumbers();
	testFloats();
	
	CFETestFinalize();

	CFETestReturn();
}