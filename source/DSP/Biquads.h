//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#pragma once
#define _USE_MATH_DEFINES 1
#include <math.h>

/// <summary>
/// RBJ filter cookbook filters
/// </summary>
class BiquadFilter {
public:
	enum PassFilterType {
		kLowpass,
		kHighpass,
		kPeak,
		kLowShelf,
		kHighShelf,
		kNotch
	};
	BiquadFilter(PassFilterType Type) {
		FilterType = Type;
	}
	void setSampleRate(int sample_rate) {
		sampleRate = sample_rate;
		nyquistFreq = sampleRate * 0.5;
	}
	void getBlock(float* Output, float* Input, int num_samples) {
		for (int i = 0; i < num_samples; i++) {
			Output[i] = getNext(Input[i]);
		}
	}
	/// <summary>
	/// Update coefficients for filter. dB only used for peaking / cut filters
	/// </summary>
	/// <param name="F"></param>
	/// <param name="Q"></param>
	/// <param name="dB"></param>
	void calculateCoefficients(float F, float Q, float dB = 1) {

		if (F > nyquistFreq) F = (nyquistFreq - 1);

		if (sampleRate <= 0) {
			return;
		}

		float w0 = 2.0 * M_PI * (F / sampleRate);
		float cosw = cos(w0);
		float sinw = sin(w0);
		float alpha = sinw / (2.0 * Q);
		float A = pow(10, dB / 40.0);

		if (FilterType == kLowpass) {
			b1 = 1.0 - cosw;
			b0 = b1 / 2.0;
			b2 = b0;

			a0 = 1.0 + alpha;
			a1 = -2.0 * cosw;
			a2 = 1.0 - alpha;

		}

		if (FilterType == kHighpass) {
			b0 = (1.0 + cosw) / 2.0;
			b1 = -1.0 * (1.0 + cosw);
			b2 = b0;

			a0 = 1.0 + alpha;
			a1 = -2.0 * cosw;
			a2 = 1.0 - alpha;
		}

		if (FilterType == kPeak) {
			b0 = 1.0 + (alpha * A);
			b1 = -2.0 * cosw;
			b2 = 1.0 - (alpha * A);

			a0 = 1.0 + (alpha / A);
			a1 = -2 * cosw;
			a2 = 1.0 - (alpha / A);
		}

		if (FilterType == kHighShelf) {
			b0 = A * ((A + 1.0) + (A - 1.0) * cosw + (2.0 * sqrt(A) * alpha));
			b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw);
			b2 = A * ((A + 1.0) + (A - 1.0) * cosw - (2.0 * sqrt(A) * alpha));

			a0 = ((A + 1.0) - (A - 1.0) * cosw + (2.0 * sqrt(A) * alpha));
			a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw);
			a2 = (A + 1.0) - (A - 1.0) * cosw - 2.0 * sqrt(A) * alpha;
		}
		if (FilterType == kLowShelf) {
			b0 = A * ((A + 1.0) - (A - 1.0) * cosw + (2.0 * sqrt(A) * alpha));
			b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw);
			b2 = A * ((A + 1.0) - (A - 1.0) * cosw - (2.0 * sqrt(A) * alpha));

			a0 = ((A + 1.0) + (A - 1.0) * cosw + (2.0 * sqrt(A) * alpha));
			a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosw);
			a2 = ((A + 1.0) + (A - 1.0) * cosw - (2.0 * sqrt(A) * alpha));
		}
		if (FilterType == kNotch) {
			b0 = 1;
			b1 = -2.0 * cosw;
			b2 = 1;
			a0 = 1 + alpha;
			a1 = -2.0 * cosw;
			a2 = 1.0 - alpha;
		}

		// Normalise coefficients
		b0 /= a0;
		b1 /= a0;
		b2 /= a0;

		a1 /= a0;
		a2 /= a0;

	}

	float getNext(float spl) {
		float y = ((b0) * spl) + ((b1) * x1) + ((b2) * x2)
			- ((a1) * y1) - ((a2) * y2);


		// Update memory
		x2 = x1;
		x1 = spl;

		y2 = y1;
		y1 = y;

		return y;
	}
	/// <summary>
	/// Clears memory
	/// </summary>
	void reset() {
		x1 = 0;
		x2 = 0;
		y1 = 0;
		y2 = 0;
	}
private:
	int sampleRate = 48000;
	int blockSize = 0;
	float nyquistFreq = 24000;

	// Coefficients
	float b0 = 0;
	float b1 = 0;
	float b2 = 0;

	float a0 = 1;
	float a1 = 0;
	float a2 = 0;

	// Memory
	float x1 = 0;
	float x2 = 0;

	float y1 = 0;
	float y2 = 0;

	PassFilterType FilterType = kPeak;
};