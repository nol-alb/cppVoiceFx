#if !defined(__Lfo_hdr__)
#define __Lfo_hdr__

#define _USE_MATH_DEFINES
#include <math.h>

#include "ErrorDef.h"
#include "RingBuffer.h"

class CLfo
{
public:

	enum LfoParam_t
	{
		kAmplitude,
		kFrequency,
		kSampleRate,

		kNumParams
	};

	/**
	* Constructor for LFO class
	* @param fSampleRate : sample rate in Hz
	* @param fAmplitude : amplitude of LFO output
	* @param fFrequency : frequency of LFO in Hz
	* @return void
	*/
	CLfo(float fSampleRate = 0.0f, float fAmplitude = 0.0f, float fFrequency = 0.0f)
	{

        m_pWavetable= new CRingBuffer<float>(m_iWavetableSize);

		double angleDelta = (2.0f * M_PI) / (double)(m_iWavetableSize);
		double currentAngle = 0.0;

		for (int i = 0; i < m_iWavetableSize; i++)
		{
			float sample = static_cast<float>(std::sin(currentAngle));
			m_pWavetable->putPostInc(sample);
			currentAngle += angleDelta;
		}

		setSampleRate(fSampleRate);
		setFrequency(fFrequency);
		setAmplitude(fAmplitude);

	}

	~CLfo() 
	{
		delete m_pWavetable;
	};

	/**
	* Set parameters of LFO
	* @param param_t : enum of LfoParam_t type (kAmplitude, kFrequency, kSampleRate)
	* @param fValue : value to set parameter to
	* @return Error_t
	*/
	Error_t setParam(LfoParam_t param_t, float fValue)
	{
		switch (param_t)
		{
		case LfoParam_t::kAmplitude:
			return setAmplitude(fValue);
		case LfoParam_t::kFrequency:
			return setFrequency(fValue);
		case LfoParam_t::kSampleRate:
			return setSampleRate(fValue);
		}
		return Error_t::kFunctionInvalidArgsError;
	}

	/**
	* Returns desired LFO parameter value
	* @return Error_t
	*/
	float getParam(LfoParam_t param_t) const
	{
		switch (param_t)
		{
		case LfoParam_t::kAmplitude:
			return m_fAmplitude;
		case LfoParam_t::kFrequency:
			return m_fFrequency;
		case LfoParam_t::kSampleRate:
			return m_fSampleRate;
		}
	}

	/**
	* Resets wavetable read index. Equivalent to reseting phase of LFO
	* @return Error_t
	*/
	Error_t resetPhase()
	{
		m_fCurrentIndex = 0.0f;
		return Error_t::kNoError;
	}

	/**
	* Returns next value of LFO
	* @return float
	*/
	float process()
	{
		float fCurrentValue = m_pWavetable->get(m_fCurrentIndex);
		if ((m_fCurrentIndex += m_fTableDelta) > m_iWavetableSize)
			m_fCurrentIndex -= m_iWavetableSize;
		return m_fAmplitude * fCurrentValue;
	}

private:

	const int m_iWavetableSize = 1 << 9;
	CRingBuffer<float> *m_pWavetable = 0;
	float m_fTableDelta=0;
	float m_fCurrentIndex=0;
	float m_fAmplitude=0;
	float m_fFrequency=0;
	float m_fSampleRate=0;

	Error_t setFrequency(float fValue)
	{
		if (fValue < 0)
			return Error_t::kFunctionInvalidArgsError;
		m_fFrequency = fValue;
		m_fTableDelta = (m_fSampleRate == 0) ? 0 : m_iWavetableSize * fValue / m_fSampleRate;
		return Error_t::kNoError;
	}

	Error_t setAmplitude(float fValue)
	{
		if (fValue < 0)
			return Error_t::kFunctionInvalidArgsError;
		m_fAmplitude = fValue;
		return Error_t::kNoError;
	}

	Error_t setSampleRate(float fValue)
	{
		if (fValue < 0)
			return Error_t::kFunctionInvalidArgsError;
		m_fSampleRate = fValue;
		setFrequency(m_fFrequency);
		return Error_t::kNoError;
	}

};

#endif