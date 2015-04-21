//
// CoreJSON.c
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

#include "CoreJSON.h"

// Internal helper macro for appending elements
#define __JSON_CONSUME_AND_RETURN(create) \
  CFTypeRef __json_element = (create); \
  int __json_return = __JSONStackAppendValueAtTop(json->stack, __JSONElementsAppend(json, __json_element)); \
  CFRelease(__json_element); \
  return __json_return

#pragma Internal stack

inline __JSONStackEntryRef __JSONStackEntryCreate(CFAllocatorRef allocator, CFIndex index, CFIndex valuesInitialSize, CFIndex keysInitialSize) {
  __JSONStackEntryRef entry = CFAllocatorAllocate(allocator, sizeof(__JSONStackEntry), 0);
  if (entry) {
    entry->allocator = allocator ? CFRetain(allocator) : NULL;
    entry->retainCount = 1;
    entry->index = index;
    entry->valuesIndex = 0;
    if ((entry->valuesSize = valuesInitialSize))
      entry->values = CFAllocatorAllocate(entry->allocator, sizeof(CFIndex) * entry->valuesSize, 0);
    else
      entry->values = NULL;
    entry->keysIndex = 0;
    if ((entry->keysSize = keysInitialSize))
      entry->keys = CFAllocatorAllocate(entry->allocator, sizeof(CFIndex) * entry->keysSize, 0);
    else
      entry->keys = NULL;
  }
  return entry;
}

inline __JSONStackEntryRef __JSONStackEntryRetain(__JSONStackEntryRef entry) {
  entry->retainCount++;
  return entry;
}

inline CFIndex __JSONStackEntryRelease(__JSONStackEntryRef entry) {
  CFIndex retainCount = 0;
  if (entry) {
    if ((retainCount = --entry->retainCount) == 0) {
      CFAllocatorRef allocator = entry->allocator;
      if (entry->values)
        CFAllocatorDeallocate(allocator, entry->values);
      if (entry->keys)
        CFAllocatorDeallocate(allocator, entry->keys);
      CFAllocatorDeallocate(allocator, entry);
      if (allocator)
        CFRelease(allocator);
    }
  }
  return retainCount;
}

inline bool __JSONStackEntryAppendValue(__JSONStackEntryRef entry, CFIndex value) {
  bool success = 0;
  if (entry) {
    if (entry->valuesIndex == entry->valuesSize) { // Reallocate more space
      CFIndex largerSize = entry->valuesSize ? entry->valuesSize << 1 : CORE_JSON_STACK_ENTRY_VALUES_INITIAL_SIZE;
      CFIndex *largerValues = CFAllocatorReallocate(entry->allocator, entry->values, sizeof(CFIndex) * largerSize, 0);
      if (largerValues) {
        entry->valuesSize = largerSize;
        entry->values = largerValues;
      }
    }
    if (entry->valuesIndex < entry->valuesSize) {
      entry->values[entry->valuesIndex++] = value;
      success = 1;
    }
  }
  return success;
}

inline bool __JSONStackEntryAppendKey(__JSONStackEntryRef entry, CFIndex key) {
  bool success = 0;
  if (entry) {
    if (entry->keysIndex == entry->keysSize) { // Reallocate more space
      CFIndex largerSize = entry->keysSize ? entry->keysSize << 1 : CORE_JSON_STACK_ENTRY_KEYS_INITIAL_SIZE;
      CFIndex *largerKeys = CFAllocatorReallocate(entry->allocator, entry->keys, sizeof(CFIndex) * largerSize, 0);
      if (largerKeys) {
        entry->keysSize = largerSize;
        entry->keys = largerKeys;
      }
    }
    if (entry->keysIndex < entry->keysSize) {
      entry->keys[entry->keysIndex++] = key;
      success = 1;
    }
  }
  return success;
}

inline CFTypeRef *__JSONStackEntryCreateValues(__JSONStackEntryRef entry, CFTypeRef *elements) {
  CFTypeRef *values = CFAllocatorAllocate(entry->allocator, sizeof(CFTypeRef) * entry->valuesIndex, 0);
  if (values)
    for (CFIndex i = 0; i < entry->valuesIndex; i++)
      values[i] = elements[entry->values[i]];
  return values;
}

