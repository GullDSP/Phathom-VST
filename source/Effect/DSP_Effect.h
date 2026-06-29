//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once

#include "Effect/Parameters.h"
#include "../DSP/Smoother.h"
#include "../DSP/OnePoleFilters.h"
#include "../DSP/Biquads.h"
#include "../DSP/Oversampler.h"
#include "../DSP/config.h"
#include "../DSP/DenormalProtection.h"



/// <summary>
/// Manager class for dsp effects
/// </summary>
class AudioEffectModule {
public:

	enum HiFreqSelectorValues {
		kHiNormal = 100,
		kHiCut,
		kHiBoost
	};
	enum LoFreqSelectorValues {
		kLoNormal = 101,
		kLoCut,
		kLoBoost
	};

	void setSampleRateBlockSize(int sample_rate, int block_size) {
		sampleRate = sample_rate;
		blockSize = block_size;

		Nonlinearity.setSampleRate(sample_rate, block_size);

		InputNotch.setSampleRate(sample_rate);
		InputNotch.calculateCoefficients(CONFIG::PRE_NOTCH_HZ, CONFIG::PRE_NOTCH_Q, -24);

		PostNotch.setSampleRate(sample_rate);
		PostNotch.calculateCoefficients(CONFIG::POST_NOTCH_HZ, CONFIG::POST_NOTCH_Q, -24);
		PostLP.setSampleRate(sample_rate);
		PostLP.calculateCoefficients(CONFIG::POST_LP1_HZ, CONFIG::POST_LP1_Q);
		PostLP2.setSampleRate(sample_rate);
		PostLP2.calculateCoefficients(CONFIG::POST_LP2_HZ, CONFIG::POST_LP2_Q);

		InGainSmoother.setSmoothTime(50, sample_rate);
		OutGainSmoother.setSmoothTime(50, sample_rate);
		MixSmoother.setSmoothTime(50, sample_rate);
		
		HiCtrlLP.setSampleRate(sample_rate);
		HiCtrlLP.setCenterHz(CONFIG::HI_CUT_HZ);
		HiCtrlHP.setSampleRate(sample_rate);
		HiCtrlHP.setCenterHz(CONFIG::HI_BOOST_HZ);

		PreDriveStaticLP.setSampleRate(sample_rate);
		PreDriveStaticLP.setCenterHz(CONFIG::PRE_LP_HZ);

		BassControlHPLowBoost.setSampleRate(sample_rate);
		BassControlHPLowBoost.setCenterHz(CONFIG::LO_BOOST_HZ);
		BassControlHPLoNormal.setSampleRate(sample_rate);
		BassControlHPLoNormal.setCenterHz(CONFIG::LO_NORMAL_HZ);
		BassControlHPLoCut.setSampleRate(sample_rate);
		BassControlHPLoCut.setCenterHz(CONFIG::LO_CUT_HZ);

		DryBuffer.assign(blockSize, 0.0f);
		WetBuffer.assign(blockSize, 0.0f);

		updateParameters();
		resetAll();

	}

	void onPresetLoaded() {
		updateParameters();
		resetDSP();
		collapseSmoothers();
	}

