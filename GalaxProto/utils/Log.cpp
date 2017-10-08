#include "pch.h"
#include "Log.h"

//------------------------------------------------------------------------------
namespace logger
{

//------------------------------------------------------------------------------
uint64_t
Timing::getCurrentTimeInTicks()
{
	LARGE_INTEGER currentTime = {0LL};
	if (!QueryPerformanceCounter(&currentTime))
	{
		throw std::exception("QueryPerformanceCounter");
	}
	return currentTime.QuadPart;
}

//------------------------------------------------------------------------------
uint64_t
Timing::initFrequency()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency))
	{
		throw std::exception("QueryPerformanceFrequency");
	}
	return frequency.QuadPart;
}

//------------------------------------------------------------------------------
uint64_t&
Timing::getQpcFrequency()
{
	static uint64_t qpcFrequency = initFrequency();
	return qpcFrequency;
}

//------------------------------------------------------------------------------
uint64_t
Timing::initMaxDelta()
{
	// Initialize max delta to 1/10 of a second.
	return getQpcFrequency() / 10;
}

//------------------------------------------------------------------------------
uint64_t&
Timing::getQpcMaxDelta()
{
	static uint64_t qpcMaxDelta = initMaxDelta();
	return qpcMaxDelta;
}

//------------------------------------------------------------------------------
uint64_t
Timing::getClampedDuration(const uint64_t tEarliest, const uint64_t tLatest)
{
	uint64_t timeDelta = tLatest - tEarliest;

	// Clamp excessively large time deltas (e.g. after paused in the debugger).
	if (timeDelta > getQpcMaxDelta())
	{
		timeDelta = getQpcMaxDelta();
	}
	return timeDelta;
}

//------------------------------------------------------------------------------
double
Timing::ticksToMilliSeconds(uint64_t ticks)
{
	// Convert QPC units into a canonical tick format. This cannot overflow
	// due to the previous clamp.
	ticks *= TICKS_PER_SECOND;
	ticks /= getQpcFrequency();

	return static_cast<double>(ticks) / TICKS_PER_MILLISECOND;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Stats::AllSnapShots&
Stats::getAllSnapShots()
{
	static AllSnapShots snapShots = AllSnapShots();
	return snapShots;
}

//------------------------------------------------------------------------------
Stats::SnapShot&
Stats::getSnapShot(const int snapShotIdx)
{
	return getAllSnapShots()[snapShotIdx];
}

//------------------------------------------------------------------------------
Stats::AllAggregatedSnapShots&
Stats::getAggregatedFrameRecords()
{
	static AllAggregatedSnapShots snapShots = AllAggregatedSnapShots();
	return snapShots;
}

//------------------------------------------------------------------------------
Stats::AggregatedRecords&
Stats::getAggregatedRecords(const int snapShotIdx)
{
	return getAggregatedFrameRecords()[snapShotIdx];
}

//------------------------------------------------------------------------------
int&
Stats::getCurrentSnapShotIdx()
{
	static int idx = 0;
	return idx;
}

//------------------------------------------------------------------------------
int&
Stats::incrementSnapShotIdx()
{
	int& idx = getCurrentSnapShotIdx();
	if (++idx >= SNAPSHOT_COUNT)
	{
		idx = 0;
	}
	return idx;
}

//------------------------------------------------------------------------------
void
Stats::clearSnapShot(SnapShot& snapShot)
{
	Records temp;
	snapShot.records.swap(temp);
	snapShot.numRecords = 0;
}

//------------------------------------------------------------------------------
void
Stats::clearFrameAggregate(AggregatedRecords& snapShot)
{
	AggregatedRecords temp;
	snapShot.swap(temp);
}

//------------------------------------------------------------------------------
void
Stats::signalFrameEnd()
{
	int newIdx = incrementSnapShotIdx();
	clearSnapShot(getSnapShot(newIdx));
	clearFrameAggregate(getAggregatedRecords(newIdx));
}

//------------------------------------------------------------------------------
Stats::AnalyticRecords
Stats::computeAnalyticRecords()
{
	AnalyticRecords analyticRecords;
	const auto& aggregatedSnapShots = getAggregatedFrameRecords();
	for (const auto& snapShot : aggregatedSnapShots)
	{
		for (const auto& rec : snapShot)
		{
			const auto& sourceRecord = rec.second;
			auto& record						 = analyticRecords[rec.first];

			record.ticks.accumulate(sourceRecord.ticks);
			record.callsCount.accumulate(sourceRecord.callsCount);
			record.ticksPerCount.accumulate(
				sourceRecord.ticks / sourceRecord.callsCount);

			record.function		= sourceRecord.function;
			record.file				= sourceRecord.file;
			record.lineNumber = sourceRecord.lineNumber;
		}
	}

	return analyticRecords;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
TimedRaiiBlock::TimedRaiiBlock(
	const size_t hashIndex,
	const int line,
	const char* file,
	const char* function)
		: _parent(getCurrentOpenBlockByRef())
{
	getCurrentOpenBlockByRef() = this;

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
			__FUNCTION__);
	}

	TimedRecord& record				= currentSnapShot.records[recordIndex];
	_record										= &record;
	_record->hashIndex				= hashIndex;
	_record->startTimeInTicks = Timing::getCurrentTimeInTicks();
	_record->lineNumber				= line;
	_record->file							= file;
	_record->function					= function;

	if (_parent)
	{
		_parent->_record->childNodes.push_back(_record);
	}
	else
	{
		currentSnapShot.flameHead = _record;
	}
}

//------------------------------------------------------------------------------
TimedRaiiBlock::~TimedRaiiBlock()
{
	ASSERT(_record);
	_record->totalTicks = Timing::getClampedDuration(
		_record->startTimeInTicks, Timing::getCurrentTimeInTicks());

	auto& snapShotIdx = Stats::getCurrentSnapShotIdx();
	auto& aggregateRecord
		= Stats::getAggregatedRecords(snapShotIdx)[_record->hashIndex];
	aggregateRecord.ticks += _record->totalTicks;
	aggregateRecord.callsCount++;

	aggregateRecord.lineNumber = _record->lineNumber;
	aggregateRecord.file			 = _record->file;
	aggregateRecord.function	 = _record->function;

	// TEST message
	// double elapsedTimeMs = Timing::ticksToMilliSeconds(_record->totalTicks);
	// logMsgImp(
	//	"TIMED",
	//	"%9.6fms - hash %ull\n",
	//	_record->file,
	//	_record->lineNumber,
	//	_record->function,
	//	elapsedTimeMs,
	//	_record->hashIndex);

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
	const std::size_t h1				= std::hash<std::string_view>{}(file);
	const std::size_t h2				= std::hash<int>{}(lineNumber);
	return h1 ^ (h2 << 1);
}

//------------------------------------------------------------------------------
};		// namespace logger