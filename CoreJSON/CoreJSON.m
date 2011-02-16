//
// CoreJSON.m
// CoreJSON
//
// Copyright 2011 Mirek Rusin <mirek [at] me [dot] com> http://github.com/mirek/CoreJSON
//

#include "CoreJSON.h"

__JSONUTF8String __JSONUTF8StringMake(CFAllocatorRef allocator, CFStringRef string) {
  __JSONUTF8String utf8String;
  utf8String.allocator = allocator;
  utf8String.string = string;
  utf8String.maximumSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8) + 1;
  if ((utf8String.pointer = (const unsigned char *)CFStringGetCStringPtr(string, kCFStringEncodingUTF8))) {
    utf8String.buffer = NULL;
  } else {
    utf8String.buffer = CFAllocatorAllocate(allocator, utf8String.maximumSize, 0);
    if (utf8String.buffer) {
      CFStringGetCString(string, (char *)utf8String.buffer, utf8String.maximumSize, kCFStringEncodingUTF8);
    }
  }
  return utf8String;
}

const unsigned char *__JSONUTF8StringGetBuffer(__JSONUTF8String utf8String) {
  return utf8String.pointer ? utf8String.pointer : utf8String.buffer;
}

CFIndex __JSONUTF8StringGetMaximumSize(__JSONUTF8String utf8String) {
  return utf8String.maximumSize;
}

void __JSONUTF8StringDestroy(__JSONUTF8String utf8String) {
  if (utf8String.buffer) {
    CFAllocatorDeallocate(utf8String.allocator, (void *)utf8String.buffer);
  }
}

#pragma Helper stack

__JSONStackEntryRef __JSONStackEntryCreate(CFAllocatorRef allocator, CFIndex index, CFIndex valuesLength, CFIndex keysLength) {
  __JSONStackEntryRef entry = CFAllocatorAllocate(allocator, sizeof(__JSONStackEntry), 0);
  if (entry) {
    entry->allocator = allocator;
    entry->index = index;
    entry->valuesIndex = 0;
    if ((entry->valuesLength = valuesLength))
      entry->values = CFAllocatorAllocate(entry->allocator, valuesLength, 0);
    else
      entry->values = NULL;
    entry->keysIndex = 0;
    if ((entry->keysLength = keysLength))
      entry->keys = CFAllocatorAllocate(entry->allocator, keysLength, 0);
    else
      entry->keys = NULL;
  }
  return entry;
}

__JSONStackEntryRef __JSONStackEntryRetain(__JSONStackEntryRef entry) {
  entry->retainCount++;
  return entry;
}

CFIndex __JSONStackEntryRelease(__JSONStackEntryRef entry) {
  return --entry->retainCount;
}

__JSONStackEntryRef __JSONStackEntryReleaseRef(__JSONStackEntryRef *entry) {
  if (__JSONStackEntryRelease(*entry))
    entry = NULL;
  return *entry;
}

void __JSONStackEntryAppendValue(__JSONStackEntryRef entry, CFTypeRef value) {
  // TODO: Reallocate when out of bounds
  entry->values[entry->valuesIndex++] = value;
}

void __JSONStackEntryAppendKey(__JSONStackEntryRef entry, CFTypeRef key) {
  // TODO: Reallocate when out of bounds
  entry->keys[entry->keysIndex++] = key;
}

__JSONStackRef __JSONStackCreate(CFAllocatorRef allocator, CFIndex maxDepth) {
  __JSONStackRef stack = CFAllocatorAllocate(allocator, sizeof(__JSONStack), 0);
  if (stack) {
    stack->allocator = allocator;
    stack->size = maxDepth;
    stack->index = 0;
    stack->stack = CFAllocatorAllocate(stack->allocator, sizeof(__JSONStackEntryRef) * stack->size, 0);
    memset(stack->stack, 0, sizeof(__JSONStackEntryRef) * stack->size);
  }
  return stack;
}

__JSONStackEntryRef __JSONStackGetTop(__JSONStackRef stack) {
  if (stack->index > 0)
    return stack->stack[stack->index - 1];
  else
    return NULL;
}

BOOL __JSONStackPush(__JSONStackRef stack, __JSONStackEntryRef entry) {
  BOOL success = NO;
  if (stack && entry) {
    if (stack->index < stack->size) {
      stack->stack[stack->index++] = __JSONStackEntryRetain(entry);
    } else {
      // TODO: Out of bounds
    }
  }
  return success;
}

__JSONStackEntryRef __JSONStackPop(__JSONStackRef stack) {
  __JSONStackEntryRef entry = NULL;
  if (stack) {
    if (--stack->index >= 0) {
      entry = stack->stack[stack->index];
    } else {
      // TODO: Out of bounds
    }
  }
  return entry;
}

