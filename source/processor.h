//------------------------------------------------------------------------
// Copyright(c) 2026 GullDSP.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "Effect/Parameters.h"
#include "Effect/DSP_Effect.h"

namespace EFFECT {

class PhathomProcessor : public Steinberg::Vst::AudioEffect
{
public:
	PhathomProcessor ();
	~PhathomProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new PhathomProcessor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

	void UIUpdate(Steinberg::Vst::ProcessData& data);

	Steinberg::uint32 PLUGIN_API getTailSamples() SMTG_OVERRIDE {
		const float maxTailSeconds = 0.1;
		// 0.1 secs just to be safe
		return maxTailSeconds * sampleRateForTail;
	}

	Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool state /*in*/) override {
		// Reset effects before playback
		if (state) {
			EffectL.prepareToPlay();
			EffectR.prepareToPlay();
		}

		return AudioEffect::setProcessing(state);
	}

//------------------------------------------------------------------------
protected:
	AudioEffectModule EffectL;
	AudioEffectModule EffectR;
	ParameterHandler Params;

	int timerCount = 0;
	int oneFrameTimeSpls = 44100 / 60;
	int sampleRateForTail = 48000;
};

//------------------------------------------------------------------------
} // namespace EFFECT
