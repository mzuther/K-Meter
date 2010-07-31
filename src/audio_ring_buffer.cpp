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

#include "audio_ring_buffer.h"

AudioRingBuffer::AudioRingBuffer(const unsigned int channels, const unsigned int length, const unsigned int pre_delay)
{
  jassert(channels > 0);
  jassert(length > 0);

  this->clearCallbackClass();

  uChannels = channels;
  uLength = length;
  uPreDelay = pre_delay;
  uTotalLength = uLength + uPreDelay;

  pAudioData = (float*) malloc(uChannels * uTotalLength * sizeof(float));

  uCurrentPosition = 0;
  uSamplesInBuffer = 0;
  uChannelOffset = new unsigned int[uChannels];

  for (unsigned int uChannel=0; uChannel < uChannels; uChannel++)
	 uChannelOffset[uChannel] = uChannel * uTotalLength;

  this->clear();
}


AudioRingBuffer::~AudioRingBuffer()
{
  delete [] uChannelOffset;
  free(pAudioData);
}


void AudioRingBuffer::clear()
{
  uCurrentPosition = 0;

  for (unsigned int uChannel=0; uChannel < uChannels; uChannel++)
	 for (unsigned int uSample=0; uSample < uTotalLength; uSample++)
		pAudioData[uSample + uChannelOffset[uChannel]] = 0.0f;
}


float AudioRingBuffer::getSample(const unsigned int channel, const unsigned int relative_position, const unsigned int pre_delay)
{
  jassert(channel < uChannels);
  jassert(relative_position <= uLength);
  jassert(pre_delay <= uPreDelay);

  unsigned int uPosition = uCurrentPosition - relative_position;

  if (pre_delay)
	 uPosition -= pre_delay;

  uPosition += uTotalLength; // make sure "uPosition" is positive
  uPosition %= uTotalLength;

  return pAudioData[uPosition + uChannelOffset[channel]];
}


unsigned int AudioRingBuffer::addSamples(AudioSampleBuffer& source, const unsigned int sourceStartSample, const unsigned int numSamples)
{
  if (numSamples <= 0)
	 return 0;

  jassert(source.getNumChannels() >= (int) uChannels);
  jassert((sourceStartSample + numSamples) <= (unsigned int) source.getNumSamples());

  unsigned int uSamplesLeft = numSamples;
  unsigned int uSamplesFinished = 0;
  unsigned int uProcessedSamples = 0;

  while (uSamplesLeft > 0)
  {
  	 unsigned int uSamplesToCopy = uLength - uSamplesInBuffer;
  	 unsigned int uSamplesToCopy_2 = uTotalLength - uCurrentPosition;

  	 if (uSamplesToCopy_2 < uSamplesToCopy)
		uSamplesToCopy = uSamplesToCopy_2;

  	 if (uSamplesToCopy > uSamplesLeft)
  		uSamplesToCopy = uSamplesLeft;

	 for (unsigned int uChannel=0; uChannel < uChannels; uChannel++)
	 {
		memcpy(pAudioData + uCurrentPosition + uChannelOffset[uChannel], source.getSampleData(uChannel, sourceStartSample + uSamplesFinished), sizeof(float) * uSamplesToCopy);
	 }

	 uSamplesInBuffer += uSamplesToCopy;

	 uProcessedSamples += uSamplesToCopy;
	 bool bBufferFull = (uSamplesInBuffer == uLength);
	 uSamplesInBuffer %= uLength;

	 uCurrentPosition += uSamplesToCopy;
	 uCurrentPosition %= uTotalLength;

  	 uSamplesLeft -= uSamplesToCopy;

	 if (bBufferFull)
	 {
	 	triggerFullBuffer(source, sourceStartSample + uSamplesFinished, uProcessedSamples);
		uProcessedSamples = 0;
	 }

	 uSamplesFinished += uSamplesToCopy;
  }

  return uProcessedSamples;
}


void AudioRingBuffer::copyToBuffer(AudioSampleBuffer& destination, const unsigned int destStartSample, const unsigned int numSamples, const unsigned int pre_delay)
{
  if (numSamples <= 0)
	 return;

  jassert(destination.getNumChannels() >= (int) uChannels);
  jassert(numSamples <= uLength);
  jassert(pre_delay <= uPreDelay);
  jassert((destStartSample + numSamples) <= (unsigned int) destination.getNumSamples());

  unsigned int uSamplesLeft = numSamples;
  unsigned int uSamplesFinished = 0;

  unsigned int uStartPosition = uCurrentPosition - numSamples - pre_delay;
  uStartPosition += uTotalLength; // make sure "uStartPosition" is positive
  uStartPosition %= uTotalLength;

  while (uSamplesLeft > 0)
  {
  	 unsigned int uSamplesToCopy = uTotalLength - uStartPosition;

  	 if (uSamplesToCopy > uSamplesLeft)
  		uSamplesToCopy = uSamplesLeft;

	 for (unsigned int uChannel=0; uChannel < uChannels; uChannel++)
	 {
		memcpy(destination.getSampleData(uChannel, destStartSample + uSamplesFinished), pAudioData + uStartPosition + uChannelOffset[uChannel], sizeof(float) * uSamplesToCopy);
	 }

	 uStartPosition += uSamplesToCopy;
	 uStartPosition %= uTotalLength;

  	 uSamplesLeft -= uSamplesToCopy;
	 uSamplesFinished += uSamplesToCopy;
  }
}


float AudioRingBuffer::getMagnitude(const unsigned int channel, const unsigned int numSamples, const unsigned int pre_delay)
{
  float fMagnitude = 0.0f;

  for (unsigned int uSample=0; uSample < numSamples; uSample++)
  {
	 float fSampleValue = fabsf(getSample(channel, uSample, pre_delay));

	 if (fSampleValue > fMagnitude)
		fMagnitude = fSampleValue;
  }

  return fMagnitude;
}


float AudioRingBuffer::getRMSLevel(const unsigned int channel, const unsigned int numSamples, const unsigned int pre_delay)
{
  double dRunningSum = 0.0;

  for (unsigned int uSample=0; uSample < numSamples; uSample++)
  {
	 float fSampleValue = getSample(channel, uSample, pre_delay);
	 dRunningSum += fSampleValue * fSampleValue;
  }

  return (float) sqrt(dRunningSum / numSamples);
}


void AudioRingBuffer::setCallbackClass(KmeterAudioProcessor* callback_class)
{
  pCallbackClass = callback_class;
}


void AudioRingBuffer::clearCallbackClass()
{
  pCallbackClass = NULL;
}


void AudioRingBuffer::triggerFullBuffer(AudioSampleBuffer& buffer, const unsigned uBufferPosition, const unsigned uProcessedSamples)
{
  if (pCallbackClass)
	 pCallbackClass->processBufferChunk(buffer, uBufferPosition, uProcessedSamples);
}
