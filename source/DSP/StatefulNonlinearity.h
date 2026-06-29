//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include "math.h"
#include "OnePoleFilters.h"
#include "Biquads.h"
#include "../DSP/config.h"

namespace NONLINEARITY_CONFIG {
	constexpr float DRIVE_MULTIPLIER = 190;
}


/// <summary>
/// Sort of hysteresis saturator with oversampling
/// </summary>
class StatefulNonlinearity {


public:
	void setSampleRate(int sample_rate, int block_size) {
		sampleRate = sample_rate;
		blockSize = block_size;

		DC.setSampleRate(sampleRate);
		DirDC.setSampleRate(sampleRate);
		MorphLP.setSampleRate(sampleRate);

		DC.setCenterHz(15);
		DirDC.setCenterHz(15);
		MorphLP.setCenterHz(3500);;
		DriveSmoother.setSmoothTime(50, sampleRate);
		// Set coefficient. Response time of hysteresis in ms is 1000 / (coefficient * sample rate)
		morph_coefficient_ = 0.28f * (44100.0f / ((float)sampleRate)); // approx 7 sample transition at 44khz

		// Keep feedback consistent between sample rates
		float target_t = 0.0000584f;
		feedback_amount_ = expf(-1.0f / (sampleRate * target_t));

	}
	void prepareToPlay() {
		collapseSmoothers();
		resetDSP();
	}
	void reset() {
		DriveSmoother.reset();
		resetDSP();
	}

	void resetDSP() {
		DC.resetState();
		DirDC.resetState();
		MorphLP.resetState();
		last_out_ = 0;
		last_in_ = 0;
		morph_direction_ = 0.0;
		morph_state_ = 0;
	}
	void collapseSmoothers() {
		DriveSmoother.collapseTo(mDrive);
	}
	void setDrive(float value) {
		mDrive = value;
	}
	void setResponse(float value) {
		// TODO implement this
		mResponse = value;
	}
	float getNext(float in) {

		morph_direction_ = 0.0f;
		float delta = in - last_in_;
		
		if (delta > 0) {
			morph_direction_ = 1.0f;
		}
		else if (delta < 0) {
			morph_direction_ = -1.0f;
		}
		// Need to remove dc offset from the signal due to nature of bass signals
		float morph_direction_filtered = DirDC.getNext(morph_direction_);

		// As soon as direction changes, the mix between the two nonlinearities is 'pinged' and starts floating towards the other.
		// This is a simple way to prevent discontinuities without maths I don't understand. Otherwise, the functions must have identical outputs
		// at the input value the inflection occurs at. This does mean that there is a frequency dependent element however, if the coefficient is too low,
		// then the mix will potentially not have time to move across before the next direction change. This coefficient is sample rate dependent

		morph_state_ = morph_state_ + (morph_direction_filtered * morph_coefficient_);
		if (morph_state_ > 1.0f) morph_state_ = 1.0f;
		if (morph_state_ < -1.0f) morph_state_ = -1.0f;
		morph_state_ = MorphLP.getNext(morph_state_);

		float if_rising = in;

		if (if_rising > 0.95f) if_rising = 0.95f;
		if (if_rising < -0.75f) if_rising = -0.75f;
		

		float if_falling = in + (feedback_amount_ * last_out_);
		if_falling = if_falling / (1.0f + fabsf(if_falling * 1.5f));
		if_falling = (if_falling / (1 + if_falling * if_falling));

	
		float mix = 0.5f * (morph_state_ + 1.0f);
		float y = (mix * if_rising) + ((1.0f - mix) * if_falling);

		y = DC.getNext(y);

		last_in_ = in;
		last_out_ = y;
		return y;
	}

	void getBlock(float* Buffer, int num_samples) {

		constexpr float compensation_factor = 2.0f;
		float mDriveNow = 0;
		for (int i = 0; i < num_samples; i++) {
			mDriveNow = DriveSmoother.getSmoothedValue(mDrive);
			float spl = Buffer[i];
			mDriveNow = mDriveNow * mDriveNow;
			Buffer[i] = getNext(spl * mDriveNow * NONLINEARITY_CONFIG::DRIVE_MULTIPLIER);
			Buffer[i] *= (1.0f / (1.0f + (mDriveNow * compensation_factor)));
		}

	}
private:
	int sampleRate = 48000;
	int blockSize = 0;

	float mDrive = 0.5;
	float mResponse = 0.5;
	float last_in_ = 0;
	float last_out_ = 0;
	float morph_direction_ = 0.0;
	float morph_state_ = 0;
	float morph_coefficient_ = 0.3;
	float feedback_amount_ = 0.75;
	OnePoleHighpass DC;
	OnePoleHighpass DirDC;

	OnePoleLowpass MorphLP;
	ValueSmoother DriveSmoother;



};