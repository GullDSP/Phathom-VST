//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------
#pragma once
#include <math.h>

#ifndef E_PI 
#define E_PI 3.14159265358979323846
#endif 

/// <summary>
/// A one pole Lowpass filter
/// </summary>
class OnePoleLowpass {
public:
    void setSampleRate(int sample_rate) {
        sampleRate = sample_rate;
        nyquist = sampleRate * 0.5;
    }

    /// <summary>
    /// Set center
    /// </summary>
    void setCenterHz(float hz) {
        if (hz >= nyquist) hz = nyquist - 100;
        double tan_val = tan((E_PI * hz) / sampleRate);
        mCoefficient = tan_val / (1.0 + tan_val);
    }

    void setCoefficient(float value) {
        mCoefficient = value;
    }

    inline float getNext(float x) {
        float y = z1 + mCoefficient * (x - z1);
        z1 = y;
        return y;
    }
    /// <summary>
    /// Reset memory
    /// </summary>
    void resetState() {
        z1 = 0.0f;

    }
    float getPhaseDelaySpls() {
        if (mCoefficient == 0) {
            return 0;
        }
        return (1.0 - mCoefficient) / mCoefficient;
    }
    void getBlock(float* out_buffer, float* in_buffer, int num_samples) {
        for (int i = 0; i < num_samples;i++) {
            out_buffer[i] = getNext(in_buffer[i]);
        }
    }
private:
    int sampleRate = 48000;
    float mCoefficient = 0.0f;
    float z1 = 0.0f;
    float nyquist = 22000;
};
/// <summary>
/// A one pole Lowpass filter
/// </summary>
class OnePoleHighpass {
public:
    void setSampleRate(int sample_rate) {
        sampleRate = sample_rate;
        nyquist = 0.5 * sampleRate;
    }

    /// <summary>
    /// Set center
    /// </summary>
    void setCenterHz(float hz) {
        if (hz >= nyquist) hz = nyquist - 100;

        double tan_val = tan((E_PI * hz) / sampleRate);
        mCoefficient = 1.0 / (1.0 + tan_val);
    }
    
    void setCoefficient(float value) {
        mCoefficient = value;

    }

    inline float getNext(float x) {
        float y = (x - x1) + z1;
        y = y * mCoefficient;
        z1 = y;
        x1 = x;
        return y;
    }

    void getBlock(float* out_buffer, float* in_buffer, int num_samples) {
        for (int i = 0; i < num_samples;i++) {
            out_buffer[i] = getNext(in_buffer[i]);
        }
    }
    /// <summary>
    /// Reset memory
    /// </summary>
    void resetState() {
        z1 = 0.0f;
        x1 = 0;
    }
    float getPhaseDelaySpls() {
        return (1.0 - mCoefficient) / mCoefficient;
    }

private:
    int sampleRate = 48000;
    float mCoefficient = 0.0f;
    float z1 = 0.0f;
    float x1 = 0;
    float nyquist = 22000;
};

/// <summary>
/// A one pole allpass filter
/// </summary>
class OnePoleAllpass {
public:
    void setSampleRate(int sample_rate) {
        sampleRate = sample_rate;
        nyquist = 0.5 * sampleRate;
    }

    /// <summary>
    /// Set center
    /// </summary>
    void setCenterHz(float hz) {
        if (hz >= nyquist) hz = nyquist - 100;
        double tan_val = tan((E_PI * hz) / sampleRate);
        mCoefficient = (tan_val - 1.0) / (tan_val + 1.0);
    }

    /// <summary>
    /// Best 0.3 - 0.9
    /// </summary>
    /// <param name="value"></param>
    void setCoefficient(float value) {
        mCoefficient = value;
    }
    void getBlock(float* out_buffer, float* in_buffer, int num_samples) {
        for (int i = 0; i < num_samples;i++) {
            out_buffer[i] = getNext(in_buffer[i]);
        }
    }
    inline float getNext(float x) {

        float y = x * mCoefficient + z1;
        z1 = x - y * mCoefficient;
        return y;
    }
    /// <summary>
    /// Reset memory
    /// </summary>
    void resetState() {
        z1 = 0.0f;

    }
    float getCoefficient() {
        return mCoefficient;
    }
    float getPhaseDelaySpls() {
        if (mCoefficient == 0) {
            return 0;
        }

        return (1.0 - mCoefficient) / mCoefficient;
    }

private:
    int sampleRate = 48000;
    float mCoefficient = 0.0f;
    float z1 = 0.0f;
    float nyquist = 22000;

};