	void setParameterHandler(ParameterHandler* params) {
		if (!params) return;

		pParams = params;
	}
	void processBlock(float* outL, float* inL, int num_samples) {
		DenormalHandler AntiDenormal;
		updateParameters();
		if (mIsBypass) {
			// if bypassed and we have buffers that are not the same location, copy them
			if (inL && outL && inL != outL) {
				memcpy(outL, inL, sizeof(float) * num_samples);
			}

			return;
		}

		if (!inL) return;
		if (!outL) return;
	
		memcpy(DryBuffer.data(), inL, sizeof(float) * num_samples);

		// Input Gain
		float inGainAsdB = CONFIG::MIN_IN_DB + (mInGain * (CONFIG::MAX_GAIN_RANGE_DB));
		float factor_in_target = powf(10.0f, (inGainAsdB / 20.0f));
		for (int i = 0; i < num_samples; i++) {
			float factor = InGainSmoother.getSmoothedValue(factor_in_target);
			DryBuffer[i] = DryBuffer[i] * factor;
		}

		//Input Filters
		InputNotch.getBlock(WetBuffer.data(), DryBuffer.data(), num_samples);

		// Lo and Hi pre gain filters
		// this is sample by sample to avoid lots of temporary buffers (all filters need to be run always anyway)
		for (int i = 0; i < num_samples; i++) {
			float spl = 0;

			// Lo Filters
			float lo_filter_boost = BassControlHPLowBoost.getNext(WetBuffer[i]);
			float lo_filter_normal = BassControlHPLoNormal.getNext(WetBuffer[i]);
			float lo_filter_cut = BassControlHPLoCut.getNext(WetBuffer[i]);

			if (mLoCut == LoFreqSelectorValues::kLoBoost) {
				spl = lo_filter_boost;
			}
			else if (mLoCut == LoFreqSelectorValues::kLoNormal) {
				spl = lo_filter_normal;
			}
			else if (mLoCut == LoFreqSelectorValues::kLoCut) {
				spl = lo_filter_cut;
			}

			spl = PreDriveStaticLP.getNext(spl);

			float hi_filter_boost = (spl + HiCtrlHP.getNext(spl));
			float hi_filter_cut = HiCtrlLP.getNext(spl);

			if (mHiCut == HiFreqSelectorValues::kHiBoost) {
				spl = hi_filter_boost;
			}
			else if (mHiCut == HiFreqSelectorValues::kHiNormal) {

			}
			else if (mHiCut == HiFreqSelectorValues::kHiCut) {
				spl = hi_filter_cut;
			}

			WetBuffer[i] = spl;
		}

		if (mDoDryEq) {
			memcpy(DryBuffer.data(), WetBuffer.data(), sizeof(float) * num_samples);
		}

		// Nonlinearity, this internally decides whether to oversample based on sample rate
		Nonlinearity.getBlockOversampled(WetBuffer.data(), num_samples);

		// Post drive filters
		PostNotch.getBlock(WetBuffer.data(), WetBuffer.data(), num_samples);
		PostLP.getBlock(WetBuffer.data(), WetBuffer.data(), num_samples);
		PostLP2.getBlock(WetBuffer.data(), WetBuffer.data(), num_samples);


		// Calculate Mix (Linear as perfectly correlated)
		float phase_mod = 1.0;
		// Phase button
		if (mInvertPhase) {
			phase_mod = phase_mod * -1;
		}


		for (int i = 0; i < num_samples; i++) {

			float mMixNow = MixSmoother.getSmoothedValue(mMix);
			if (mMixNow > 1.0f) mMixNow = 1.0f;
			if (mMixNow < 0.0f) mMixNow = 0.0f;
			outL[i] = (mMixNow * WetBuffer[i] * phase_mod) + ((1.0- mMixNow) * DryBuffer[i]);
		}

		// Output gain
		float outGainAsdB = CONFIG::MIN_IN_DB + (mOutGain * (CONFIG::MAX_GAIN_RANGE_DB));
		float factor_out_target = powf(10.0f, (outGainAsdB / 20.0f));
		for (int i = 0; i < num_samples; i++) {
			float factor = OutGainSmoother.getSmoothedValue(factor_out_target);
			outL[i] = outL[i] * factor;
		}

		// Hardclip (safety limit)
		for (int i = 0; i < num_samples; i++) {
			if (outL[i] > 1.0f) outL[i] = 1.0f;
			if (outL[i] < -1.0f) outL[i] = -1.0f;
		}

		// Clipping indicator data (1.0 if over 0.99)
		ui_max_in = 0;
		ui_max_out = 0;
		for (int i = 0; i < num_samples; i++) {
			float abs_in = fabsf(DryBuffer[i]);
			float abs_out = fabsf(outL[i]);

			if (abs_in > 0.99f) ui_max_in = 1.0;
			if (abs_out > 0.99f) ui_max_out = 1.0;

		}

	}

	void setBypass(float value) {
		mIsBypass = (value > 0.5) ? true : false;

		if (mIsBypass) {
			// if just enabled collapse smoothers and reset phases
			resetAll();
		}
	}

	void getUIData(float& InputLevel, float& OutLevel) {
		InputLevel = ui_max_in;
		OutLevel = ui_max_out;
	}
	
