//
// Created by mitchell on 4/21/2015.
//

#ifndef GROUNDGAME_CLIENT_CJPLATFORMCOMPATIBILITY_H
#define GROUNDGAME_CLIENT_CJPLATFORMCOMPATIBILITY_H

#ifdef __MINGW32__
#include <stdlib.h>
#include <stdio.h>
#include <CFExtension/CFStringAdditions.h>

static inline int backtrace(void** buffer, int size) { return 0; }
static inline char** backtrace_symbols(void* const buffer, int size) { return calloc(1, sizeof(char)); }

static inline void _P(CFStringRef str)
{
	CFStringRef string = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@"), str);
	char * s = CFStringCopyCString(string);
	printf("%s\n", s);
	fflush(stdout);
	free(s);
}

// CFShow doesn't behave the same on Windows
#define CFShow _P
#endif

#ifdef __APPLE__
#endif

#ifdef __LINUX__
#endif

#endif //GROUNDGAME_CLIENT_CJPLATFORMCOMPATIBILITY_H