inline CFTypeRef *__JSONStackEntryCreateKeys(__JSONStackEntryRef entry, CFTypeRef *elements) {
  CFTypeRef *keys = CFAllocatorAllocate(entry->allocator, sizeof(CFTypeRef) * entry->keysIndex, 0);
  if (keys)
    for (CFIndex i = 0; i < entry->keysIndex; i++)
      keys[i] = elements[entry->keys[i]];
  return keys;
}

inline __JSONStackRef __JSONStackCreate(CFAllocatorRef allocator, CFIndex initialSize) {
  __JSONStackRef stack = CFAllocatorAllocate(allocator, sizeof(__JSONStack), 0);
  if (stack) {
    stack->allocator = allocator ? CFRetain(allocator) : NULL;
    stack->retainCount = 1;
    stack->size = initialSize;
    stack->index = 0;
    stack->stack = CFAllocatorAllocate(stack->allocator, sizeof(__JSONStackEntryRef) * stack->size, 0);
    memset(stack->stack, 0, sizeof(__JSONStackEntryRef) * stack->size);
  }
  return stack;
}

inline __JSONStackRef __JSONStackRelease(__JSONStackRef stack) {
  if (stack) {
    if ( --stack->retainCount == 0) {
      CFAllocatorRef allocator = stack->allocator;
      if (stack->stack) {
        while (--stack->index >= 0)
          __JSONStackEntryRelease(stack->stack[stack->index]);
        if (stack->stack)
          CFAllocatorDeallocate(allocator, stack->stack);
        stack->stack = NULL;
      }
      CFAllocatorDeallocate(allocator, stack);
      stack = NULL;
      if (allocator)
        CFRelease(allocator);
    }
  }
  return stack;
}

inline __JSONStackEntryRef __JSONStackGetTop(__JSONStackRef stack) {
  if (stack->index > 0)
    return stack->stack[stack->index - 1];
  else
    return NULL;
}

inline bool __JSONStackPush(__JSONStackRef stack, __JSONStackEntryRef entry) {
  bool success = 0;
  if (stack && entry) {
    
    // Do we need more space? Reallocate to 2 * current size.
    if (stack->index == stack->size) {
      CFIndex largerSize = stack->size ? stack->size << 1 : CORE_JSON_STACK_INITIAL_SIZE;
      __JSONStackEntryRef *largerStack = CFAllocatorReallocate(stack->allocator, stack->stack, sizeof(__JSONStackEntryRef) * largerSize, 0);
      if (largerStack) {
        stack->size = largerSize;
        stack->stack = largerStack;
      }
    }
    if (stack->index < stack->size) {
      stack->stack[stack->index++] = __JSONStackEntryRetain(entry);
      success = 1;
    }
  }
  return success;
}

// The caller is responsible for releasing returned __JSONStackEntryRef
inline __JSONStackEntryRef __JSONStackPop(__JSONStackRef stack) {
  __JSONStackEntryRef entry = NULL;
  if (stack) {
    if (--stack->index >= 0) {
      entry = stack->stack[stack->index];
    } else {
      // TODO: Out of bounds, this is error, should never happen
    }
  }
  return entry;
}

inline bool __JSONStackAppendValueAtTop(__JSONStackRef stack, CFIndex value) {
  return __JSONStackEntryAppendValue(__JSONStackGetTop(stack), value);
}

inline bool __JSONStackAppendKeyAtTop(__JSONStackRef stack, CFIndex key) {
  return __JSONStackEntryAppendKey(__JSONStackGetTop(stack), key);
}

#pragma Parser callbacks

inline int __JSONParserAppendStringWithBytes(void *context, const unsigned char *value, size_t length) {
  __JSONRef json = (__JSONRef)context;
  __JSON_CONSUME_AND_RETURN(CFStringCreateWithBytes(json->allocator, value, length, kCFStringEncodingUTF8, 0));
}

inline int __JSONParserAppendNull(void *context) {
  __JSONRef json = (__JSONRef)context;
  return __JSONStackAppendValueAtTop(json->stack, __JSONElementsAppend(json, kCFNull));
}

inline int __JSONParserAppendBooleanWithInteger(void *context, int value) {
  __JSONRef json = (__JSONRef)context;
  return __JSONStackAppendValueAtTop(json->stack, __JSONElementsAppend(json, value ? kCFBooleanTrue : kCFBooleanFalse));
}

