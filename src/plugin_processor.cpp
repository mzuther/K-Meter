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
	if (DEBUG_FILTER)
	{
		DBG("********************************************************************************");
		DBG("** Debugging average filtering.  Please reset DEBUG_FILTER before committing! **");
		DBG("********************************************************************************");
	}

	pRingBufferInput = NULL;
	pRingBufferOutput = NULL;

	pAverageLevelFilteredRms = new AverageLevelFilteredRms(2, KMETER_BUFFER_SIZE);
	pMeterBallistics = new MeterBallistics(false, false);

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

	delete pAverageLevelFilteredRms;
	pAverageLevelFilteredRms = NULL;

	delete pMeterBallistics;
	pMeterBallistics = NULL;
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

	// make sure that ring buffer can hold at least KMETER_BUFFER_SIZE
	// samples and is large enough to receive a full block of audio
	nSamplesInBuffer = 0;
	unsigned int uRingBufferSize = (samplesPerBlock > KMETER_BUFFER_SIZE) ? samplesPerBlock : KMETER_BUFFER_SIZE;

	pRingBufferInput = new AudioRingBuffer(T("Input ring buffer"), 2, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
	pRingBufferInput->setCallbackClass(this);

	pRingBufferOutput = new AudioRingBuffer(T("Output ring buffer"), 2, uRingBufferSize, KMETER_BUFFER_SIZE, KMETER_BUFFER_SIZE);
}


void KmeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

	delete pRingBufferOutput;
	pRingBufferOutput = NULL;

	delete pRingBufferInput;
	pRingBufferInput = NULL;
}


void KmeterAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	// This is the place where you'd normally do the guts of your
	// plugin's audio processing...

	bool isStereo = (getNumInputChannels() > 1);
	int nNumSamples = buffer.getNumSamples();

	// convert stereo input to mono if "Mono" button has been pressed
	if (isStereo && makeMono)
	{
	  float* output_left = buffer.getSampleData(0);
	  float* output_right = buffer.getSampleData(1);

	  for (int i=0; i < nNumSamples; i++)
	  {
		 output_left[i] = 0.5f * (output_left[i] + output_right[i]);
		 output_right[i] = output_left[i];
	  }
	}

	pRingBufferInput->addSamples(buffer, 0, nNumSamples);

	nSamplesInBuffer += nNumSamples;
	nSamplesInBuffer %= KMETER_BUFFER_SIZE;

	pRingBufferOutput->copyToBuffer(buffer, 0, nNumSamples, KMETER_BUFFER_SIZE - nSamplesInBuffer);

	// In case we have more outputs than inputs, we'll clear any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
		buffer.clear(i, 0, nNumSamples);
}


void KmeterAudioProcessor::processBufferChunk(AudioSampleBuffer& buffer, const unsigned int uChunkSize, const unsigned int uBufferPosition, const unsigned int uProcessedSamples)
{
  unsigned int uPreDelay = uChunkSize / 2;
  bool isStereo = (getNumInputChannels() > 1);

  // copy ring buffer to determine average level (FIR filter already
  // adds delay of (uChunkSize / 2) samples)
  pAverageLevelFilteredRms->copyFromBuffer(*pRingBufferInput, 0, (int) getSampleRate());

  // determine peak level for uChunkSize samples (use pre-delay)
  fPeakLeft = pRingBufferInput->getMagnitude(0, uChunkSize, uPreDelay);

  // determine overflows for uChunkSize samples (use pre-delay)
  nOverflowsLeft = countContigousOverflows(pRingBufferInput, 0, uChunkSize, uPreDelay, bLastSampleOverLeft);

  // determine average level for uChunkSize samples
  fAverageLeft = pAverageLevelFilteredRms->getLevel(0);

  if (isStereo && !makeMono)
  {
	 // determine peak level for uChunkSize samples (use pre-delay)
	 fPeakRight = pRingBufferInput->getMagnitude(1, uChunkSize, uPreDelay);

	 // determine overflows for uChunkSize samples (use pre-delay)
	 nOverflowsRight = countContigousOverflows(pRingBufferInput, 1, uChunkSize, uPreDelay, bLastSampleOverRight);

	 // determine average level for uChunkSize samples (FIR filter
	 // already adds delay of (uChunkSize / 2) samples)
	 fAverageRight = pAverageLevelFilteredRms->getLevel(1);

	 // do not process levels below -80 dB (and prevent division by
	 // zero)
	 if ((fAverageLeft < 0.0001f) && (fAverageRight < 0.0001f))
		fCorrelation = 1.0f;
	 else
	 {
		float sum_of_product = 0.0f;
		float sum_of_squares_left = 0.0f;
		float sum_of_squares_right = 0.0f;

		// determine correlation for uChunkSize samples (use pre-delay)
		for (unsigned int uSample=0; uSample < uChunkSize; uSample++)
	 	{
		  float ringbuffer_left = pRingBufferInput->getSample(0, uSample, uPreDelay);
		  float ringbuffer_right = pRingBufferInput->getSample(1, uSample, uPreDelay);

		  sum_of_product += ringbuffer_left * ringbuffer_right;
		  sum_of_squares_left += ringbuffer_left * ringbuffer_left;
		  sum_of_squares_right += ringbuffer_right * ringbuffer_right;
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

  fTimeFrame = (float) getSampleRate() / (float) uChunkSize;
  pMeterBallistics->update(isStereo ? 2 : 1, fTimeFrame, fPeakLeft, fPeakRight, fAverageLeft, fAverageRight, fCorrelation, nOverflowsLeft, nOverflowsRight);

  sendChangeMessage(this);

  // To hear the audio source after average filtering, simply set
  // DEBUG_FILTER to 1.  Please remember to disable this setting
  // before commiting your changes.
  if (DEBUG_FILTER)
  {
    pAverageLevelFilteredRms->copyToBuffer(*pRingBufferOutput, 0, uChunkSize);
  }
  else
  {
	 int nNumInputChannels = getNumInputChannels();

	 AudioSampleBuffer TempAudioBuffer = AudioSampleBuffer(nNumInputChannels, uChunkSize);
	 pRingBufferInput->copyToBuffer(TempAudioBuffer, 0, uChunkSize, 0);
	 pRingBufferOutput->addSamples(TempAudioBuffer, 0, uChunkSize);
  }
}


int KmeterAudioProcessor::countContigousOverflows(AudioRingBuffer* ring_buffer, const unsigned int channel, const unsigned int length, const unsigned int pre_delay, bool& bLastSampleOver)
{
	int nOverflows = 0;

	for (unsigned int uSample=0; uSample < length; uSample++)
	{
		float fSampleValue = fabsf(ring_buffer->getSample(channel, uSample, pre_delay));

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


void KmeterAudioProcessor::convertMono(const bool bMono)
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
