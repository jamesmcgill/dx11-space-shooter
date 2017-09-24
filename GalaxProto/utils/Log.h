#pragma once
//------------------------------------------------------------------------------
//#include "pch.h"
#include <windows.h>		// OutputDebugStringA

//------------------------------------------------------------------------------
// Very Basic Logging
// Currently outputs to Visual Studio Debugger (Which is slow!)
//
//------------------------------------------------------------------------------
// These macros must be enabled locally (on each and every file)
//  e.g. TRACE requires ENABLE_TRACE be defined before including this file.
//  However TRACE_GLOBAL_OVERRIDE will force activation of all TRACE macros.
//
// TRACE            : requires ENABLE_TRACE or TRACE_GLOBAL_OVERRIDE be defined
// LOG_LOCAL        : requires ENABLE_LOCAL or LOCAL_GLOBAL_OVERRIDE be defined
//
//------------------------------------------------------------------------------
// Logs filtered based on the logging level:
//
// LOG_VERBOSE
// LOG_VERBOSE_IF
// LOG_DEBUG
// LOG_DEBUG_IF
// LOG_INFO
// LOG_INFO_IF
// LOG_WARNING
// LOG_WARNING_IF
// LOG_ERROR
// LOG_ERROR_IF
// LOG_FATAL_ERROR
// LOG_FATAL_ERROR_IF
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Logging levels
//------------------------------------------------------------------------------
#define __LOG_LEVEL_VERBOSE 0				 // Show all logs
#define __LOG_LEVEL_DEBUG 1					 // Show debug, info, warnings and errors
#define __LOG_LEVEL_INFO 2					 // Show info, warnings and all errors
#define __LOG_LEVEL_WARNING 3				 // Show warnings and all errors
#define __LOG_LEVEL_ERROR 4					 // Show all errors
#define __LOG_LEVEL_FATAL_ERROR 5		 // Show only fatal errors
#define __LOG_LEVEL_DISABLED 6			 // Don't show any logs

// Set this to desired level
#define LOG_LEVEL __LOG_LEVEL_INFO

//------------------------------------------------------------------------------
// Locally override logging level
//------------------------------------------------------------------------------
#if defined(LOG_LEVEL_VERBOSE)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_VERBOSE
#elif defined(LOG_LEVEL_DEBUG)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_DEBUG
#elif defined(LOG_LEVEL_INFO)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_INFO
#elif defined(LOG_LEVEL_WARNING)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_WARNING
#elif defined(LOG_LEVEL_ERROR)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_ERROR
#elif defined(LOG_LEVEL_FATAL_ERROR)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_FATAL_ERROR
#elif defined(LOG_LEVEL_DISABLED)
#undef LOG_LEVEL
#define LOG_LEVEL __LOG_LEVEL_DISABLED
#endif

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// INTERNAL IMPLEMENTATION
//------------------------------------------------------------------------------
template <typename... Args>
static void
__logMsgImp__(
	const char* level,
	const char* fmt,
	const char* file,
	const int line,
	const char* function,
	Args... args)
{
	static const size_t BUFFER_SIZE = 512;

	char buffer[BUFFER_SIZE];
	auto count = sprintf_s(
		buffer, BUFFER_SIZE, "%s: %s() [%s:%d]: ", level, function, file, line);
	sprintf_s(buffer + count, BUFFER_SIZE - count, fmt, args...);

	OutputDebugStringA(buffer);
}

// clang-format off

//------------------------------------------------------------------------------
// Begining of actual implementation to all logging macros
// Extract file and line number and pass them along with the message
//------------------------------------------------------------------------------
#define __LOG_MESSAGE_IMPL__(level, fmt, ...) do{                         \
	__logMsgImp__(level, fmt, __FILE__, __LINE__, __func__, __VA_ARGS__);   \
}while(false)

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// PUBLIC INTERFACE
//------------------------------------------------------------------------------
#undef TRACE
#if defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE)
#define TRACE() do{                                \
__LOG_MESSAGE_IMPL__("TRACE", "\n");               \
}while(false)
#else
#define TRACE(fmt, ...) do{}while(false)
#endif

#undef LOG_LOCAL
#if defined(LOCAL_GLOBAL_OVERRIDE) || defined(ENABLE_LOCAL)
#define LOG_LOCAL(fmt, ...) do{                    \
__LOG_MESSAGE_IMPL__("LOG", fmt, __VA_ARGS__);     \
}while(false)
#else
#define LOG_LOCAL(fmt, ...) do{}while(false)
#endif


