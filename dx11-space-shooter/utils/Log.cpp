#include "pch.h"
#include "Log.h"

//------------------------------------------------------------------------------
namespace logger
{
//------------------------------------------------------------------------------
Ticks
Timing::getCurrentTimeInTicks()
{
	LARGE_INTEGER currentTime;
	if (!QueryPerformanceCounter(&currentTime))
	{
		throw std::exception("QueryPerformanceCounter");
	}
	return currentTime.QuadPart;
}

//------------------------------------------------------------------------------
Ticks
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
Ticks&
Timing::getQpcFrequency()
{
	static Ticks qpcFrequency = initFrequency();
	return qpcFrequency;
}

//------------------------------------------------------------------------------
Ticks
Timing::initMaxDelta()
{
	// Initialize max delta to 1/10 of a second.
	return getQpcFrequency() / 10;
}

//------------------------------------------------------------------------------
Ticks&
Timing::getQpcMaxDelta()
{
	static Ticks qpcMaxDelta = initMaxDelta();
	return qpcMaxDelta;
}

//------------------------------------------------------------------------------
Ticks
Timing::getClampedDuration(const Ticks tEarliest, const Ticks tLatest)
{
	Ticks timeDelta = tLatest - tEarliest;

	// Clamp excessively large time deltas (e.g. after paused in the debugger).
	if (timeDelta > getQpcMaxDelta())
	{
		timeDelta = getQpcMaxDelta();
	}
	return timeDelta;
}

//------------------------------------------------------------------------------
double
Timing::ticksToMilliSeconds(Ticks ticks)
{
	// Convert QPC units into a canonical tick format. This cannot overflow
	// due to the previous clamp.
	ticks *= TICKS_PER_SECOND;
	ticks /= getQpcFrequency();

	return static_cast<double>(ticks) / TICKS_PER_MILLISECOND;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Stats::IntervalRecords&
Stats::getIntervalRecords()
{
	static IntervalRecords interval = IntervalRecords();
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
	frame.numRecords		= 0;
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
		accumRecord.file			 = srcRecord.file;
		accumRecord.function	 = srcRecord.function;
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
			auto& record					= accumulatedRecords[r.first];

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
	auto recordIndex	 = currentFrame.numRecords;

	TimedRecord& record = currentFrame.records[recordIndex];
	_record							= &record;
	_record->startTime	= Timing::getCurrentTimeInTicks();
	_record->lineNumber = line;
	_record->file				= file;
	_record->function		= function;

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
		logMsgImp(
			"ERROR",
			"MAX_RECORD_COUNT exceeded. Increase Value\n",
			__FILE__,
			__LINE__,
			__FUNCTION__);
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
	const std::size_t h1				= std::hash<std::string_view>{}(file);
	const std::size_t h2				= std::hash<int>{}(lineNumber);
	return h1 ^ (h2 << 1);
}

//------------------------------------------------------------------------------
};		// namespace logger
