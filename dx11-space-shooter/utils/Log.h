#pragma once
//------------------------------------------------------------------------------
#include "pch.h"
//#include <windows.h>			// OutputDebugStringA
//#include <unordered_map>

namespace logger
{
//------------------------------------------------------------------------------
// Very Basic Logging and profiler
// Currently outputs to Visual Studio Debugger (Which is slow!)
//
// Profiling is NOT thread-safe
//
//------------------------------------------------------------------------------
// TRACE
// Performs 2 functions (which can be enabled independently):
//
// 1) if ENABLE_TRACE_LOG is defined prior to including this header file then
//			calling TRACE will output the current function call to the log output.
//			(ENABLE_TRACE_LOG must be defined for each file. However
//			TRACE_GLOBAL_OVERRIDE will force activte all TRACE macros everywhere.)
//
// 2) if ENABLE_TIMED_TRACE is defined (below) then every call to TRACE
//			anywhere in the program will log the duration until the end of
//			the enclosing function call. This happens REGARDLESS of
//			whether ENABLE_TRACE_LOG was set.
//
//------------------------------------------------------------------------------
// Logs filtered based on the global logging level:
// (logging level may be overridden per file e.g. with LOG_LEVEL_VERBOSE)
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
#define LOG_LEVEL_VERBOSE 0				 // Show all logs
#define LOG_LEVEL_DEBUG 1					 // Show debug, info, warnings and errors
#define LOG_LEVEL_INFO 2					 // Show info, warnings and all errors
#define LOG_LEVEL_WARNING 3				 // Show warnings and all errors
#define LOG_LEVEL_ERROR 4					 // Show all errors
#define LOG_LEVEL_FATAL_ERROR 5		 // Show only fatal errors
#define LOG_LEVEL_DISABLED 6			 // Don't show any logs

// Set this to desired default level (may be overridden per file)
#define LOG_LEVEL LOG_LEVEL_INFO

// Comment out to disable profiling (via TRACE)
#define ENABLE_TIMED_TRACE

//------------------------------------------------------------------------------
// Locally override logging level
//------------------------------------------------------------------------------
#if defined(LOG_LEVEL_VERBOSE)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#elif defined(LOG_LEVEL_DEBUG)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#elif defined(LOG_LEVEL_INFO)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#elif defined(LOG_LEVEL_WARNING)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_WARNING
#elif defined(LOG_LEVEL_ERROR)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_ERROR
#elif defined(LOG_LEVEL_FATAL_ERROR)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_FATAL_ERROR
#elif defined(LOG_LEVEL_DISABLED)
#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DISABLED
#endif

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// INTERNAL IMPLEMENTATION
//------------------------------------------------------------------------------
template <typename... Args>
static void
logMsgImp(
	const char* level,
	const char* fmt,
	const char* file,
	const int line,
	const char* function,
	Args... args)
{
	static const size_t BUFFER_SIZE = 12 * 1024;
	char buffer[BUFFER_SIZE];

	// Add prefix (line number, file etc)
	int count = sprintf_s(
		buffer, BUFFER_SIZE, "%s: [%s:%d] %s(): ", level, file, line, function);
	ASSERT(count >= 0);

	// Add the message
	int startIdx = (count >= 0) ? count : 0;
	if (startIdx >= BUFFER_SIZE)
	{
		ASSERT(false);
		return;
	}
	count = sprintf_s(buffer + startIdx, BUFFER_SIZE - startIdx, fmt, args...);
	ASSERT(count >= 0);

	// Add newline
	startIdx = (count >= 0) ? startIdx + count : startIdx;
	if (startIdx >= BUFFER_SIZE)
	{
		ASSERT(false);
		return;
	}
	count = sprintf_s(buffer + startIdx, BUFFER_SIZE - startIdx, "\n");
	ASSERT(count >= 0);

	OutputDebugStringA(buffer);
}

//------------------------------------------------------------------------------
using Ticks = uint64_t;

//------------------------------------------------------------------------------
struct TimedRecord
{
	Ticks startTime;
	Ticks duration;

	int32_t lineNumber;
	const char* file;
	const char* function;

