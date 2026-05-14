//------------------------------------------------------------------------
// Copyright(c) 2026 GullDSP.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
using namespace Steinberg;

namespace EFFECT {
//------------------------------------------------------------------------
// Processor
//------------------------------------------------------------------------
PhathomProcessor::PhathomProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kPhathomControllerUID);
}

//------------------------------------------------------------------------
PhathomProcessor::~PhathomProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::initialize (FUnknown* context)
{
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);
	


	Steinberg::Vst::ParameterContainer tparameters;
	registerParameters(tparameters);
	Params.getHostParameters(tparameters);
	EffectL.setParameterHandler(&Params);
	EffectR.setParameterHandler(&Params);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::terminate ()
{
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::setActive (TBool state)
{
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::process (Vst::ProcessData& data)
{
	//Read inputs parameter changes-----------
	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
			{
				Vst::ParamValue value;
				int32 sampleOffset = 0;
				int32 id = paramQueue->getParameterId();
				if (id == Steinberg::Vst::kNoParamId) continue;
			
				int32 numPoints = paramQueue->getPointCount();
				if (numPoints == 0) continue;
				paramQueue->getPoint(numPoints - 1, sampleOffset, value); // Last point
				Params.setParameter(id, value);
			}
		}
	}

	int numBusInputs = data.numInputs;
	int numBusOutputs = data.numOutputs;


	if (data.numSamples < 1) {
		return kResultOk;
	}

	if (!numBusInputs) {
		return kResultOk;
	}
	if (!numBusOutputs) {
		return kResultOk;
	}

	int numChanInputs = data.inputs[0].numChannels;
	int numChanOutputs = data.outputs[0].numChannels;

	if (numChanInputs == 0 || numChanOutputs == 0) return kResultOk;

	// we know we have at least these
	float* inputL = data.inputs[0].channelBuffers32[0];
	float* outputL = data.outputs[0].channelBuffers32[0];

	float* inputR = (numChanInputs > 1) ? data.inputs[0].channelBuffers32[1] : nullptr;
	float* outputR = (numChanOutputs > 1) ? data.outputs[0].channelBuffers32[1] : nullptr;

	if (inputL && outputL) {
		// dual mono
		EffectL.processBlock(outputL, inputL, data.numSamples);

		if (outputR && !inputR) {
			// if two outs one in, copy left to right
			memcpy(outputR, outputL, sizeof(float) * data.numSamples);
		}
	}

	if (inputR && outputR) {
		// dual mono
		EffectR.processBlock(outputR, inputR, data.numSamples);

	}

	Params.setAllClean();

	// Send UI data
	timerCount += data.numSamples;

	if (timerCount < oneFrameTimeSpls) {
		return kResultOk;
	}

	UIUpdate(data);

	timerCount = 0;

	return kResultOk;
}


void PhathomProcessor::UIUpdate(Vst::ProcessData& data) {
	if (data.outputParameterChanges) {

		float in_uiL = 0;
		float out_uiL = 0;
		float in_uiR = 0;
		float out_uiR = 0;

		EffectL.getUIData(in_uiL, out_uiL);
		EffectR.getUIData(in_uiR, out_uiR);

		float in_ui = std::max(in_uiL, in_uiR);
		float out_ui = std::max(out_uiL, out_uiR);

		int index = 1;
		Steinberg::Vst::IParamValueQueue* queueA = data.outputParameterChanges->addParameterData(kUIInAmp, index);
		Steinberg::Vst::IParamValueQueue* queueB = data.outputParameterChanges->addParameterData(kUIOutAmp, index);
		if (!queueA || !queueB) return;
		queueA->addPoint(0, in_ui, index);
		queueB->addPoint(0, out_ui, index);

	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	sampleRateForTail = newSetup.sampleRate;

	EffectL.setSampleRateBlockSize(newSetup.sampleRate, newSetup.maxSamplesPerBlock);
	EffectR.setSampleRateBlockSize(newSetup.sampleRate, newSetup.maxSamplesPerBlock);
	EffectL.onPresetLoaded();
	EffectR.onPresetLoaded();

	// 30th of second as spls
	oneFrameTimeSpls = newSetup.sampleRate / 60;
	return AudioEffect::setupProcessing(newSetup);


}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::setState (IBStream* state)
{

	if (!state) return kResultFalse;
	Steinberg::IBStreamer streamer(state, kLittleEndian);

	Params.setState(streamer);
	EffectL.onPresetLoaded();
	EffectR.onPresetLoaded();

	return kResultOk;

}

//------------------------------------------------------------------------
tresult PLUGIN_API PhathomProcessor::getState (IBStream* state)
{
	Steinberg::IBStreamer streamer(state, kLittleEndian);
	
	return Params.getState(streamer);
}

//------------------------------------------------------------------------
} // namespace EFFECT