	void updateParameters() {
		if (!pParams) return;

		if (pParams->isParamDirty(kBypass)) {
			setBypass(pParams->getParamValue(kBypass));
			
		}
		if (pParams->isParamDirty(kInGain)) {
			mInGain = pParams->getParamValue(kInGain);
		}
		if (pParams->isParamDirty(kOutLevel)) {
			mOutGain = pParams->getParamValue(kOutLevel);
		}
		if (pParams->isParamDirty(kDrive)) {
			 Nonlinearity.setDrive(pParams->getParamValue(kDrive));
		}
		if (pParams->isParamDirty(kMix)) {
			mMix = pParams->getParamValue(kMix);
		}
		if (pParams->isParamDirty(kDryEq)) {
			mDoDryEq = pParams->getParamValue(kDryEq) > 0.5 ? true : false;
		}
		if (pParams->isParamDirty(kPhaseFlip)) {
			mInvertPhase = pParams->getParamValue(kPhaseFlip) > 0.5 ? true : false;
		}
		if (pParams->isParamDirty(kHiSwitch)) {
			float raw = pParams->getParamValue(kHiSwitch);
			
			if (raw < 0.3) {
				mHiCut = HiFreqSelectorValues::kHiNormal;
			
			}
			else if (raw > 0.7) {
				mHiCut = HiFreqSelectorValues::kHiBoost;
			}
			else {
				mHiCut = HiFreqSelectorValues::kHiCut;
			}

		}
		if (pParams->isParamDirty(kLoSwitch)) {
			float raw = pParams->getParamValue(kLoSwitch);

			if (raw < 0.3) {
				mLoCut = LoFreqSelectorValues::kLoNormal;

			}
			else if (raw > 0.7) {
				mLoCut = LoFreqSelectorValues::kLoBoost;
			}
			else {
				mLoCut = LoFreqSelectorValues::kLoCut;
			}
			
		}

	}
	/// <summary>
	/// Reset LFO phases, and collapse smoothers
	/// to targets
	/// </summary>
	void resetAll() {
		InGainSmoother.reset();
		OutGainSmoother.reset();
		MixSmoother.reset();

		resetDSP();
	}
	/// <summary>
	/// Reset DSP and set smoothers to last known 
	/// Call before bouncing etc to prevent drifting parameters
	/// or stale states
	/// </summary>
	void prepareToPlay() {
		collapseSmoothers();
		resetDSP();
	}

	void resetDSP() {
		HiCtrlLP.resetState();
		HiCtrlHP.resetState();
		PreDriveStaticLP.resetState();
		BassControlHPLowBoost.resetState();
		BassControlHPLoNormal.resetState();
		BassControlHPLoCut.resetState();
		InputNotch.reset();
		PostNotch.reset();
		PostLP.reset();
		PostLP2.reset();
		Nonlinearity.prepareToPlay();
		DecimationFilter.resetState();
	}

	void collapseSmoothers() {
		InGainSmoother.collapseTo(mInGain);
		OutGainSmoother.collapseTo(mOutGain);
		MixSmoother.collapseTo(mMix);

	}

private:
	int sampleRate = 48000;
	int blockSize = 0;
	bool mIsBypass = false;

	OnePoleLowpass DecimationFilter;

	std::vector<float> UpsampledBuffer;

	ParameterHandler* pParams = nullptr;
	ValueSmoother InGainSmoother;
	ValueSmoother OutGainSmoother;
	ValueSmoother MixSmoother;



	float mInGain = 0.714;
	float mOutGain = 0.714;
	float mMix = 0.5;
	bool mDoDryEq = false;
	bool mInvertPhase = false;


	HiFreqSelectorValues mHiCut = HiFreqSelectorValues::kHiNormal;
	LoFreqSelectorValues mLoCut = LoFreqSelectorValues::kLoNormal;

	float ui_max_in = 0;
	float ui_max_out = 0;

	BiquadFilter InputNotch{ BiquadFilter::PassFilterType::kPeak };
	BiquadFilter PostNotch{ BiquadFilter::PassFilterType::kPeak };
	BiquadFilter PostLP{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter PostLP2{ BiquadFilter::PassFilterType::kLowpass };

	BiquadFilter HighShelf{ BiquadFilter::PassFilterType::kHighShelf };

	OnePoleLowpass HiCtrlLP;
	OnePoleHighpass HiCtrlHP;
	OnePoleLowpass PreDriveStaticLP;

	OnePoleHighpass BassControlHPLowBoost;
	OnePoleHighpass BassControlHPLoNormal;
	OnePoleHighpass BassControlHPLoCut;

	DSPOversamplerWrapper Nonlinearity;

	std::vector<float> DryBuffer;
	std::vector<float> WetBuffer;
};
