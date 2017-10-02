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
	static const size_t BUFFER_SIZE = 512;

	char buffer[BUFFER_SIZE];
	auto count = sprintf_s(
		buffer,
		BUFFER_SIZE,
		"%7s: [%30s:%4d] %20s(): ",
		level,
		file,
		line,
		function);
	sprintf_s(buffer + count, BUFFER_SIZE - count, fmt, args...);

	OutputDebugStringA(buffer);
}

//------------------------------------------------------------------------------
struct TimedRecord
{
	uint64_t totalTicks;

	size_t hashIndex;
	int32_t lineNumber;
	const char* file;
	const char* function;

	std::vector<TimedRecord*> childNodes;

	//----------------------------------------------------------------------------
	static TimedRecord*& getHeadNode()
	{
		static TimedRecord* head = nullptr;
		return head;
	}
};

//------------------------------------------------------------------------------
template <typename T> struct AggregateValue
{
	T total				= 0;
	T min					= std::numeric_limits<T>::max();
	T max					= std::numeric_limits<T>::lowest();
	int32_t count = 0;

	void accumulate(T newValue)
	{
		total += newValue;
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
		return total / count;
	}
};

//------------------------------------------------------------------------------
struct AggregateRecord
{
	AggregateValue<uint64_t> ticks;
	AggregateValue<int32_t> callsCount;
	AggregateValue<uint64_t> ticksPerCount;