BOOL __JSONStackAppendValueAtTop(__JSONStackRef stack, CFTypeRef value) {
  BOOL success = NO;
  if (stack) {
    __JSONStackEntryRef entry = __JSONStackGetTop(stack);
    if (entry) {
      __JSONStackEntryAppendValue(entry, value);
      success = YES;
    }
  }
  return success;
}

BOOL __JSONStackAppendKeyAtTop(__JSONStackRef stack, CFTypeRef key) {
  BOOL success = NO;
  if (stack) {
    __JSONStackEntryRef entry = __JSONStackGetTop(stack);
    if (entry) {
      __JSONStackEntryAppendKey(entry, key);
      success = YES;
    }
  }
  return success;
}

#pragma Parser callbacks

int JSONParserAppendStringWithBytes(void *context, const unsigned char *value, unsigned int length) {
  CoreJSONRef json = (CoreJSONRef)context;
  CFStringRef string = CFStringCreateWithBytes(json->allocator, value, length, kCFStringEncodingUTF8, NO);
  CFArrayAppendValue(json->elements, string);
  __JSONStackAppendValueAtTop(json->stack, string);
  CFRelease(string);
  return 1;
}

int JSONParserAppendNull(void *context) {
  CoreJSONRef json = (CoreJSONRef)context;
  CFArrayAppendValue(json->elements, kCFNull);
  __JSONStackAppendValueAtTop(json->stack, kCFNull);
  return 1;
}

int JSONParserAppendBooleanWithInteger(void *context, int value) {
  CoreJSONRef json = (CoreJSONRef)context;
  CFBooleanRef boolean = value ? kCFBooleanTrue : kCFBooleanFalse;
  CFArrayAppendValue(json->elements, boolean);
  __JSONStackAppendValueAtTop(json->stack, boolean);
  return 1;
}

//int JSONParserAppendNumberWithString(void *context, const char *value, unsigned int length) {
//  CFStringRef string = CFStringCreateWithBytes(NULL, (const UInt8 *)s, l, NSUTF8StringEncoding, YES);
//  //  CFArrayAppendValue(json->elements, string);
//  CFRelease(string);
//  return 1;
//}

int JSONParserAppendNumberWithLong(void *context, long value) {
  CoreJSONRef json = (CoreJSONRef)context;
  CFNumberRef number = CFNumberCreate(json->allocator, kCFNumberLongType, &value);
  CFArrayAppendValue(json->elements, number);
  __JSONStackAppendValueAtTop(json->stack, number);
  CFRelease(number);
  return 1;
}

int JSONParserAppendNumberWithDouble(void *context, double value) {
  CoreJSONRef json = (CoreJSONRef)context;
  CFNumberRef number = CFNumberCreate(json->allocator, kCFNumberDoubleType, &value);
  CFArrayAppendValue(json->elements, number);
  __JSONStackAppendValueAtTop(json->stack, number);
  CFRelease(number);
  return 1;
}

int JSONParserAppendMapKeyWithBytes(void *context, const unsigned char *value, unsigned int length) {
  CoreJSONRef json = (CoreJSONRef)context;
  CFStringRef string = CFStringCreateWithBytes(json->allocator, value, length, kCFStringEncodingUTF8, NO);
  CFArrayAppendValue(json->elements, string);
  __JSONStackAppendKeyAtTop(json->stack, string);
  CFRelease(string);
  return 1;
}

