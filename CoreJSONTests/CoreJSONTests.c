//
// Created by mitchell on 4/21/2015.
//

#include <CoreJSON/CoreJSON-Prefix.pch>
#include "CoreJSONTests.h"
#include <CFExtension/CFExtension.h>

CFAllocatorRef setup_CoreJSONTests(void)
{
	return TestAllocatorCreate();
}

void tearDown(CFAllocatorRef allocator)
{
	CFETestBegin();

	CFETestTrue(TestAllocatorGetAllocationsCount(allocator) > 0, CFSTR("Allocations count not greater than 0."))
	CFETestTrue(TestAllocatorGetDeallocationsCount(allocator) > 0, CFSTR("Deallocations count not greater than 0."));

	if(TestAllocatorGetAllocationsCount(allocator) != TestAllocatorGetDeallocationsCount(allocator))
		TestAllocatorPrintAddressesAndBacktraces(allocator);
	CFETestEqual(TestAllocatorGetAllocationsCount(allocator), TestAllocatorGetDeallocationsCount(allocator), CFSTR("Allocations count not equal to deallocations count."));

	CFETestEnd();
}

void testGenerator(void)
{
	CFETestBegin();

	CFUUIDRef uuid = CFUUIDCreate(NULL);

	CFMutableStringRef expected = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFSTR("{\"uuid\":\""));
	CFStringRef uuidString = CFUUIDCreateString(kCFAllocatorDefault, uuid);
	CFStringAppend(expected, uuidString);
	CFStringAppend(expected, CFSTR("\",\"number\":9223372036854775807}"));
	CFRelease(uuidString);

	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	long long number = 9223372036854775807;
	CFDictionaryAddValue(dict, CFSTR("number"), CFNumberCreateWithLongLong(number));
	CFDictionaryAddValue(dict, CFSTR("uuid"), uuid);

	CFErrorRef error = NULL;

	CFStringRef json = JSONCreateString(kCFAllocatorDefault, dict, kJSONWriteOptionsDefault, &error);

	CFRelease(dict);

	CFETestNotNull(json, CFSTR("Failed to create JSON string."));
	CFETestEqualObjects(expected, json, CFSTR("Test string doesn't match expected."));

	CFRelease(json);

	CFETestEnd();
}

void testSimpleStuff(void)
{
	CFETestBegin();

	int32_t int1 = 1;
	int32_t int2 = 3;
	int32_t int3 = 5;

	CFMutableArrayRef expectedArray = CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);

	CFArrayAppendValue(expectedArray, CFNumberCreateWithSInt32(int1));
	CFArrayAppendValue(expectedArray, CFNumberCreateWithSInt32(int2));
	CFArrayAppendValue(expectedArray, CFNumberCreateWithSInt32(int3));

	CFErrorRef error = NULL;
	CFArrayRef array = JSONCreateWithString(kCFAllocatorDefault, CFSTR("[1, 3, 5]"), kJSONReadOptionsDefault, &error);
	CFETestNotNull(array, CFSTR("Failed to create array."));
	CFETestEqualObjects(expectedArray, array, CFSTR("Test array doesn't match expected."));

	CFRelease(array);

	CFMutableDictionaryRef expectedDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDictionaryAddValue(expectedDict, CFSTR("a"), CFNumberCreateWithSInt32(int1));
	CFDictionaryAddValue(expectedDict, CFSTR("b"), CFNumberCreateWithSInt32(int2));
	CFDictionaryAddValue(expectedDict, CFSTR("c"), CFNumberCreateWithSInt32(int3));

	CFDictionaryRef dict = JSONCreateWithString(kCFAllocatorDefault, CFSTR("{ \"a\": 1, \"b\": 3, \"c\": 5 }"), kJSONReadOptionsDefault, &error);
	CFETestNotNull(dict, CFSTR("Failed to create dictionary."));
	CFETestEqualObjects(expectedDict, dict, CFSTR("Test dictionary doesn't match expected."));

	CFRelease(dict);

	CFETestEnd();
}

void testUTF8Strings(void)
{
	CFETestBegin();

	CFErrorRef error;
	CFArrayRef array = JSONCreateWithString(kCFAllocatorDefault, CFSTR("[\"a’la\"]"), kJSONReadOptionsDefault, &error);
	CFETestNotNull(array, CFSTR("Failed to create array."));
	CFShow(array);
	CFRelease(array);

	CFETestEnd();
}

void testLargeNumbers(void)
{
	CFETestBegin();

	long long long1 = 4294967297;
	long long long2 = 9223372036854775807;
	long long long3 = -9223372036854775807;

	CFMutableArrayRef expected = CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);

	CFArrayAppendValue(expected, CFNumberCreateWithLongLong(long1));
	CFArrayAppendValue(expected, CFNumberCreateWithLongLong(long2));
	CFArrayAppendValue(expected, CFNumberCreateWithLongLong(long3));

	CFErrorRef error;
	CFArrayRef array = JSONCreateWithString(kCFAllocatorDefault, CFSTR("[4294967297, 9223372036854775807, -9223372036854775807]"), kJSONReadOptionsDefault, &error);
	CFETestNotNull(array, CFSTR("Failed to create array."));
	CFETestEqualObjects(expected, array, CFSTR("Test array does not match expected."));

	CFRelease(expected);
	CFRelease(array);

	CFETestEnd();
}

void testFloats(void)
{
	CFETestBegin();

	float float1 = 3.14159265f;
	float float2 = 1.61803399f;
	float float3 = -57.2957795f;

	CFMutableArrayRef expected = CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);

	CFArrayAppendValue(expected, CFNumberCreateWithFloat(3.14159265f));
	CFArrayAppendValue(expected, CFNumberCreateWithFloat(1.61803399f));
	CFArrayAppendValue(expected, CFNumberCreateWithFloat(-57.2957795f));

	CFErrorRef error;
	CFArrayRef array = JSONCreateWithString(kCFAllocatorDefault, CFSTR("[3.14159265, 1.61803399, -57.2957795]"), kJSONReadOptionsDefault, &error);
	CFETestNotNull(array, CFSTR("Failed to create array."));
	CFETestEqualObjects(expected, array, CFSTR("Test array does not match expected."));

	CFRelease(expected);
	CFRelease(array);

	CFETestEnd();
}
