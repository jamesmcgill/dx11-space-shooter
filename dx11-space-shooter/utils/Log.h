//------------------------------------------------------------------------------
// Very Basic Logging and profiler
//
// NOT thread-safe ( see printBuffer() )
//
// Usage: (in one cpp file only and only if profiler support is required)
//
//  #define LOGGER_PROFILER_IMPLEMENTATION
//  #include "log.h"
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
#pragma once

#ifdef _WIN32
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // OutputDebugStringA, _ASSERTE
#else
#include <assert.h>
#endif

#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <array>
#include <string_view>
#include <limits>

namespace logger
{
#ifdef _WIN32
#define ASSERT _ASSERTE
#else
#define ASSERT(x) assert(x)
#endif

//------------------------------------------------------------------------------
// Logging levels
//------------------------------------------------------------------------------
#define LOG_LEVEL_VERBOSE 0        // Show all logs
#define LOG_LEVEL_DEBUG 1          // Show debug, info, warnings and errors
#define LOG_LEVEL_INFO 2           // Show info, warnings and all errors
#define LOG_LEVEL_WARNING 3        // Show warnings and all errors
#define LOG_LEVEL_ERROR 4          // Show all errors
#define LOG_LEVEL_FATAL_ERROR 5    // Show only fatal errors
#define LOG_LEVEL_DISABLED 6       // Don't show any logs

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
static const size_t BUFFER_SIZE = 12 * 1024;
static char buffer[BUFFER_SIZE];

//------------------------------------------------------------------------------
static void printBuffer()
{
  std::cout << buffer;
  #if defined(_WIN32) && defined(_DEBUG)
    OutputDebugStringA(buffer);
  #endif
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
template <typename T>
struct AccumulatedValue
{
  T sum         = 0;
  T min         = std::numeric_limits<T>::max();
  T max         = std::numeric_limits<T>::lowest();
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
  Ticks ticks        = 0;
  int32_t callsCount = 0;

  int32_t lineNumber;
  const char* file;
  const char* function;
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Timing
{
  static const Ticks MICROSECONDS_PER_SECOND = 1'000'000;
  static const Ticks MILLISECONDS_PER_SECOND = 1'000;

  //----------------------------------------------------------------------------
  static Ticks getCurrentTimeInTicks();
  static Ticks initFrequency();
  static Ticks& getQpcFrequency();
  static Ticks initMaxClampDelta();
  static Ticks& getMaxClampDelta();
  static Ticks getClampedDuration(const Ticks tEarliest, const Ticks tLatest);
  static double ticksToMilliSeconds(Ticks ticks);
};

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct Stats
{
  static const int FRAME_COUNT      = 120;
  static const int MAX_RECORD_COUNT = 120;

  using TimedRecordArray = std::array<TimedRecord, MAX_RECORD_COUNT>;
  struct FrameRecords
  {
    size_t numRecords          = 0;
    TimedRecord* callGraphHead = nullptr;
    TimedRecordArray records;
  };
  using IntervalRecords = std::array<FrameRecords, FRAME_COUNT>;

  using CollatedFrameRecords    = std::unordered_map<size_t, CollatedRecord>;
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
  TimedRecord* _record          = nullptr;

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
} // namespace logger

//------------------------------------------------------------------------------
// clang-format off

//------------------------------------------------------------------------------
#ifdef _WIN32
#define __FILENAME__ \
  (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define _CONCAT_IMPL(x, y) x ## y
#define CAT(x, y) _CONCAT_IMPL(x, y)

//------------------------------------------------------------------------------
// Extract file and line number and pass them along with the message
//
// This uses a common trick for performing overloading with macros.
// However it limits the maximum number of arguments (12 currently)
// Dispatching to a macro that doesn't use __VAR_ARGS__ when there are none
// prevents warnings when the -pedantic flag enabled.
//------------------------------------------------------------------------------
#define _LG_MSG(level, msg)                                                    \
  std::snprintf(logger::buffer, logger::BUFFER_SIZE,                           \
  "%s: [%s:%d] %s(): %s\n",                                                    \
  level, __FILENAME__, __LINE__, __FUNCTION__, msg);                           \
  logger::printBuffer();

#define _LG_MSG_FMT(level, fmt, ...)                                           \
  std::snprintf(logger::buffer, logger::BUFFER_SIZE,                           \
  "%s: [%s:%d] %s(): " fmt "\n",                                               \
  level, __FILENAME__, __LINE__, __FUNCTION__, __VA_ARGS__);                   \
  logger::printBuffer();

#define _GET_NTH( _0, _1, _2, _3, _4, _5, _6,                                  \
                  _7, _8, _9, _10, _11, _12, NAME, ...) NAME

#define MSVC_EXPAND( x ) _GET_NTH x

#define _GET_OVERLOAD(...)                                                     \
MSVC_EXPAND((__VA_ARGS__,                                                      \
_LG_MSG_FMT, _LG_MSG_FMT, _LG_MSG_FMT, _LG_MSG_FMT,                            \
_LG_MSG_FMT, _LG_MSG_FMT, _LG_MSG_FMT, _LG_MSG_FMT,                            \
_LG_MSG_FMT, _LG_MSG_FMT, _LG_MSG_FMT, _LG_MSG_FMT,                            \
_LG_MSG, ))  // NB. Intentionally trailing comma as _GET_NTH has a trailing ...

#define _LOG_MESSAGE_IMPL(level, ...)                                          \
  _GET_OVERLOAD(__VA_ARGS__)(level, __VA_ARGS__)

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PUBLIC INTERFACE
//------------------------------------------------------------------------------
#undef TIMED_TRACE_IMPL
#define TIMED_TRACE_IMPL(N)                                                    \
  logger::TimedRaiiBlock CAT(timedBlock_,N)(                                   \
    __LINE__, __FILENAME__, __FUNCTION__);

#undef TIMED_TRACE
#define TIMED_TRACE TIMED_TRACE_IMPL(__COUNTER__);

//------------------------------------------------------------------------------
#undef TRACE
#if (defined(ENABLE_TIMED_TRACE))                                              \
 && (defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE TIMED_TRACE                                                      \
do{                                                                            \
_LOG_MESSAGE_IMPL("TRACE", "");                                                \
}while(false);

#elif (defined(ENABLE_TIMED_TRACE))                                            \
 && !(defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE TIMED_TRACE

#elif (!defined(ENABLE_TIMED_TRACE))                                           \
 && (defined(TRACE_GLOBAL_OVERRIDE) || defined(ENABLE_TRACE_LOG))
#define TRACE do{                                                              \
_LOG_MESSAGE_IMPL("TRACE", "");                                                \
}while(false);

#else
#define TRACE do{}while(false);
#endif

//------------------------------------------------------------------------------
#undef LOG_VERBOSE
#if LOG_LEVEL <= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(...) do{                                                   \
_LOG_MESSAGE_IMPL("VERBOSE", __VA_ARGS__);                                     \
}while(false)
#else
#define LOG_VERBOSE(...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_VERBOSE_IF
#if LOG_LEVEL <= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE_IF(cond, ...) do{                                          \
if (cond) {                                                                    \
_LOG_MESSAGE_IMPL("VERBOSE", __VA_ARGS__);                                     \
}                                                                              \
}while(false)
#else
#define LOG_VERBOSE_IF(cond, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_DEBUG
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(...) do{                                                     \
_LOG_MESSAGE_IMPL("DEBUG", __VA_ARGS__);                                       \
}while(false)
#else
#define LOG_DEBUG(...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_DEBUG_IF
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG_IF(cond, ...) do{                                            \
if (cond) {                                                                    \
_LOG_MESSAGE_IMPL("DEBUG", __VA_ARGS__);                                       \
}                                                                              \
}while(false)
#else
#define LOG_DEBUG_IF(cond, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_INFO
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(...) do{                                                      \
_LOG_MESSAGE_IMPL("INFO", __VA_ARGS__);                                        \
}while(false)
#else
#define LOG_INFO(...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_INFO_IF
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO_IF(cond, ...) do{                                             \
if (cond) {                                                                    \
_LOG_MESSAGE_IMPL("INFO", __VA_ARGS__);                                        \
}                                                                              \
}while(false)
#else
#define LOG_INFO_IF(cond, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_WARNING
#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(...) do{                                                   \
_LOG_MESSAGE_IMPL("WARNING", __VA_ARGS__);                                     \
}while(false)
#else
#define LOG_WARNING(...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_WARNING_IF
#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING_IF(cond, ...) do{                                          \
if (cond) {                                                                    \
_LOG_MESSAGE_IMPL("WARNING", __VA_ARGS__);                                     \
}                                                                              \
}while(false)
#else
#define LOG_WARNING_IF(cond, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_ERROR
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(...) do{                                                     \
_LOG_MESSAGE_IMPL("ERROR", __VA_ARGS__);                                       \
}while(false)
#else
#define LOG_ERROR(...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_ERROR_IF
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR_IF(cond, ...) do{                                            \
if (cond) {                                                                    \
_LOG_MESSAGE_IMPL("ERROR", __VA_ARGS__);                                       \
}                                                                              \
}while(false)
#else
#define LOG_ERROR_IF(cond, ...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_FATAL_ERROR
#if LOG_LEVEL <= LOG_LEVEL_FATAL_ERROR
#define LOG_FATAL_ERROR(...) do{                                               \
_LOG_MESSAGE_IMPL("FATAL", __VA_ARGS__);                                       \
}while(false)
#else
#define LOG_FATAL_ERROR(...) do{}while(false)
#endif

