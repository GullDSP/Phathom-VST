#pragma once
#include <math.h>
//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include "math.h"
#include "Biquads.h"
#include "../DSP/StatefulNonlinearity.h"

namespace OVERSAMPLER_CONFIG {
	constexpr float RECONSTRUCTION_NYQ_MULT = 0.8;
	constexpr float INTERPOLATION_NYQ_MULT = 0.8;
	constexpr float X2_OVERSAMPLING_THRESHOLD_SAMPLE_RATE = 80000.0f;
	constexpr float X1_OVERSAMPLING_THRESHOLD_SAMPLE_RATE = 160000.0f;


}

/// <summary>
/// A Wrapper performing oversampling
/// </summary>
class DSPOversamplerWrapper {


public:
	void setSampleRate(int sample_rate, int block_size) {

		reset();

		sampleRate = sample_rate;
		blockSize = block_size;

		// Sample Rate Oversampling selection
		if (sampleRate >= OVERSAMPLER_CONFIG::X1_OVERSAMPLING_THRESHOLD_SAMPLE_RATE) {
			OVERSAMPLE_FACTOR = 1;
			b_will_oversample_ = false;
		} 
		else if (sampleRate >= OVERSAMPLER_CONFIG::X2_OVERSAMPLING_THRESHOLD_SAMPLE_RATE) {
			OVERSAMPLE_FACTOR = 2;
			b_will_oversample_ = true;
		}
		else {
			OVERSAMPLE_FACTOR = 4;
			b_will_oversample_ = true;
		}


		int currentSampleRate = sample_rate;
		int currentBlockSize = block_size;

		if (b_will_oversample_) {
			currentSampleRate = OVERSAMPLE_FACTOR * sample_rate;
			currentBlockSize = OVERSAMPLE_FACTOR * block_size;

			RECONSTRUCTION_HZ = OVERSAMPLER_CONFIG::RECONSTRUCTION_NYQ_MULT * 0.5f * sample_rate;
			INTERPOLATION_HZ = OVERSAMPLER_CONFIG::INTERPOLATION_NYQ_MULT * 0.5f * sample_rate;

		}
		else {
			// not used anyway, here for completenessa
			RECONSTRUCTION_HZ = OVERSAMPLER_CONFIG::RECONSTRUCTION_NYQ_MULT * 0.5f * sample_rate;
			INTERPOLATION_HZ = OVERSAMPLER_CONFIG::INTERPOLATION_NYQ_MULT * 0.5f * sample_rate;
		}

		Nonlinearity.setSampleRate(currentSampleRate, currentBlockSize);
		HystLoop.setSampleRate(currentSampleRate);
		ReconstructionLPA.setSampleRate(currentSampleRate);
		InterpolationLPA.setSampleRate(currentSampleRate);
		ReconstructionLPB.setSampleRate(currentSampleRate);
		InterpolationLPB.setSampleRate(currentSampleRate);
		ReconstructionLPC.setSampleRate(currentSampleRate);
		InterpolationLPC.setSampleRate(currentSampleRate);
		ReconstructionLPD.setSampleRate(currentSampleRate);
		InterpolationLPD.setSampleRate(currentSampleRate);

		ReconstructionLPA.calculateCoefficients(RECONSTRUCTION_HZ, 0.707f);
		InterpolationLPA.calculateCoefficients(INTERPOLATION_HZ, 0.707f);
		ReconstructionLPB.calculateCoefficients(RECONSTRUCTION_HZ, 0.707f);
		InterpolationLPB.calculateCoefficients(INTERPOLATION_HZ, 0.707f);
		ReconstructionLPC.calculateCoefficients(RECONSTRUCTION_HZ, 0.707f);
		InterpolationLPC.calculateCoefficients(INTERPOLATION_HZ, 0.707f);
		ReconstructionLPD.calculateCoefficients(RECONSTRUCTION_HZ, 0.707f);
		InterpolationLPD.calculateCoefficients(INTERPOLATION_HZ, 0.707f);

		UpsampledBuffer.assign(currentBlockSize, 0.0);

		

	}
	void prepareToPlay() {
		Nonlinearity.prepareToPlay();
		HystLoop.prepareToPlay();
		resetDSP();
	}
	void resetDSP() {
		ReconstructionLPA.reset();
		InterpolationLPA.reset();
		ReconstructionLPB.reset();
		InterpolationLPB.reset();
		ReconstructionLPC.reset();
		InterpolationLPC.reset();
		ReconstructionLPD.reset();
		InterpolationLPD.reset();

	}
	void reset() {
		resetDSP();
		HystLoop.reset();
		Nonlinearity.reset();
	}
	void collapseSmoothers() {
		Nonlinearity.collapseSmoothers();
		HystLoop.collapseSmoother();
	}
	void setDrive(float value) {
		Nonlinearity.setDrive(value);
		HystLoop.setDrive(value);
	}
	void setResponse(float value) {
		Nonlinearity.setResponse(value);
	}
	void setNonlinType(float value) {

		if (value < 0.5) {
			mUseHysLoop = true;

		}
		else {
			mUseHysLoop = false;
		}

	}
	void getBlockOversampled(float* Buffer, int num_samples) {

		int factor = OVERSAMPLE_FACTOR;

		if (!b_will_oversample_) {
			// If we don't need to oversample call the non oversampling block
			if (!mUseHysLoop) {
				Nonlinearity.getBlock(Buffer, num_samples);
			}
			else {
				HystLoop.getBlock(Buffer, Buffer, num_samples);
			}
		
			return;
		}

		// Oversampling
		int num_oversampled = num_samples * factor;

		// Zero stuff
		for (int i = 0; i < num_samples; i++) {
			int base_index = i * factor;
			UpsampledBuffer[base_index] = Buffer[i] * factor;

			for (int s = 1; s < factor; s++) {
				UpsampledBuffer[base_index + s] = 0;
			}

		}

		InterpolationLPA.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		InterpolationLPB.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		InterpolationLPC.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		InterpolationLPD.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		// Operation
		if (!mUseHysLoop) {
			Nonlinearity.getBlock(UpsampledBuffer.data(), num_oversampled);

		}
		else {
			HystLoop.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		}

		ReconstructionLPA.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		ReconstructionLPB.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		ReconstructionLPC.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		ReconstructionLPD.getBlock(UpsampledBuffer.data(), UpsampledBuffer.data(), num_oversampled);
		// Decimate 
		for (int i = 0; i < num_samples; i++) {
			Buffer[i] = UpsampledBuffer[(i * factor)];
		}
	}
private:
	int sampleRate = 48000;
	int blockSize = 0;
	int currentSampleRate = 0;
	int currentBlockSize = 0;

	bool b_will_oversample_ = true;
	std::vector<float> UpsampledBuffer;

	BiquadFilter ReconstructionLPA{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter ReconstructionLPB{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter ReconstructionLPC{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter ReconstructionLPD{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter InterpolationLPA{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter InterpolationLPB{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter InterpolationLPC{ BiquadFilter::PassFilterType::kLowpass };
	BiquadFilter InterpolationLPD{ BiquadFilter::PassFilterType::kLowpass };

	StatefulNonlinearity Nonlinearity;
	HysLoop HystLoop;

	int OVERSAMPLE_FACTOR = 1;
	float RECONSTRUCTION_HZ = 18000.0f;
	float INTERPOLATION_HZ = 18000.0f;

	bool mUseHysLoop = false;
};