//------------------------------------------------------------------------------
#undef LOG_VERBOSE
#if LOG_LEVEL <= __LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(fmt, ...) do{                  \
__LOG_MESSAGE_IMPL__("VERBOSE", fmt, __VA_ARGS__); \
}while(false)
#else
#define LOG_VERBOSE(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_VERBOSE_IF
#if LOG_LEVEL <= __LOG_LEVEL_VERBOSE
#define LOG_VERBOSE_IF(cond, fmt, ...) do{         \
if (cond) {                                        \
__LOG_MESSAGE_IMPL__("VERBOSE", fmt, __VA_ARGS__); \
}                                                  \
}while(false)
#else
#define LOG_VERBOSE_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_DEBUG
#if LOG_LEVEL <= __LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) do{                    \
__LOG_MESSAGE_IMPL__("DEBUG", fmt, __VA_ARGS__);   \
}while(false)
#else
#define LOG_DEBUG(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_DEBUG_IF
#if LOG_LEVEL <= __LOG_LEVEL_DEBUG
#define LOG_DEBUG_IF(cond, fmt, ...) do{           \
if (cond) {                                        \
__LOG_MESSAGE_IMPL__("DEBUG", fmt, __VA_ARGS__);   \
}                                                  \
}while(false)
#else
#define LOG_DEBUG_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_INFO
#if LOG_LEVEL <= __LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) do{                     \
__LOG_MESSAGE_IMPL__("INFO", fmt, __VA_ARGS__);    \
}while(false)
#else
#define LOG_INFO(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_INFO_IF
#if LOG_LEVEL <= __LOG_LEVEL_INFO
#define LOG_INFO_IF(cond, fmt, ...) do{            \
if (cond) {                                        \
__LOG_MESSAGE_IMPL__("INFO", fmt, __VA_ARGS__);    \
}                                                  \
}while(false)
#else
#define LOG_INFO_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_WARNING
#if LOG_LEVEL <= __LOG_LEVEL_WARNING
#define LOG_WARNING(fmt, ...) do{                  \
__LOG_MESSAGE_IMPL__("WARNING", fmt, __VA_ARGS__); \
}while(false)
#else
#define LOG_WARNING(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_WARNING_IF
#if LOG_LEVEL <= __LOG_LEVEL_WARNING
#define LOG_WARNING_IF(cond, fmt, ...) do{         \
if (cond) {                                        \
__LOG_MESSAGE_IMPL__("WARNING", fmt, __VA_ARGS__); \
}                                                  \
}while(false)
#else
#define LOG_WARNING_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_ERROR
#if LOG_LEVEL <= __LOG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) do{                    \
__LOG_MESSAGE_IMPL__("ERROR", fmt, __VA_ARGS__);   \
}while(false)
#else
#define LOG_ERROR(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_ERROR_IF
#if LOG_LEVEL <= __LOG_LEVEL_ERROR
#define LOG_ERROR_IF(cond, fmt, ...) do{           \
if (cond) {                                        \
__LOG_MESSAGE_IMPL__("ERROR", fmt, __VA_ARGS__);   \
}                                                  \
}while(false)
#else
#define LOG_ERROR_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_FATAL_ERROR
#if LOG_LEVEL <= __LOG_LEVEL_FATAL_ERROR
#define LOG_FATAL_ERROR(fmt, ...) do{              \
__LOG_MESSAGE_IMPL__("FATAL", fmt, __VA_ARGS__);   \
}while(false)
#else
#define LOG_FATAL_ERROR(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_FATAL_ERROR_IF
#if LOG_LEVEL <= __LOG_LEVEL_FATAL_ERROR
#define LOG_FATAL_ERROR_IF(cond, fmt, ...) do{     \
if (cond) {                                        \
__LOG_MESSAGE_IMPL__("FATAL", fmt, __VA_ARGS__);   \
}                                                  \
}while(false)
#else
#define LOG_FATAL_ERROR_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
// Ensure these are switched off for the next file
//------------------------------------------------------------------------------
#undef ENABLE_TRACE
#undef ENABLE_LOCAL

//------------------------------------------------------------------------------
// clang-format on