	std::vector<TimedRecord*> childNodes;
};

//------------------------------------------------------------------------------
template <typename T> struct AccumulatedValue
{
	T sum					= 0;
	T min					= std::numeric_limits<T>::max();
	T max					= std::numeric_limits<T>::lowest();
	int32_t count = 0;

	void accumulate(T newValue)
	{
		sum += newValue;
		if (newValue < min)
		{
			min = newValue;
		}
		if (newValue > max)
		{
			max = newValue;
		}
		count++;
	}

	T average() const
	{
		if (!count)
		{
			return 0;
		}

		return sum / count;
	}
};

//------------------------------------------------------------------------------
struct AccumulatedRecord
{
	AccumulatedValue<Ticks> ticks;
	AccumulatedValue<int32_t> callsCount;
	AccumulatedValue<Ticks> ticksPerCount;
};

//------------------------------------------------------------------------------
struct CollatedRecord
{
	Ticks ticks				 = 0;
	int32_t callsCount = 0;

	int32_t lineNumber;
	const char* file;
	const char* function;
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Timing
{
	// Integer format represents time using 10,000,000 ticks per second.
	static const Ticks TICKS_PER_SECOND			 = 10'000'000;
	static const Ticks TICKS_PER_MILLISECOND = 10'000;

	//----------------------------------------------------------------------------
	static Ticks getCurrentTimeInTicks();
	static Ticks initFrequency();
	static Ticks& getQpcFrequency();
	static Ticks initMaxDelta();
	static Ticks& getQpcMaxDelta();
	static Ticks getClampedDuration(const Ticks tEarliest, const Ticks tLatest);
	static double ticksToMilliSeconds(Ticks ticks);
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Stats
{
	static const int FRAME_COUNT			= 120;
	static const int MAX_RECORD_COUNT = 120;

	using TimedRecordArray = std::array<TimedRecord, MAX_RECORD_COUNT>;
	struct FrameRecords
	{
		size_t numRecords					 = 0;
		TimedRecord* callGraphHead = nullptr;
		TimedRecordArray records;
	};
	using IntervalRecords = std::array<FrameRecords, FRAME_COUNT>;

	using CollatedFrameRecords		= std::unordered_map<size_t, CollatedRecord>;
	using CollatedIntervalRecords = std::array<CollatedFrameRecords, FRAME_COUNT>;

	using AccumulatedRecords = std::unordered_map<size_t, AccumulatedRecord>;

	//----------------------------------------------------------------------------
	static IntervalRecords& getIntervalRecords();
	static FrameRecords& getFrameRecords(const int frameIdx);

	static CollatedIntervalRecords& getCollatedIntervalRecords();
	static CollatedFrameRecords& getCollatedFrameRecords(const int frameIdx);

	static int& getCurrentFrameIdx();
	static int& incrementCurrentFramedIdx();

	static void clearFrame(FrameRecords& frame);
	static void clearCollatedFrame(CollatedFrameRecords& frame);

	static void condenseFrameRecords(int frameIdx);
	static void signalFrameEnd();

	static AccumulatedRecords accumulateRecords();
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct TimedRaiiBlock
{
	const TimedRaiiBlock* _parent = nullptr;
	TimedRecord* _record					= nullptr;

	//----------------------------------------------------------------------------
	TimedRaiiBlock(const int line, const char* file, const char* function);

	~TimedRaiiBlock();

	static TimedRaiiBlock*& getCurrentOpenBlockByRef();
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
size_t
createTimedRecordHash(const std::string_view& filePath, const int lineNumber);

//------------------------------------------------------------------------------
// clang-format off


//------------------------------------------------------------------------------
// Begining of actual implementation to all logging macros
// Extract file and line number and pass them along with the message
//------------------------------------------------------------------------------
#define LOG_MESSAGE_IMPL(level, fmt, ...) do{                                  \
	logger::logMsgImp(level, fmt, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);\
}while(false)

//------------------------------------------------------------------------------
}; // namespace logger


//------------------------------------------------------------------------------
// PUBLIC INTERFACE
//------------------------------------------------------------------------------
#define CONCAT_IMPL(x, y) x ## y
#define CAT(x, y) CONCAT_IMPL(x, y)

#undef TIMED_TRACE_IMPL
#define TIMED_TRACE_IMPL(N)                                                    \
	logger::TimedRaiiBlock CAT(timedBlock_,N)(__LINE__, __FILE__, __FUNCTION__);

#undef TIMED_TRACE
#define TIMED_TRACE TIMED_TRACE_IMPL(__COUNTER__);

//------------------------------------------------------------------------------
#undef TRACE
#if (defined(ENABLE_TIMED_TRACE))                                              \
 && (defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE TIMED_TRACE                                                      \
do{                                                                            \
LOG_MESSAGE_IMPL("TRACE", "");                                                 \
}while(false);

#elif (defined(ENABLE_TIMED_TRACE))                                            \
 && !(defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE TIMED_TRACE

#elif (!defined(ENABLE_TIMED_TRACE))                                           \
 && (defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE do{                                                              \
LOG_MESSAGE_IMPL("TRACE", "");                                                 \
}while(false);

#else
#define TRACE do{}while(false);
#endif

//------------------------------------------------------------------------------
#undef LOG_VERBOSE
#if LOG_LEVEL <= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(fmt, ...) do{                                              \
LOG_MESSAGE_IMPL("VERBOSE", fmt, __VA_ARGS__);                                 \
}while(false)
#else
#define LOG_VERBOSE(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_VERBOSE_IF
#if LOG_LEVEL <= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE_IF(cond, fmt, ...) do{                                     \
if (cond) {                                                                    \
LOG_MESSAGE_IMPL("VERBOSE", fmt, __VA_ARGS__);                                 \
}                                                                              \
}while(false)
#else
#define LOG_VERBOSE_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_DEBUG
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) do{                                                \
LOG_MESSAGE_IMPL("DEBUG", fmt, __VA_ARGS__);                                   \
}while(false)
#else
#define LOG_DEBUG(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_DEBUG_IF
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG_IF(cond, fmt, ...) do{                                       \
if (cond) {                                                                    \
LOG_MESSAGE_IMPL("DEBUG", fmt, __VA_ARGS__);                                   \
}                                                                              \
}while(false)
#else
#define LOG_DEBUG_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_INFO
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) do{                                                 \
LOG_MESSAGE_IMPL("INFO", fmt, __VA_ARGS__);                                    \
}while(false)
#else
#define LOG_INFO(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_INFO_IF
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO_IF(cond, fmt, ...) do{                                        \
if (cond) {                                                                    \
LOG_MESSAGE_IMPL("INFO", fmt, __VA_ARGS__);                                    \
}                                                                              \
}while(false)
#else
#define LOG_INFO_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_WARNING
#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(fmt, ...) do{                                              \
LOG_MESSAGE_IMPL("WARNING", fmt, __VA_ARGS__);                                 \
}while(false)
#else
#define LOG_WARNING(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_WARNING_IF
#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING_IF(cond, fmt, ...) do{                                     \
if (cond) {                                                                    \
LOG_MESSAGE_IMPL("WARNING", fmt, __VA_ARGS__);                                 \
}                                                                              \
}while(false)
#else
#define LOG_WARNING_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_ERROR
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) do{                                                \
LOG_MESSAGE_IMPL("ERROR", fmt, __VA_ARGS__);                                   \
}while(false)
#else
#define LOG_ERROR(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_ERROR_IF
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR_IF(cond, fmt, ...) do{                                       \
if (cond) {                                                                    \
LOG_MESSAGE_IMPL("ERROR", fmt, __VA_ARGS__);                                   \
}                                                                              \
}while(false)
#else
#define LOG_ERROR_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_FATAL_ERROR
#if LOG_LEVEL <= LOG_LEVEL_FATAL_ERROR
#define LOG_FATAL_ERROR(fmt, ...) do{                                          \
LOG_MESSAGE_IMPL("FATAL", fmt, __VA_ARGS__);                                   \
}while(false)
#else
#define LOG_FATAL_ERROR(fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_FATAL_ERROR_IF
#if LOG_LEVEL <= LOG_LEVEL_FATAL_ERROR
#define LOG_FATAL_ERROR_IF(cond, fmt, ...) do{                                 \
if (cond) {                                                                    \
LOG_MESSAGE_IMPL("FATAL", fmt, __VA_ARGS__);                                   \
}                                                                              \
}while(false)
#else
#define LOG_FATAL_ERROR_IF(cond, fmt, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
// Ensure these are switched off for the next file
//------------------------------------------------------------------------------
#undef ENABLE_TRACE_LOG

//------------------------------------------------------------------------------
// clang-format on
