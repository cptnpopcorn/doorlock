#ifndef B567AA07_C379_4687_9027_2CA1DC9C569F
#define B567AA07_C379_4687_9027_2CA1DC9C569F

#include "TargetFrameWriter.h"
#include <utility>

template<
	class LcsInvalidFunc,
	class AckFunc,
	class NackFunc,
	class TfiInvalidFunc,
	class DataToHostFunc,
	class ErrorFunc,
	class IncompleteFunc>
	class FrameWriterFuncs final : public TargetFrameWriter
{
public:
	FrameWriterFuncs(
		LcsInvalidFunc &&lcsInvalidFunc,
		AckFunc &&ackFunc,
		NackFunc &&nackFunc,
		TfiInvalidFunc &&tfiInvalidFunc,
		DataToHostFunc &&dataTohostFunc,
		ErrorFunc &&errorFunc,
		IncompleteFunc &&incompleteFunc
	) :
	lcsInvalidFunc{std::move(lcsInvalidFunc)},
	ackFunc{std::move(ackFunc)},
	nackFunc{std::move(nackFunc)},
	tfiInvalidFunc{std::move(tfiInvalidFunc)},
	dataTohostFunc{std::move(dataTohostFunc)},
	errorFunc{std::move(errorFunc)},
	incompleteFunc{std::move(incompleteFunc)}
	{
	}

	void LcsInvalid() override { lcsInvalidFunc(); }
	void Ack() override { ackFunc(); }
	void Nack() override { nackFunc(); }
	void TfiInvalid(uint8_t tfi) override { tfiInvalidFunc(); }
	TargetDataWriter& DataToHost() override { return dataTohostFunc(); }
	TargetDataValidator& Error(uint8_t err) override { return errorFunc(); }
	void FrameIncomplete() override { incompleteFunc(); }

	LcsInvalidFunc lcsInvalidFunc;
	AckFunc ackFunc;
	NackFunc nackFunc;
	TfiInvalidFunc tfiInvalidFunc;
	DataToHostFunc dataTohostFunc;
	ErrorFunc errorFunc;
	IncompleteFunc incompleteFunc;
};

template<
	class LcsInvalidFunc,
	class AckFunc,
	class NackFunc,
	class TfiInvalidFunc,
	class DataToHostFunc,
	class ErrorFunc,
	class IncompleteFunc>
	FrameWriterFuncs<LcsInvalidFunc, AckFunc, NackFunc, TfiInvalidFunc, DataToHostFunc, ErrorFunc, IncompleteFunc>
	make_frame_writer(
		LcsInvalidFunc &&lcsInvalidFunc,
		AckFunc &&ackFunc,
		NackFunc &&nackFunc,
		TfiInvalidFunc &&tfiInvalidFunc,
		DataToHostFunc &&dataTohostFunc,
		ErrorFunc &&errorFunc,
		IncompleteFunc &&incompleteFunc
	)
{
	return {
		std::move(lcsInvalidFunc),
		std::move(ackFunc),
		std::move(nackFunc),
		std::move(tfiInvalidFunc),
		std::move(dataTohostFunc),
		std::move(errorFunc),
		std::move(incompleteFunc)};
}

#endif /* B567AA07_C379_4687_9027_2CA1DC9C569F */
