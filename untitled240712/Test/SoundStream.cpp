#include <memory>
//#include <numbers>

#include "SoundStream.hpp"

using namespace Test;

SoundStream::SoundStream(Waveform newWaveform, unsigned int newFrequency, unsigned char newVolume)
{
	frequency = newFrequency;
	waveform = newWaveform;
	volume = newVolume;

	periodPos = 0.0F;
	bufferSize = defaultBufferSize;
	soundBuffer = (sf::Int16 *)malloc(sizeof(sf::Int16) * channelCount * bufferSize);
	initialize(channelCount, sampleRate);
}

SoundStream::~SoundStream()
{
	if (soundBuffer != NULL)
		free(soundBuffer);
}

/*
void PatternViewer::SoundStream::ProcessNextBuffer()
{

}
*/

const double pi = std::atan(1) * 4.0;

bool SoundStream::onGetData(Chunk &data)
{
	float period = (float)sampleRate / frequency;
	unsigned int i = 0;
	while (i < bufferSize)
	{
		float raw = 0.0F;
		switch (waveform)
		{
		case Sine:
			raw = sin(2.0 * pi * periodPos / period);
			break;
		case Square:
			raw = periodPos < period / 2.0F ? 1.0F : -1.0F;
			break;
		case Triangle:
			raw = periodPos < period / 2.0F ? (periodPos / period * 2.0F - 1.0F) : ((-periodPos / period * 2.0F + 2.0F) - 1.0F);
			break;
		case Sawtooth:
			raw = periodPos / period * 2.0F - 1.0F;
			break;
		}
		raw = raw * ((int)volume << 7);
		soundBuffer[i] = raw;

		periodPos += 1.0F;
		periodPos = fmod(periodPos, period);

		i ++;
	}

	data.sampleCount = bufferSize;
	data.samples = soundBuffer;

	return true;
}

void SoundStream::onSeek(sf::Time timeOffset) {}