	int32_t lineNumber;
	const char* file;
	const char* function;
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Timing
{
	// Integer format represents time using 10,000,000 ticks per second.
	static const uint64_t TICKS_PER_SECOND			= 10'000'000;
	static const uint64_t TICKS_PER_MILLISECOND = 10'000;

	//------------------------------------------------------------------------------
	static uint64_t getCurrentTimeInTicks()
	{
		LARGE_INTEGER currentTime = {0LL};
		if (!QueryPerformanceCounter(&currentTime))
		{
			throw std::exception("QueryPerformanceCounter");
		}
		return currentTime.QuadPart;
	}

	//------------------------------------------------------------------------------
	static uint64_t initFrequency()
	{
		LARGE_INTEGER frequency;
		if (!QueryPerformanceFrequency(&frequency))
		{
			throw std::exception("QueryPerformanceFrequency");
		}
		return frequency.QuadPart;
	}

	//------------------------------------------------------------------------------
	static uint64_t& getQpcFrequency()
	{
		static uint64_t qpcFrequency = initFrequency();
		return qpcFrequency;
	}

	//------------------------------------------------------------------------------
	static uint64_t initMaxDelta()
	{
		// Initialize max delta to 1/10 of a second.
		return getQpcFrequency() / 10;
	}

	//------------------------------------------------------------------------------
	static uint64_t& getQpcMaxDelta()
	{
		static uint64_t qpcMaxDelta = initMaxDelta();
		return qpcMaxDelta;
	}

	//------------------------------------------------------------------------------
	static uint64_t
	getClampedDuration(const uint64_t tEarliest, const uint64_t tLatest)
	{
		uint64_t timeDelta = tLatest - tEarliest;

		// Clamp excessively large time deltas (e.g. after paused in the
		// debugger).
		if (timeDelta > getQpcMaxDelta())
		{
			timeDelta = getQpcMaxDelta();
		}
		return timeDelta;
	}

	//------------------------------------------------------------------------------
	static double ticksToMilliSeconds(uint64_t ticks)
	{
		// Convert QPC units into a canonical tick format. This cannot overflow
		// due to the previous clamp.
		ticks *= TICKS_PER_SECOND;
		ticks /= getQpcFrequency();

		return static_cast<double>(ticks) / TICKS_PER_MILLISECOND;
	}
};		// struct Timing

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Stats
{
	static const int SNAPSHOT_COUNT		= 120;
	static const int MAX_RECORD_COUNT = 120;

	using Records = std::array<TimedRecord, MAX_RECORD_COUNT>;
	struct SnapShot
	{
		Records records;
		size_t numRecords = 0;
	};

	using AllSnapShots			= std::array<SnapShot, SNAPSHOT_COUNT>;
	using AggregateSnapShot = std::unordered_map<size_t, AggregateRecord>;

	//------------------------------------------------------------------------------
	static AllSnapShots& getAllSnapShots()
	{
		static AllSnapShots snapShots = AllSnapShots();
		return snapShots;
	}

	//------------------------------------------------------------------------------
	static SnapShot& getSnapShot(const int snapShotIdx)
	{
		return getAllSnapShots()[snapShotIdx];
	}

	//------------------------------------------------------------------------------
	static int& getCurrentSnapShotIdx()
	{
		static int idx = 0;
		return idx;
	}

	//------------------------------------------------------------------------------
	static int& incrementSnapShotIdx()
	{
		int& idx = getCurrentSnapShotIdx();
		if (++idx >= SNAPSHOT_COUNT)
		{
			idx = 0;
		}
		return idx;
	}

	//----------------------------------------------------------------------------
	static void clearSnapShot(SnapShot& snapShot)
	{
		Records temp;
		snapShot.records.swap(temp);
		snapShot.numRecords = 0;
	}

	//----------------------------------------------------------------------------
	static void signalFrameEnd()
	{
		int newIdx = incrementSnapShotIdx();
		clearSnapShot(getSnapShot(newIdx));
	}

	//----------------------------------------------------------------------------
	static AggregateSnapShot aggregateData()
	{
		AggregateSnapShot aggregatedSnapShots;
		auto& allSnapShots = getAllSnapShots();
		for (auto& snapShot : allSnapShots)
		{
			// Aggregate identical calls within ONE frame (i.e. same hash)
			// E.g. 3 calls to Enemy::update() become one entry with total time
			AggregateSnapShot aggregatedFrame;
			for (size_t i = 0; i < snapShot.numRecords; ++i)
			{
				auto& sourceRecord = snapShot.records[i];
				auto& record			 = aggregatedFrame[sourceRecord.hashIndex];
				record.ticks.total += sourceRecord.totalTicks;
				record.callsCount.total++;

				record.function		= sourceRecord.function;
				record.file				= sourceRecord.file;
				record.lineNumber = sourceRecord.lineNumber;
			}

			// Aggregate identical calls over SEVERAL frames, to get averages,
			// min, max etc.
			for (auto& r : aggregatedFrame)
			{
				auto& record			 = aggregatedSnapShots[r.first];
				auto& sourceRecord = r.second;

				record.ticks.accumulate(sourceRecord.ticks.total);
				record.callsCount.accumulate(sourceRecord.callsCount.total);
				record.ticksPerCount.accumulate(
					sourceRecord.ticks.total / sourceRecord.callsCount.total);

				record.function		= sourceRecord.function;
				record.file				= sourceRecord.file;
				record.lineNumber = sourceRecord.lineNumber;
			}
		}

		return aggregatedSnapShots;
	}

};		// struct Stats

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct TimedRaiiBlock
{
	const TimedRaiiBlock* _parent = nullptr;
	const uint64_t _qpcStartTicks;
	TimedRecord* _record = nullptr;

	//----------------------------------------------------------------------------
	TimedRaiiBlock(
		const size_t hashIndex,
		const int line,
		const char* file,
		const char* function)
			: _parent(getCurrentBlock())
			, _qpcStartTicks(Timing::getCurrentTimeInTicks())
	{
		TimedRaiiBlock* current = getCurrentBlock();
		current									= this;

		auto& currentSnapShot = Stats::getSnapShot(Stats::getCurrentSnapShotIdx());
		auto recordIndex			= currentSnapShot.numRecords;

		if (currentSnapShot.numRecords < Stats::MAX_RECORD_COUNT - 1)
		{
			currentSnapShot.numRecords++;
		}
		else
		{
			logMsgImp(
				"ERROR",
				"MAX_RECORD_COUNT exceeded. Increase Value\n",
				__FILE__,
				__LINE__,
				__func__);
		}

		TimedRecord& record = currentSnapShot.records[recordIndex];
		_record							= &record;
		_record->hashIndex	= hashIndex;
		_record->lineNumber = line;
		_record->file				= file;
		_record->function		= function;

		if (_parent)
		{
			_parent->_record->childNodes.push_back(_record);
		}
		else
		{
			auto& head = TimedRecord::getHeadNode();
			head			 = _record;
		}
	}

	//----------------------------------------------------------------------------
	~TimedRaiiBlock()
	{
		uint64_t elapsedTicks = Timing::getClampedDuration(
			_qpcStartTicks, Timing::getCurrentTimeInTicks());

		_record->totalTicks = elapsedTicks;

		// TEST message
		double elapsedTimeMs = Timing::ticksToMilliSeconds(elapsedTicks);
		logMsgImp(
			"TIMED",
			"%9.6fms - hash %ull\n",
			_record->file,
			_record->lineNumber,
			_record->function,
			elapsedTimeMs,
			_record->hashIndex);

		TimedRaiiBlock* current = getCurrentBlock();
		current									= const_cast<TimedRaiiBlock*>(_parent);
	}

	//----------------------------------------------------------------------------
	static TimedRaiiBlock* getCurrentBlock()
	{
		static TimedRaiiBlock* current = nullptr;
		return current;
	}

};		// TimedRaiiBlock

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
static size_t
createTimedRecordHash(const std::string_view& filePath, const int lineNumber)
{
	// Don't bother hashing the full path. Only the last few characters
	// will differ enough to be useful for hashing.
	const size_t NUM_CHARS = 12;
	const size_t subPos
		= (filePath.length() > NUM_CHARS) ? filePath.length() - NUM_CHARS : 0;
	const std::string_view file = filePath.substr(subPos);
	const std::size_t h1				= std::hash<std::string_view>{}(file);
	const std::size_t h2				= std::hash<int>{}(lineNumber);
	return h1 ^ (h2 << 1);
}

//------------------------------------------------------------------------------
// clang-format off


//------------------------------------------------------------------------------
// Begining of actual implementation to all logging macros
// Extract file and line number and pass them along with the message
//------------------------------------------------------------------------------
#define LOG_MESSAGE_IMPL(level, fmt, ...) do{                                  \
	logger::logMsgImp(level, fmt, __FILE__, __LINE__, __func__, __VA_ARGS__);    \
}while(false)

//------------------------------------------------------------------------------
}; // namespace logger


//------------------------------------------------------------------------------
// PUBLIC INTERFACE
//------------------------------------------------------------------------------
#undef TIMED_TRACE
#define TIMED_TRACE                                                            \
	const std::string_view& filePath = __FILE__;                                 \
	const size_t hashIndex = logger::createTimedRecordHash(filePath, __LINE__);  \
	logger::TimedRaiiBlock timedBlock_##__COUNTER__(                             \
			hashIndex, __LINE__, __FILE__, __func__);

//------------------------------------------------------------------------------
#undef TRACE
#if (defined(ENABLE_TIMED_TRACE))                                              \
 && (defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE TIMED_TRACE                                                      \
do{                                                                            \
LOG_MESSAGE_IMPL("TRACE", "\n");                                               \
}while(false);

#elif (defined(ENABLE_TIMED_TRACE))                                            \
 && !(defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE TIMED_TRACE

#elif (!defined(ENABLE_TIMED_TRACE))                                           \
 && (defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE do{                                                              \
LOG_MESSAGE_IMPL("TRACE", "\n");                                               \
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
