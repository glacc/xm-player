#pragma once

#include <SFML/Audio.hpp>

namespace Test
{
	class SoundStream : public sf::SoundStream
	{
	private:
		const unsigned int channelCount = 2;
		const unsigned int sampleRate = 44100;
		// 25ms
		const unsigned int defaultBufferSize = sampleRate / 40;
		unsigned int bufferSize = defaultBufferSize;

		sf::Int16 *soundBuffer;

	public:
		unsigned int frequency;
		enum Waveform
		{
			Sine,
			Square,
			Triangle,
			Sawtooth,
			Noise
		};
		Waveform waveform;
		unsigned char volume;

		SoundStream(Waveform newWaveform = Sine, unsigned int newFrequency = 500, unsigned char newVolume = 255);

		~SoundStream();

	private:
		float periodPos = 0.0F;

		bool onGetData(Chunk &data) override;

		void onSeek(sf::Time timeOffset) override;
	};
}