//------------------------------------------------------------------------------
#undef LOG_FATAL_ERROR_IF
#if LOG_LEVEL <= LOG_LEVEL_FATAL_ERROR
#define LOG_FATAL_ERROR_IF(cond, ...) do{                                      \
if (cond) {                                                                    \
_LOG_MESSAGE_IMPL("FATAL", __VA_ARGS__);                                       \
}                                                                              \
}while(false)
#else
#define LOG_FATAL_ERROR_IF(cond, ...) do{}while(false)
#endif

// clang-format on

//------------------------------------------------------------------------------
// Ensure these are switched off for the next file
//------------------------------------------------------------------------------
#undef ENABLE_TRACE_LOG

//------------------------------------------------------------------------------
#ifdef LOGGER_PROFILER_IMPLEMENTATION

#ifndef _WIN32
#include <chrono>
#endif

//------------------------------------------------------------------------------
namespace logger
{
//------------------------------------------------------------------------------
Ticks
Timing::getCurrentTimeInTicks()
{
#ifdef _WIN32
  LARGE_INTEGER currentTime;
  if (!QueryPerformanceCounter(&currentTime))
  {
    throw std::exception("QueryPerformanceCounter");
  }
  return currentTime.QuadPart;
#else
  using namespace std::chrono;
  auto now = high_resolution_clock::now();
  return time_point_cast<nanoseconds>(now)
    .time_since_epoch()
    .count();    // nano-seconds since epoch
#endif
}

//------------------------------------------------------------------------------
Ticks
Timing::initFrequency()
{
#ifdef _WIN32
  LARGE_INTEGER frequency;
  if (!QueryPerformanceFrequency(&frequency))
  {
    throw std::exception("QueryPerformanceFrequency");
  }
  return frequency.QuadPart;
#else
  return std::nano::den;
#endif
}

//------------------------------------------------------------------------------
Ticks&
Timing::getQpcFrequency()
{
  static Ticks qpcFrequency = initFrequency();
  return qpcFrequency;
}

//------------------------------------------------------------------------------
Ticks
Timing::initMaxClampDelta()
{
  // Initialize max delta to 1/10 of a second.
  return getQpcFrequency() / 10;
}

//------------------------------------------------------------------------------
Ticks&
Timing::getMaxClampDelta()
{
  static Ticks maxClampDelta = initMaxClampDelta();
  return maxClampDelta;
}

//------------------------------------------------------------------------------
Ticks
Timing::getClampedDuration(const Ticks tEarliest, const Ticks tLatest)
{
  Ticks timeDelta = tLatest - tEarliest;

  // Clamp excessively large time deltas (e.g. after paused in the debugger).
  if (timeDelta > getMaxClampDelta())
  {
    timeDelta = getMaxClampDelta();
  }
  return timeDelta;
}

//------------------------------------------------------------------------------
double
Timing::ticksToMilliSeconds(Ticks ticks)
{
  // NB: ticks should be clamped with getClampedDuration() to prevent overflow
  ticks *= MILLISECONDS_PER_SECOND;

  return static_cast<double>(ticks) / getQpcFrequency();
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Stats::IntervalRecords&
Stats::getIntervalRecords()
{
  static IntervalRecords interval = Stats::IntervalRecords();
  return interval;
}

//------------------------------------------------------------------------------
Stats::FrameRecords&
Stats::getFrameRecords(const int frameIdx)
{
  return getIntervalRecords()[frameIdx];
}

//------------------------------------------------------------------------------
Stats::CollatedIntervalRecords&
Stats::getCollatedIntervalRecords()
{
  static CollatedIntervalRecords interval = CollatedIntervalRecords();
  return interval;
}

//------------------------------------------------------------------------------
Stats::CollatedFrameRecords&
Stats::getCollatedFrameRecords(const int frameIdx)
{
  return getCollatedIntervalRecords()[frameIdx];
}

//------------------------------------------------------------------------------
int&
Stats::getCurrentFrameIdx()
{
  static int idx = 0;
  return idx;
}

//------------------------------------------------------------------------------
int&
Stats::incrementCurrentFramedIdx()
{
  int& idx = getCurrentFrameIdx();
  if (++idx >= FRAME_COUNT)
  {
    idx = 0;
  }
  return idx;
}

//------------------------------------------------------------------------------
void
Stats::clearFrame(FrameRecords& frame)
{
  TimedRecordArray temp;
  frame.records.swap(temp);
  frame.numRecords    = 0;
  frame.callGraphHead = nullptr;
}

//------------------------------------------------------------------------------
void
Stats::clearCollatedFrame(CollatedFrameRecords& frame)
{
  CollatedFrameRecords temp;
  frame.swap(temp);
}

//------------------------------------------------------------------------------
void
Stats::condenseFrameRecords(int frameIdx)
{
  auto& dstFrame = getCollatedFrameRecords(frameIdx);
  clearCollatedFrame(dstFrame);

  const auto& srcFrame = getFrameRecords(frameIdx);
  for (size_t i = 0; i < srcFrame.numRecords; ++i)
  {
    const auto& srcRecord = srcFrame.records[i];

    size_t hash = createTimedRecordHash(srcRecord.file, srcRecord.lineNumber);
    auto& accumRecord = dstFrame[hash];

    accumRecord.ticks += srcRecord.duration;
    accumRecord.callsCount++;

    accumRecord.lineNumber = srcRecord.lineNumber;
    accumRecord.file       = srcRecord.file;
    accumRecord.function   = srcRecord.function;
  }
}

//------------------------------------------------------------------------------
void
Stats::signalFrameEnd()
{
  condenseFrameRecords(getCurrentFrameIdx());

  int newIdx = incrementCurrentFramedIdx();
  clearFrame(getFrameRecords(newIdx));
}

//------------------------------------------------------------------------------
Stats::AccumulatedRecords
Stats::accumulateRecords()
{
  AccumulatedRecords accumulatedRecords;
  const auto& srcFrames = getCollatedIntervalRecords();
  for (const auto& frame : srcFrames)
  {
    for (const auto& r : frame)
    {
      const auto& srcRecord = r.second;
      auto& record          = accumulatedRecords[r.first];

      record.ticks.accumulate(srcRecord.ticks);
      record.callsCount.accumulate(srcRecord.callsCount);
      record.ticksPerCount.accumulate(srcRecord.ticks / srcRecord.callsCount);
    }
  }

  return accumulatedRecords;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TimedRaiiBlock::TimedRaiiBlock(
  const int line, const char* file, const char* function)
    : _parent(getCurrentOpenBlockByRef())
{
  getCurrentOpenBlockByRef() = this;

  auto& currentFrame = Stats::getFrameRecords(Stats::getCurrentFrameIdx());
  auto recordIndex   = currentFrame.numRecords;

  TimedRecord& record = currentFrame.records[recordIndex];
  _record             = &record;
  _record->startTime  = Timing::getCurrentTimeInTicks();
  _record->lineNumber = line;
  _record->file       = file;
  _record->function   = function;

  if (_parent)
  {
    _parent->_record->childNodes.push_back(_record);
  }
  else
  {
    currentFrame.callGraphHead = _record;
  }

  if (currentFrame.numRecords < Stats::MAX_RECORD_COUNT - 1)
  {
    currentFrame.numRecords++;
  }
  else
  {
    LOG_ERROR("MAX_RECORD_COUNT exceeded. Increase Value");
  }
}

//------------------------------------------------------------------------------
TimedRaiiBlock::~TimedRaiiBlock()
{
  ASSERT(_record);
  _record->duration = Timing::getClampedDuration(
    _record->startTime, Timing::getCurrentTimeInTicks());

  getCurrentOpenBlockByRef() = const_cast<TimedRaiiBlock*>(_parent);
}

//------------------------------------------------------------------------------
TimedRaiiBlock*&
TimedRaiiBlock::getCurrentOpenBlockByRef()
{
  static TimedRaiiBlock* current = nullptr;
  return current;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
size_t
createTimedRecordHash(const std::string_view& filePath, const int lineNumber)
{
  // Don't bother hashing the full path. Only the last few characters
  // will differ enough to be useful for hashing.
  const size_t NUM_CHARS = 12;
  const size_t subPos
    = (filePath.length() > NUM_CHARS) ? filePath.length() - NUM_CHARS : 0;
  const std::string_view file = filePath.substr(subPos);
  const std::size_t h1        = std::hash<std::string_view>{}(file);
  const std::size_t h2        = std::hash<int>{}(lineNumber);
  return h1 ^ (h2 << 1);
}

//------------------------------------------------------------------------------
}    // namespace logger

#endif    // LOGGER_PROFILER_IMPLEMENTATION
