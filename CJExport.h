//
// Created by mitchell on 6/16/2015.
//

#ifndef GROUNDGAME_CLIENT_CJEXPORT_H
#define GROUNDGAME_CLIENT_CJEXPORT_H

#if TARGET_OS_WIN32 && defined(BUILD_SHARED_LIBS)

	#if !defined(CJ_EXPORT)
	#if defined(CJ_BUILDING_CJ) && defined(__cplusplus)
	#define CJ_EXPORT extern "C" __declspec(dllexport)
	#elif defined(CJ_BUILDING_CJ) && !defined(__cplusplus)
	#define CJ_EXPORT extern __declspec(dllexport)
	#elif defined(__cplusplus)
	#define CJ_EXPORT extern "C" __declspec(dllimport)
	#else
	#define CJ_EXPORT extern __declspec(dllimport)
	#endif
	#endif

#else

#ifdef __cplusplus
	#define CJ_EXPORT extern "C"
	#else
#define CJ_EXPORT extern
#endif

#endif

#endif //GROUNDGAME_CLIENT_CJEXPORT_H
