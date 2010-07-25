/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010 Martin Zuther (http://www.mzuther.de/)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Thank you for using free software!

---------------------------------------------------------------------------- */

#include "plugin_processor.h"
#include "plugin_editor.h"


//==============================================================================
KmeterAudioProcessor::KmeterAudioProcessor()
{
	pRingBuffer = new AudioSampleBuffer(2, KMETER_BUFFER_SIZE);
	pRingBuffer->clear();
	nRingBufferPosition = 0;

	pAverageLevelFilteredRms = new AverageLevelFilteredRms(pRingBuffer, KMETER_BUFFER_SIZE);
	pMeterBallistics = new MeterBallistics(false, false);

	isStereo = false;
	makeMono = false;

	fTimeFrame = 0.0f;

	fPeakLeft = 0.0f;
	fPeakRight = 0.0f;
	fAverageLeft = 0.0f;
	fAverageRight = 0.0f;
	fCorrelation = 0.0f;
	nOverflowsLeft = 0;
	nOverflowsRight = 0;

	bLastSampleOverLeft = false;
	bLastSampleOverRight = false;
}

KmeterAudioProcessor::~KmeterAudioProcessor()
{
	removeAllChangeListeners();

	delete pRingBuffer;
	delete pAverageLevelFilteredRms;
	delete pMeterBallistics;
}

//==============================================================================
const String KmeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int KmeterAudioProcessor::getNumParameters()
{
    return 0;
}

float KmeterAudioProcessor::getParameter(int index)
{
    return 0.0f;
}

void KmeterAudioProcessor::setParameter(int index, float newValue)
{
}

const String KmeterAudioProcessor::getParameterName(int index)
{
    return String::empty;
}

const String KmeterAudioProcessor::getParameterText(int index)
{
    return String::empty;
}

const String KmeterAudioProcessor::getInputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

const String KmeterAudioProcessor::getOutputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

bool KmeterAudioProcessor::isInputChannelStereoPair(int index) const
{
    return true;
}

bool KmeterAudioProcessor::isOutputChannelStereoPair(int index) const
{
    return true;
}

bool KmeterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool KmeterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

int KmeterAudioProcessor::getNumPrograms()
{
    return 0;
}

int KmeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KmeterAudioProcessor::setCurrentProgram(int index)
{
}

const String KmeterAudioProcessor::getProgramName(int index)
{
    return String::empty;
}

void KmeterAudioProcessor::changeProgramName(int index, const String& newName)
{
}

//==============================================================================
void KmeterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void KmeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void KmeterAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	// This is the place where you'd normally do the guts of your
	// plugin's audio processing...

	// To hear the audio source after average filtering, make sure that
	// your audio host's buffer matches the size of KMETER_BUFFER_SIZE.
	// Only then set DEBUG_FILTER to 1.  This will also disable metering.
	#define DEBUG_FILTER 0

	if (DEBUG_FILTER)
	{
		DBG("Debugging average filtering.  Please reset DEBUG_FILTER when you're done!");

		pRingBuffer->copyFrom(0, 0, buffer, 0, 0, KMETER_BUFFER_SIZE);
		fAverageLeft = pAverageLevelFilteredRms->getLevel(0, (int) getSampleRate());
		buffer.copyFrom(0, 0, pAverageLevelFilteredRms->getProcessedSamples(0), KMETER_BUFFER_SIZE);

		if (getNumInputChannels() > 1)
		{
			pRingBuffer->copyFrom(1, 0, buffer, 1, 0, KMETER_BUFFER_SIZE);
			fAverageRight = pAverageLevelFilteredRms->getLevel(1, (int) getSampleRate());
			buffer.copyFrom(1, 0, pAverageLevelFilteredRms->getProcessedSamples(1), KMETER_BUFFER_SIZE);
		}

		// clear remaining output channels
		for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
			buffer.clear(i, 0, buffer.getNumSamples());

		return;
	}

	bool isStereo = (getNumInputChannels() > 1);

	// convert stereo input to mono if "Mono" button has been pressed
	if (isStereo && makeMono)
	{
	  float* output_left = buffer.getSampleData(0);
	  float* output_right = buffer.getSampleData(1);

	  for (int i=0; i < buffer.getNumSamples(); i++)
	  {
		 output_left[i] = 0.5f * (output_left[i] + output_right[i]);
		 output_right[i] = output_left[i];
	  }
	}

	for (int nSourcePosition=0; nSourcePosition < buffer.getNumSamples(); nSourcePosition++)
	{
		pRingBuffer->copyFrom(0, nRingBufferPosition, buffer, 0, nSourcePosition, 1);
		if (isStereo)
			pRingBuffer->copyFrom(1, nRingBufferPosition, buffer, 1, nSourcePosition, 1);

		nRingBufferPosition = (nRingBufferPosition + 1) % KMETER_BUFFER_SIZE;

		if (nRingBufferPosition == (KMETER_BUFFER_SIZE - 1))
		{
			fPeakLeft = pRingBuffer->getMagnitude(0, 0, KMETER_BUFFER_SIZE);
			fAverageLeft = pAverageLevelFilteredRms->getLevel(0, getSampleRate());
			nOverflowsLeft = countContigousOverflows(pRingBuffer, 0, bLastSampleOverLeft);

			if (isStereo)
			{
				fPeakRight = pRingBuffer->getMagnitude(1, 0, KMETER_BUFFER_SIZE);
				fAverageRight = pAverageLevelFilteredRms->getLevel(1, getSampleRate());
				nOverflowsRight = countContigousOverflows(pRingBuffer, 1, bLastSampleOverRight);

				// do not process levels below -80 dB (and prevent division by zero)
				if ((fAverageLeft < 0.0001f) && (fAverageRight < 0.0001f))
					fCorrelation = 1.0f;
				else
				{
					float sum_of_product = 0.0f;
					float sum_of_squares_left = 0.0f;
					float sum_of_squares_right = 0.0f;

					float* ringbuffer_left = pRingBuffer->getSampleData(0);
					float* ringbuffer_right = pRingBuffer->getSampleData(1);

					for (int i=0; i < KMETER_BUFFER_SIZE; i++)
					{
						sum_of_product += ringbuffer_left[i] * ringbuffer_right[i];
						sum_of_squares_left += ringbuffer_left[i] * ringbuffer_left[i];
						sum_of_squares_right += ringbuffer_right[i] * ringbuffer_right[i];
					}

					fCorrelation = sum_of_product / sqrt(sum_of_squares_left * sum_of_squares_right);
				}
			}
			else
			{
				fPeakRight = fPeakLeft;
				fAverageRight = fAverageLeft;
				nOverflowsRight = nOverflowsLeft;

				fCorrelation = 1.0f;
			}

			fTimeFrame = (float) getSampleRate() / (float) KMETER_BUFFER_SIZE;
			pMeterBallistics->update(isStereo ? 2 : 1, fTimeFrame, fPeakLeft, fPeakRight, fAverageLeft, fAverageRight, fCorrelation, nOverflowsLeft, nOverflowsRight);

			sendChangeMessage(this);
		}
	}

	// In case we have more outputs than inputs, we'll clear any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
		buffer.clear(i, 0, buffer.getNumSamples());
}

int KmeterAudioProcessor::countContigousOverflows(const AudioSampleBuffer* buffer, int channel, bool& bLastSampleOver)
{
	int nOverflows = 0;
	
	float* samples = buffer->getSampleData(channel);
	for (int n=0; n < buffer->getNumSamples(); n++)
	{
		float fSampleValue = abs(samples[n]);

		if (fSampleValue > 1.0f)
		{
			nOverflows++;
			bLastSampleOver = true;
		}
		else if (fSampleValue == 1.0f)
		{
			if (bLastSampleOver)
				nOverflows++;

			bLastSampleOver = true;
		}
		else
			bLastSampleOver = false;
	}

	return nOverflows;
}

MeterBallistics* KmeterAudioProcessor::getLevels()
{
	return pMeterBallistics;
}

void KmeterAudioProcessor::convertMono(bool bMono)
{
	makeMono = bMono;
}

//==============================================================================
AudioProcessorEditor* KmeterAudioProcessor::createEditor()
{
	return new KmeterAudioProcessorEditor(this);
}

//==============================================================================
void KmeterAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void KmeterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KmeterAudioProcessor();
}
