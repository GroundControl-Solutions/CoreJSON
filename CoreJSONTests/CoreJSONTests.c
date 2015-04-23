//
// Created by mitchell on 4/21/2015.
//

#ifdef __MINGW32__
#include <CoreJSON/CoreJSON-Prefix.pch>
#endif

#include "CoreJSONTests.h"
#include <CFExtension/CFExtension.h>

void testGenerator(void)
{
	CFETestBegin();

	CFUUIDRef uuid = CFUUIDCreateFromString(kCFAllocatorDefault, CFSTR("3DE58A99-5619-43D0-93FB-DCE82EB17BD3"));
	
	CFStringRef expected1 = CFSTR("{\"uuid\":\"3DE58A99-5619-43D0-93FB-DCE82EB17BD3\",\"number\":9223372036854775807}");
	CFStringRef expected2 = CFSTR("{\"number\":9223372036854775807,\"uuid\":\"3DE58A99-5619-43D0-93FB-DCE82EB17BD3\"}");
	
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	long long number = 9223372036854775807;
	CFDictionaryAddValue(dict, CFSTR("number"), CFNumberCreateWithLongLong(number));
	CFDictionaryAddValue(dict, CFSTR("uuid"), uuid);

	CFErrorRef error = NULL;

	CFStringRef json = JSONCreateString(kCFAllocatorDefault, dict, kJSONWriteOptionsDefault, &error);

	CFRelease(dict);

	CFETestNotNull(json, CFSTR("Failed to create JSON string."));
	CFETestTrue(CFEqual(json, expected1) || CFEqual(json, expected2), CFSTR("Test string doesn't match any possible expected."));

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

	CFDataRef expectedData = CFDataCreateFromHexadecimalStringRepresentation(CFSTR("61E280996C61"));
	CFStringRef expectedString = CFDataCopyUTF8String(expectedData);
	CFMutableArrayRef expected = CFArrayCreateMutable(kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(expected, expectedString);

	CFRelease(expectedData);

	CFErrorRef error;
	CFDataRef utf8data = CFDataCreateFromHexadecimalStringRepresentation(CFSTR("5B2261E280996C61225D"));
	CFStringRef utf8string = CFDataCopyUTF8String(utf8data);
	CFArrayRef array = JSONCreateWithString(kCFAllocatorDefault, utf8string, kJSONReadOptionsDefault, &error);
	CFETestNotNull(array, CFSTR("Failed to create array."));
	CFETestEqualObjects(expected, array, CFSTR("Test array doesn't match expected."));

	CFRelease(expected);
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

	CFMutableArrayRef expected = CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);

	CFArrayAppendValue(expected, CFNumberCreateWithDouble(3.14159265));
	CFArrayAppendValue(expected, CFNumberCreateWithDouble(1.61803399));
	CFArrayAppendValue(expected, CFNumberCreateWithDouble(-57.2957795));

	CFErrorRef error;
	CFArrayRef array = JSONCreateWithString(kCFAllocatorDefault, CFSTR("[3.14159265, 1.61803399, -57.2957795]"), kJSONReadOptionsDefault, &error);
	CFETestNotNull(array, CFSTR("Failed to create array."));
	CFETestEqualObjects(expected, array, CFSTR("Test array does not match expected."));

	CFRelease(expected);
	CFRelease(array);

	CFETestEnd();
}