inline int __JSONParserAppendNumberWithBytes(void *context, const char *value, size_t length) {
  __JSONRef json = (__JSONRef)context;
  CFNumberRef number = NULL;

  // TODO: How to do it better? Anybody?
  bool looksLikeFloat = 0;
  for (int i = 0; i < length; i++)
    if (value[i] == '.' || value[i] == 'e' || value[i] == 'E')
      looksLikeFloat = 1;

  if (looksLikeFloat) {
    double value_ = strtod((char *)value, NULL);
    number = CFNumberCreate(json->allocator, kCFNumberDoubleType, &value_);
  } else {
    long long value_ = strtoll((char *)value, NULL, 0);
    number = CFNumberCreate(json->allocator, kCFNumberLongLongType, &value_);
  }
  
  __JSON_CONSUME_AND_RETURN(number);
}

inline int __JSONParserAppendNumberWithLong(void *context, long value) {
  __JSONRef json = (__JSONRef)context;
  __JSON_CONSUME_AND_RETURN(CFNumberCreate(json->allocator, kCFNumberLongType, &value));
}

inline int __JSONParserAppendNumberWithDouble(void *context, double value) {
  __JSONRef json = (__JSONRef)context;
  __JSON_CONSUME_AND_RETURN(CFNumberCreate(json->allocator, kCFNumberDoubleType, &value));
}

// LOOK OUT! Appending to key array, not the value array
inline int __JSONParserAppendMapKeyWithBytes(void *context, const unsigned char *value, size_t length) {
  __JSONRef json = (__JSONRef)context;
  CFTypeRef __json_element = CFStringCreateWithBytes(json->allocator, value, length, kCFStringEncodingUTF8, 0);
  int __json_return = __JSONStackAppendKeyAtTop(json->stack, __JSONElementsAppend(json, __json_element));
  CFRelease(__json_element);
  return __json_return;
}

inline int __JSONParserAppendMapStart(void *context) {
  __JSONRef json = (__JSONRef)context;
  
  // Placeholder for the CFDictionaryRef which will be set when we get map end token
  CFIndex index = __JSONElementsAppend(json, kCFNull);
  
  // Add element to the parent container
  __JSONStackAppendValueAtTop(json->stack, index);
  
  // Push this element as the new container
  __JSONStackEntryRef entry = __JSONStackEntryCreate(json->allocator, index, CORE_JSON_STACK_ENTRY_VALUES_INITIAL_SIZE, CORE_JSON_STACK_ENTRY_KEYS_INITIAL_SIZE);
  __JSONStackPush(json->stack, entry);
  __JSONStackEntryRelease(entry);
  return 1;
}