int JSONParserAppendMapStart(void *context) {
  CoreJSONRef json = (CoreJSONRef)context;
  
  // Placeholder for the CFDictionaryRef which will come later
  CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(json->allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFArrayAppendValue(json->elements, dictionary);
  __JSONStackAppendValueAtTop(json->stack, dictionary);
  CFRelease(dictionary);
  
  // New container to the stack
  __JSONStackPush(json->stack, __JSONStackEntryCreate(json->allocator, CFArrayGetCount(json->elements) - 1, 1024, 1024));
  
  return 1;
}

int JSONParserAppendMapEnd(void *context) {
  CoreJSONRef json = (CoreJSONRef)context;
  __JSONStackEntryRef entry = __JSONStackPop(json->stack);
  
  // TODO: Optimize creating dict
  // CFDictionaryRef dictionary = CFDictionaryCreate(json->allocator, entry->keys, entry->values, entry->valuesIndex, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  // CFArraySetValueAtIndex(json->elements, entry->index, dictionary);
  
  CFMutableDictionaryRef d = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(json->elements, entry->index);
  for (int i = 0; i < entry->keysIndex; i++)
    CFDictionaryAddValue(d, entry->keys[i], entry->values[i]);
  
  __JSONStackEntryRelease(entry);
  return 1;
}

int JSONParserAppendArrayStart(void *context) {
  CoreJSONRef json = (CoreJSONRef)context;
  
  // Let's save this space in elements for the array. When we
  // reach the end of the array, we'll create one in this place.
  CFMutableArrayRef array = CFArrayCreateMutable(json->allocator, 0, &kCFTypeArrayCallBacks);
  CFArrayAppendValue(json->elements, array);
  __JSONStackAppendValueAtTop(json->stack, array);
  CFRelease(array);
  
  // Add the container to the stack. Container element is just
  // 3-value array [container index in the elements array, 
  __JSONStackEntryRef entry = __JSONStackEntryCreate(json->allocator, CFArrayGetCount(json->elements) - 1, 1024, 0);
  __JSONStackPush(json->stack, entry);
  __JSONStackEntryRelease(entry);
  
  return 1;
}

int JSONParserAppendArrayEnd(void *context) {
  CoreJSONRef json = (CoreJSONRef)context;
  __JSONStackEntryRef entry = __JSONStackPop(json->stack);
  
  CFArrayRef array = CFArrayCreate(json->allocator, entry->values, entry->valuesIndex, &kCFTypeArrayCallBacks);
  CFMutableArrayRef a2 = (CFMutableArrayRef)CFArrayGetValueAtIndex(json->elements, entry->index);
  CFArrayAppendArray(a2, array, CFRangeMake(0, CFArrayGetCount(array)));
  //CFArraySetValueAtIndex(json->elements, entry->index, array);
  CFRelease(array);
  
  __JSONStackEntryRelease(entry);
  return 1;
}

CoreJSONRef JSONCreate(CFAllocatorRef allocator) {
  CoreJSONRef json = CFAllocatorAllocate(allocator, sizeof(CoreJSON), 0);
  if (json) {
    json->allocator = allocator;
    
    json->yajlParserCallbacks.yajl_null        = JSONParserAppendNull;
    json->yajlParserCallbacks.yajl_boolean     = JSONParserAppendBooleanWithInteger;
    
    // Set number or integer and double. Never all 3.
    json->yajlParserCallbacks.yajl_number      = NULL;
    json->yajlParserCallbacks.yajl_integer     = JSONParserAppendNumberWithLong;
    json->yajlParserCallbacks.yajl_double      = JSONParserAppendNumberWithDouble;
    
    json->yajlParserCallbacks.yajl_start_map   = JSONParserAppendMapStart;
    json->yajlParserCallbacks.yajl_map_key     = JSONParserAppendMapKeyWithBytes;
    json->yajlParserCallbacks.yajl_end_map     = JSONParserAppendMapEnd;
    
    json->yajlParserCallbacks.yajl_start_array = JSONParserAppendArrayStart;
    json->yajlParserCallbacks.yajl_end_array   = JSONParserAppendArrayEnd;
    
    json->yajlParserCallbacks.yajl_string      = JSONParserAppendStringWithBytes;
    
    json->yajlParserConfig.allowComments = YES;
    json->yajlParserConfig.checkUTF8 = YES;
    json->yajlParser = yajl_alloc(&json->yajlParserCallbacks, &json->yajlParserConfig, NULL, (void *)json);
    
    json->yajlGeneratorConfig.beautify = YES;
    json->yajlGeneratorConfig.indentString = "  ";
    json->yajlGenerator = yajl_gen_alloc(&json->yajlGeneratorConfig, NULL);
    
    json->elements = CFArrayCreateMutable(json->allocator, 0, &kCFTypeArrayCallBacks);
    json->stack = __JSONStackCreate(json->allocator, CORE_JSON_STACK_MAX_DEPTH);
  }
  return json;
}

void JSONParseWithString(CoreJSONRef json, CFStringRef string) {
  
  // Let's make sure we've got a clean plate first
  CFArrayRemoveAllValues(json->elements);
  
  __JSONUTF8String utf8 = __JSONUTF8StringMake(json->allocator, string);
  if ((json->yajlParserStatus = yajl_parse(json->yajlParser, __JSONUTF8StringGetBuffer(utf8), __JSONUTF8StringGetMaximumSize(utf8))) != yajl_status_ok) {
    // TODO: Error stuff
    //printf("ERROR: %s\n", yajl_get_error(json->yajlParser, YES, __JSONUTF8StringGetBuffer(utf8), __JSONUTF8StringGetMaximumSize(utf8)));
  }
  __JSONUTF8StringDestroy(utf8);
  
  json->yajlParserStatus = yajl_parse_complete(json->yajlParser);
  
  yajl_free(json->yajlParser);
}

CoreJSONRef JSONCreateWithString(CFAllocatorRef allocator, CFStringRef string) {
  CoreJSONRef json = JSONCreate(allocator);
  if (json) {
    JSONParseWithString(json, string);
  }
  return json;
}

CFTypeRef JSONGetObject(CoreJSONRef json) {
  return CFArrayGetObjectAtIndex(json->elements, 0);
}
