//
// CoreJSONTests.h
// CoreJSON Framework
//
// Copyright 2011 Mirek Rusin <mirek [at] me [dot] com>
//                http://github.com/mirek/CoreJSON
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include <CoreJSON/CoreJSON.h>
#include "time.h"
#include <CFExtension/CFETests.h>

void testGenerator(void);
void testSimpleStuff(void);
void testUTF8Strings(void);
void testLargeNumbers(void);
void testFloats(void);

#define MAX_FRAMES 100

//void demangle(const char* symbol) {
//  size_t size;
//  int status;
//  char temp[128];
//  char* demangled;
//  //first, try to demangle a c++ name
//  if (1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp)) {
//    if (NULL != (demangled = abi::__cxa_demangle(temp, NULL, &size, &status))) {
//      std::string result(demangled);
//      free(demangled);
//      return result;
//    }
//  }
//  //if that didn't work, try to get a regular c symbol
//  if (1 == sscanf(symbol, "%127s", temp)) {
//    return temp;
//  }
//  
//  //if all else fails, just return the symbol
//  return symbol;
//}

//@interface CoreJSONTests : SenTestCase {
//@private
//  CFAllocatorRef testAllocator;
//}
//
//@end