inline int __JSONParserAppendMapEnd(void *context) {
  int success = 0;
  __JSONRef json = (__JSONRef)context;
  __JSONStackEntryRef entry = __JSONStackPop(json->stack);
  if (entry) {
    if (entry->keysIndex == entry->valuesIndex) {
      CFTypeRef *keys = __JSONStackEntryCreateKeys(entry, json->elements);
      if (keys) {
        CFTypeRef *values = __JSONStackEntryCreateValues(entry, json->elements);
        if (values) {
          json->elements[entry->index] = CFDictionaryCreate(json->allocator, keys, values, entry->keysIndex, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
          CFAllocatorDeallocate(entry->allocator, values);
          success = 1;
        }
        CFAllocatorDeallocate(entry->allocator, keys);
      } else {
        json->elements[entry->index] = CFDictionaryCreate(json->allocator, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        success = 1;
      }
    } else {
      // TODO: The number of keys and values does not match
    }
    __JSONStackEntryRelease(entry);
  } else {
    // TODO: Container on the stack can't be NULL (too deep?)
  }
  return success;
}

inline int __JSONParserAppendArrayStart(void *context) {
  __JSONRef json = (__JSONRef)context;
  
  // Placeholder for the CFArrayRef which will be set when we get array end token
  CFIndex index = __JSONElementsAppend(json, kCFNull);
  
  // Add element to the parent container
  __JSONStackAppendValueAtTop(json->stack, index);
  
  // Push this element as the new container
  __JSONStackEntryRef entry = __JSONStackEntryCreate(json->allocator, index, CORE_JSON_STACK_ENTRY_VALUES_INITIAL_SIZE, 0);
  __JSONStackPush(json->stack, entry);
  __JSONStackEntryRelease(entry);
  
  return 1;
}

inline int __JSONParserAppendArrayEnd(void *context) {
  int success = 0;
  __JSONRef json = (__JSONRef)context;
  __JSONStackEntryRef entry = __JSONStackPop(json->stack);
  if (entry) {
    CFTypeRef *values = __JSONStackEntryCreateValues(entry, json->elements); // TODO: change allocation to here.
    if (values) {
      json->elements[entry->index] = CFArrayCreate(json->allocator, values, entry->valuesIndex, &kCFTypeArrayCallBacks);
      CFAllocatorDeallocate(entry->allocator, values);
      success = 1;
    } else {
      json->elements[entry->index] = CFArrayCreate(json->allocator, NULL, 0, &kCFTypeArrayCallBacks);
      success = 1;  
    }
    __JSONStackEntryRelease(entry);
  }
  return success;
}

inline CFIndex __JSONElementsAppend(__JSONRef json, CFTypeRef value) {
  CFIndex index = json->elementsIndex;
  if (json->elementsIndex == json->elementsSize) { // Reallocate
    CFIndex largerSize = json->elementsSize ? json->elementsSize << 1 : CORE_JSON_ELEMENTS_INITIAL_SIZE;
    CFTypeRef *largerElements = CFAllocatorReallocate(json->allocator, json->elements, sizeof(CFTypeRef) * largerSize, 0);
    if (largerElements) {
      json->elementsSize = largerSize;
      json->elements = largerElements;
    }
  }
  if (json->elementsIndex < json->elementsSize)
    json->elements[json->elementsIndex++] = CFRetain(value);
  return index;
}

#pragma Memory allocation

inline void *__JSONAllocatorAllocate(void *ctx, size_t sz) {
  return CFAllocatorAllocate(ctx, sz, 0);
}

inline void __JSONAllocatorDeallocate(void *ctx, void *ptr) {
  CFAllocatorDeallocate(ctx, ptr);
}

inline void *__JSONAllocatorReallocate(void *ctx, void *ptr, size_t sz) {
  return CFAllocatorReallocate(ctx, ptr, sz, 0);
}

inline __JSONRef __JSONCreate(CFAllocatorRef allocator, JSONReadOptions options) {
  __JSONRef json = CFAllocatorAllocate(allocator, sizeof(__JSON), 0);
  if (json) {
    json->allocator = allocator ? CFRetain(allocator) : NULL;
    json->retainCount = 1;
    
    json->yajlAllocFuncs.ctx     = (void *)json->allocator;
    json->yajlAllocFuncs.malloc  = __JSONAllocatorAllocate;
    json->yajlAllocFuncs.realloc = __JSONAllocatorReallocate;
    json->yajlAllocFuncs.free    = __JSONAllocatorDeallocate;
    
    json->yajlParserCallbacks.yajl_null        = __JSONParserAppendNull;
    json->yajlParserCallbacks.yajl_boolean     = __JSONParserAppendBooleanWithInteger;
    
    // Set number or integer and double. Never all 3.
    json->yajlParserCallbacks.yajl_number      = __JSONParserAppendNumberWithBytes;
    json->yajlParserCallbacks.yajl_integer     = NULL; // __JSONParserAppendNumberWithLong
    json->yajlParserCallbacks.yajl_double      = NULL; // __JSONParserAppendNumberWithDouble
    
    json->yajlParserCallbacks.yajl_start_map   = __JSONParserAppendMapStart;
    json->yajlParserCallbacks.yajl_map_key     = __JSONParserAppendMapKeyWithBytes;
    json->yajlParserCallbacks.yajl_end_map     = __JSONParserAppendMapEnd;
    
    json->yajlParserCallbacks.yajl_start_array = __JSONParserAppendArrayStart;
    json->yajlParserCallbacks.yajl_end_array   = __JSONParserAppendArrayEnd;
    
    json->yajlParserCallbacks.yajl_string      = __JSONParserAppendStringWithBytes;
    
    json->elementsIndex = 0;
    json->elementsSize = CORE_JSON_ELEMENTS_INITIAL_SIZE;
    if (NULL == (json->elements = CFAllocatorAllocate(json->allocator, sizeof(CFTypeRef) * json->elementsSize, 0)))
      json = __JSONRelease(json);

    if (json)
      if (NULL == (json->stack = __JSONStackCreate(json->allocator, CORE_JSON_STACK_INITIAL_SIZE)))
        json = __JSONRelease(json);
  }
  return json;
}

inline __JSONRef __JSONRelease(__JSONRef json) {
  if (json) {
    if (--json->retainCount == 0) {
      CFAllocatorRef allocator = json->allocator;
      
      if (json->elements) {
        while (--json->elementsIndex >= 0)
          CFRelease(json->elements[json->elementsIndex]);
        CFAllocatorDeallocate(allocator, json->elements);
      }
      
      if (json->stack)
        json->stack = __JSONStackRelease(json->stack);
      
      CFAllocatorDeallocate(allocator, json);
      
      if (allocator)
        CFRelease(allocator);
      
      json = NULL;
    }
  }
  return json;
}

inline bool __JSONParseWithString(__JSONRef json, CFStringRef string, CFErrorRef *error) {
  bool success = 1;
  json->yajlParser = yajl_alloc(&json->yajlParserCallbacks, &json->yajlAllocFuncs, (void *)json);
  if (json->yajlParser) {
//  yajl_config(json->yajlParser, yajl_allow_comments, kJSONReadOptionAllowComments | options ? 1 : 0);
//  yajl_config(json->yajlParser, yajl_dont_validate_strings, kJSONReadOptionCheckUTF8 | options ? 1 : 0);
  
    CFDataRef data = CFStringCreateExternalRepresentation(json->allocator, string, kCFStringEncodingUTF8, 0);
    if (data) {
      if ((json->yajlParserStatus = yajl_parse(json->yajlParser, CFDataGetBytePtr(data), CFDataGetLength(data))) != yajl_status_ok) {
        if (error) {
          success = 0;
          
          unsigned char * str = yajl_get_error(json->yajlParser, 1, CFDataGetBytePtr(data), CFDataGetLength(data));
          fprintf(stderr, "%s", (const char *) str);
          yajl_free_error(json->yajlParser, str);
          
          *error = CFErrorCreateWithUserInfoKeysAndValues(json->allocator, CFSTR("com.github.mirek.CoreJSON"), (CFIndex)json->yajlParserStatus, (const void *) { kCFErrorDescriptionKey }, (const void *) { CFSTR("Test") }, 1);
        }
        // TODO: Error stuff
        //printf("ERROR: %s\n", yajl_get_error(json->yajlParser, 1, __JSONUTF8StringGetBuffer(utf8), __JSONUTF8StringGetMaximumSize(utf8)));
      }
    
      json->yajlParserStatus = yajl_complete_parse(json->yajlParser);
      CFRelease(data);
    } else {
      success = 0;
      // TODO: data is 0
    }
    yajl_free(json->yajlParser);
    json->yajlParser = NULL;
  } else {
    
    // TODO: Couldn't allocate
    *error = CFErrorCreate(json->allocator, CFSTR("com.github.mirek.CoreJSON"), -1, NULL);
    success = 0;
  }
  
  return success;
}

inline CFTypeRef JSONCreateWithString(CFAllocatorRef allocator, CFStringRef string, JSONReadOptions options, CFErrorRef *error) {
  CFTypeRef result = NULL;
  __JSONRef json = NULL;
  if ((json = __JSONCreate(allocator, options))) {
    __JSONParseWithString(json, string, error);
    if ((result = __JSONCreateObject(json))) {
      // TODO: Set error
    }
    __JSONRelease(json);
  }
  return result;
}

inline CFTypeRef __JSONCreateObject(__JSONRef json) {
  return (json && json->elements && json->elementsIndex && *json->elements) ? CFRetain(*json->elements) : NULL;
}

#pragma Generator

inline void __JSONGeneratorAppendString(CFAllocatorRef allocator, yajl_gen *g, CFStringRef value) {
  CFDataRef data = CFStringCreateExternalRepresentation(allocator, value, kCFStringEncodingUTF8, 0);
  if (data) {
    yajl_gen_string(*g, CFDataGetBytePtr(data), CFDataGetLength(data));
    CFRelease(data);
  } else {
    // TODO: Error
  }
}

inline void __JSONGeneratorAppendDoubleTypeNumber(CFAllocatorRef allocator, yajl_gen *g, CFNumberRef value) {
  double value_ = 0.0;
  CFNumberGetValue(value, kCFNumberDoubleType, &value_);
  yajl_gen_double(*g, value_);
}

inline void __JSONGeneratorAppendLongLongTypeNumber(CFAllocatorRef allocator, yajl_gen *g, CFNumberRef value) {
  long long value_ = 0;
  CFNumberGetValue(value, kCFNumberLongLongType, &value_);
  char buffer[21]; // Maximum string length is "±9223372036854775807\0"
  int length = sprintf(buffer, "%lld", value_);
  yajl_gen_number(*g, buffer, length);
}

inline void __JSONGeneratorAppendNumber(CFAllocatorRef allocator, yajl_gen *g, CFNumberRef value) {
  if (CFNumberIsFloatType(value))
    __JSONGeneratorAppendDoubleTypeNumber(allocator, g, value);
  else
    __JSONGeneratorAppendLongLongTypeNumber(allocator, g, value);
}

inline void __JSONGeneratorAppendArray(CFAllocatorRef allocator, yajl_gen *g, CFArrayRef value) {
  yajl_gen_array_open(*g);
  CFIndex n = CFArrayGetCount(value);
  CFTypeRef *values = CFAllocatorAllocate(allocator, sizeof(CFTypeRef) * n, 0);
  CFArrayGetValues(value, CFRangeMake(0, n), values);
  for (CFIndex i = 0; i < n; i++)
    __JSONGeneratorAppendValue(allocator, g, values[i]);
  CFAllocatorDeallocate(allocator, values);
  yajl_gen_array_close(*g);
}

inline void __JSONGeneratorAppendDictionary(CFAllocatorRef allocator, yajl_gen *g, CFDictionaryRef value) {
  yajl_gen_map_open(*g);
  CFIndex n = CFDictionaryGetCount(value);
  CFTypeRef *keys = CFAllocatorAllocate(allocator, sizeof(CFTypeRef) * n, 0);
  CFTypeRef *values = CFAllocatorAllocate(allocator, sizeof(CFTypeRef) * n, 0);
  CFDictionaryGetKeysAndValues(value, keys, values);
  for (CFIndex i = 0; i < n; i++) {
    __JSONGeneratorAppendValue(allocator, g, keys[i]); // TODO: append as string
    __JSONGeneratorAppendValue(allocator, g, values[i]);
  }
  CFAllocatorDeallocate(allocator, values);
  CFAllocatorDeallocate(allocator, keys);
  yajl_gen_map_close(*g);
}

inline void __JSONGeneratorAppendAttributedString(CFAllocatorRef allocator, yajl_gen *g, CFAttributedStringRef value) {
  __JSONGeneratorAppendString(allocator, g, CFAttributedStringGetString(value));
}

inline void __JSONGeneratorAppendBoolean(CFAllocatorRef allocator, yajl_gen *g, CFBooleanRef value) {
  yajl_gen_bool(*g, CFBooleanGetValue(value));
}

inline void __JSONGeneratorAppendNull(CFAllocatorRef allocator, yajl_gen *g, CFNullRef value) {
  yajl_gen_null(*g);
}

inline void __JSONGeneratorAppendURL(CFAllocatorRef allocator, yajl_gen *g, CFURLRef value) {
  __JSONGeneratorAppendString(allocator, g, CFURLGetString(value));
}

inline void __JSONGeneratorAppendUUID(CFAllocatorRef allocator, yajl_gen *g, CFUUIDRef value) {
  CFStringRef string = CFUUIDCreateString(allocator, value);
  __JSONGeneratorAppendString(allocator, g, string);
  CFRelease(string);
}

inline void __JSONGeneratorAppendValue(CFAllocatorRef allocator, yajl_gen *g, CFTypeRef value) {
  if (value) {
    CFTypeID typeID = CFGetTypeID(value);
         if (typeID == CFStringGetTypeID())           __JSONGeneratorAppendString           (allocator, g, value);
    else if (typeID == CFNumberGetTypeID())           __JSONGeneratorAppendNumber           (allocator, g, value);
    else if (typeID == CFArrayGetTypeID())            __JSONGeneratorAppendArray            (allocator, g, value);
    else if (typeID == CFDictionaryGetTypeID())       __JSONGeneratorAppendDictionary       (allocator, g, value);
    else if (typeID == CFAttributedStringGetTypeID()) __JSONGeneratorAppendAttributedString (allocator, g, value);
//    else if (typeID == CFBagGetTypeID())              __JSONGeneratorAppendBag              (allocator, g, value);
//    else if (typeID == CFBinaryHeapGetTypeID())       __JSONGeneratorAppendBinaryHeap       (allocator, g, value);
//    else if (typeID == CFBitVectorGetTypeID())        __JSONGeneratorAppendBitVector        (allocator, g, value);
    else if (typeID == CFBooleanGetTypeID())          __JSONGeneratorAppendBoolean          (allocator, g, value);
//    else if (typeID == CFDataGetTypeID())             __JSONGeneratorAppendData             (allocator, g, value);
//    else if (typeID == CFDateGetTypeID())             __JSONGeneratorAppendDate             (allocator, g, value);
    else if (typeID == CFNullGetTypeID())             __JSONGeneratorAppendNull             (allocator, g, value);
//    else if (typeID == CFSetGetTypeID())              __JSONGeneratorAppendSet              (allocator, g, value);
//    else if (typeID == CFTreeGetTypeID())             __JSONGeneratorAppendTree             (allocator, g, value);
    else if (typeID == CFURLGetTypeID())              __JSONGeneratorAppendURL              (allocator, g, value);
    else if (typeID == CFUUIDGetTypeID())             __JSONGeneratorAppendUUID             (allocator, g, value);
  }
}

inline CFStringRef JSONCreateString(CFAllocatorRef allocator, CFTypeRef value, JSONWriteOptions options, CFErrorRef *error) {
  yajl_alloc_funcs yajlAllocFuncs;
  yajlAllocFuncs.ctx = (void *)allocator;
  yajlAllocFuncs.malloc = __JSONAllocatorAllocate;
  yajlAllocFuncs.free = __JSONAllocatorDeallocate;
  yajlAllocFuncs.realloc = __JSONAllocatorReallocate;
//  yajl_gen_config yajlGenConfig;
//  yajlGenConfig.beautify     = options | kJSONWriteOptionIndent ? 1 : 0;
//  yajlGenConfig.indentString = options | kJSONWriteOptionIndent ? "  " : 0;
  yajl_gen yajlGen = yajl_gen_alloc(&yajlAllocFuncs);
  __JSONGeneratorAppendValue(allocator, &yajlGen, value);
  const unsigned char *buffer = NULL;
  size_t length = 0;
  switch (yajl_gen_get_buf(yajlGen, &buffer, &length)) {
    case yajl_gen_status_ok: // no error
      break;

    case yajl_gen_keys_must_be_strings: // at a point where a map key is generated, a function other than yajl_gen_string was called 
      break;
      
    case yajl_max_depth_exceeded: //  YAJL's maximum generation depth was exceeded. See YAJL_MAX_DEPTH
      break;
      
    case yajl_gen_in_error_state: // A generator function (yajl_gen_XXX) was called while in an error state
      break;
      
    case yajl_gen_generation_complete: // A complete JSON document has been generated
      break;
      
    case yajl_gen_invalid_number: // yajl_gen_double was passed an invalid floating point value (infinity or NaN)
      break;
      
    case yajl_gen_invalid_string: // returned from yajl_gen_string() when the yajl_gen_validate_utf8 option is enabled and an invalid was passed by client code.
      break;
      
    case yajl_gen_no_buf: // A print callback was passed in, so there is no internal buffer to get from
      break;
  }
  
  // TODO: If buffer is not null and length > 0, otherwise empty string.
  CFStringRef string = CFStringCreateWithBytes(allocator, buffer, length, kCFStringEncodingUTF8, 0);
  
  yajl_gen_clear(yajlGen);
  yajl_gen_free(yajlGen);
  
  return string;
}
