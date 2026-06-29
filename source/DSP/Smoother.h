//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include <math.h>

/// <summary>
	/// Simple smoother for general purpose smoothing
	/// </summary>
class ValueSmoother {
public:
	double getSmoothedValue(double target) {
		currentTarget = target;
		double difference = currentTarget - lastValue;
		// Snap to target if the difference is very small
		if (fabsf(difference) < 1e-9f) {
			lastValue = target;
		}
		else {
			lastValue += difference * smoothFactor;
		}
		return lastValue;
	}
	void setSmoothTime(double time_ms, int sample_rate) {
		if (time_ms > 0) {
			smoothFactor = 1.0f - expf(-2.0f * 3.141592653589f / (time_ms * 0.001f * sample_rate));
		}
		else {
			smoothFactor = 1.0f; // No smoothing
		}
	}
	void reset() {
		lastValue = 0;
		currentTarget = 0;
	}
	/// <summary>
	/// Instantly set the current value to the 
	/// target value. Used to prevent drifting 
	/// etc after preset loading
	/// </summary>
	void collapse() {
		lastValue = currentTarget;
	}
	void collapseTo(double target) {
		lastValue = target;
		currentTarget = target;
	}

private:
	double lastValue = 0;
	double smoothFactor = 0.005;
	double currentTarget = 0;
};