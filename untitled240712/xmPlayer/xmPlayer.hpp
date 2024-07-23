#pragma once
//Version 240714

#ifndef _XMPLAYER_HPP_
#define _XMPLAYER_HPP_

//#define _SDL2_GCC
//#define _SDL_NUGET
#define _SFML

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <algorithm>
#include <time.h>

// Another abandoned mixer and doesn't support scope data
//#define _MACRO_MIXER

#ifdef _SDL2_GCC
#undef _SDL_NUGET
#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_audio.h>
#define _SDL2
#undef _SFML
#endif
#ifdef _SDL_NUGET
#include <SDL.h>
#include <SDL_stdinc.h>
#include <SDL_audio.h>
#define _SDL2
#undef _SFML
#endif
#ifdef _SFML
#include <SFML/Audio.hpp>
#endif

#define SMP_RATE 44100
#define BUFFER_SIZE 1024

namespace Glacc
{
	class XMPlayer
	{
		/* Struct Definitions */

	public:
		struct Instrument
		{
			int16_t sampleNum;
			int16_t fadeOut;
			int16_t sampleMap[96];
			int16_t volEnvelops[24];
			int16_t panEnvelops[24];
			char name[23];
			int8_t volPoints;
			int8_t panPoints;
			uint8_t volSustainPt;
			uint8_t volLoopStart;
			uint8_t volLoopEnd;
			uint8_t panSustainPt;
			uint8_t panLoopStart;
			uint8_t panLoopEnd;
			int8_t volType;
			int8_t panType;
			int8_t vibratoType;
			int8_t vibratoSweep;
			int8_t vibratoDepth;
			int8_t vibratoRate;
		};

		struct Sample
		{
			int32_t length;
			int32_t loopStart;
			int32_t loopLength;
			int8_t *data;
			int8_t volume;
			int8_t fineTune;
			int8_t type;
			int8_t relNote;
			uint8_t origInst;
			bool is16Bit;
			uint8_t pan;
			char name[23];
		};

		struct Channel
		{
			int8_t note;
			int8_t relNote;
			int8_t noteArpeggio;
			uint8_t lastNote;
			int8_t fineTune;

			uint8_t instrument;
			uint8_t nextInstrument;
			uint8_t lastInstrument;

			uint8_t volCmd;
			uint8_t volPara;
			int16_t volume;
			int16_t lastVol;
			int16_t samplePlaying;
			int16_t sample;
			int16_t pan;
			int16_t panFinal;
			int16_t volEnvelope;
			int16_t panEnvelope;
			int16_t delay;
			int16_t RxxCounter;
			int16_t fadeTick;
			uint8_t effect;
			uint8_t parameter;

			uint8_t vibratoPos;
			uint8_t vibratoAmp;
			uint8_t vibratoType;
			bool volVibrato;
			uint8_t tremorPos;
			uint8_t tremorAmp;
			uint8_t tremorType;

			uint8_t slideUpSpd;
			uint8_t slideDnSpd;
			uint8_t slideSpd;
			uint8_t vibratoPara;
			uint8_t tremoloPara;
			int8_t volSlideSpd;
			uint8_t fineProtaUp;
			uint8_t fineProtaDn;
			uint8_t fineVolUp;
			uint8_t fineVolDn;
			int8_t globalVolSlideSpd;
			int8_t panSlideSpd;
			uint8_t retrigPara;
			uint8_t tremorPara;
			uint8_t EFProtaUp;
			uint8_t EFProtaDn;

			int8_t tremorTick;
			bool tremorMute;

			uint8_t autoVibPos;
			uint8_t autoVibSweep;
			int8_t envFlags;

			bool instTrig;
			bool LxxEffect;
			bool active;
			bool fading;
			bool keyOff;

			//Current playing sample
			bool muted;
			int8_t loop;
			int8_t loopType;
			bool is16Bit;
			int16_t period;
			int16_t targetPeriod;
			int16_t periodOfs;
			int16_t startCount;
			int16_t endCount;
			int32_t smpLeng;
			int32_t loopStart;
			int32_t loopEnd;
			int32_t loopLeng;
			int32_t delta;
			float facL;
			float facR;
			int8_t *data;
			int8_t *dataPrev;
			int32_t pos;
			int32_t posL16;
			int32_t volTargetInst;
			int32_t volFinalInst;
			int32_t volTargetL;
			int32_t volTargetR;
			int32_t volFinalL;
			int32_t volFinalR;
			int32_t volRampSpdL;
			int32_t volRampSpdR;
			int32_t volRampSpdInst;
			int32_t prevSmp;
			int32_t endSmp;
		};

		struct Note
		{
			uint8_t note;
			uint8_t instrument;
			uint8_t volCmd;
			uint8_t effect;
			uint8_t parameter;
		};

		struct Stat
		{
			char *songName;
			char *trackerName;
			int16_t trackerVersion;
			int16_t songLength;
			int16_t numOfPatterns;
			int16_t numOfInstruments;
			int16_t numOfChannels;

			uint8_t speed;
			uint8_t tempo;

			int16_t pos;
			int16_t row;
			uint8_t pattern;
			int16_t  patternLen;

			uint8_t *patternOrder;

			Channel *channels;
			Instrument *instruments;
			Sample *samples;
		};

		bool scopeEnabled = true;
		struct Scope
		{
			int8_t *scopeData = NULL;
			int sizePerCh;
			int numOfCh;
		};

		Scope scopes;

	private:
		struct EnvInfo
		{
			uint8_t value;
			int16_t maxPoint;
		};

		/* Variable Definitions */

#ifdef _SDL2
		SDL_AudioSpec audioSpec, actualSpec;
		SDL_AudioDeviceID deviceID = 0;
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

		char songName[21];
		char trackerName[21];
		int16_t trackerVersion;

		bool loop;
		bool stereo;
		double amplifier = 1;
		bool ignoreF00 = false;
		int8_t panMode = 0;

		int bufferSize;
		int sampleRate;
		bool isPlaying = false;
		uint8_t masterVolume;

		bool useAmigaFreqTable;
		int16_t numOfChannels;
		int16_t numOfPatterns;
		int16_t numOfInstruments;
		uint8_t orderTable[256];

		int16_t songLength;
		int16_t rstPos;
		int16_t defaultTempo;
		int16_t defaultSpd;

		uint8_t *songData;
		uint8_t *patternData;
		int8_t *sampleData;
		//int16_t *sndBuffer[2];

		int32_t totalPatSize;
		int32_t totalInstSize;
		int32_t totalSampleSize;
		int32_t totalSampleNum;
		int32_t patternAddr[256];
		Instrument *instruments;
		Sample *samples;
		int16_t *sampleStartIndex;
		int32_t *sampleHeaderAddr;

		int16_t tick, curRow, curPos;
		int16_t patBreak, patJump, patDelay;
		int16_t patRepeat, repeatPos, repeatTo;
		uint8_t speed, tempo, globalVol;
		int32_t sampleToNextTick;
		double timer, timePerSample, timePerTick, samplePerTick;
		bool songLoaded = false;
		double amplifierFinal;

		long startTime, endTime, excuteTime;
		
		bool setPosReq = false;
		int setPosReqTo = -255;
	public:
		// Some public settings:
		bool interpolation;

	private:
		Scope scopeInt;

		const uint8_t vibTab[32] =
		{
			  0, 24, 49, 74, 97,120,141,161,
			180,197,212,224,235,244,250,253,
			255,253,250,244,235,224,212,197,
			180,161,141,120, 97, 74, 49, 24
		};

		const uint16_t periodTab[105] =
		{
			907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
			808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
			720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
			640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
			570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
			508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
			453,450,447,445,442,439,436,433,428
		};

		const int8_t fineTuneTable[32] =
		{
			   0,  16,  32,  48,  64,  80,  96, 112,
			-128,-112, -96, -80, -64, -48, -32, -16,
			-128,-112, -96, -80, -64, -48, -32, -16,
			   0,  16,  32,  48,  64,  80,  96, 112
		};

		const int8_t RxxVolSlideTable[16] =
		{
			0, -1, -2, -4, -8, -16,  0,  0,
			0,  1,  2,  4,  8,  16,  0,  0
		};

		Channel *channels;

		void RecalcAmp();

		void UpdateTimer();

		void ResetChannels();

		void ResetPatternEffects();

		Note GetNote(uint8_t pos, uint8_t row, uint8_t col);

		EnvInfo CalcEnvelope(Instrument inst, int16_t pos, bool calcPan);

		void CalcPeriod(uint8_t i);

		void UpdateChannelInfo();

		void ChkEffectRow(Note thisNote, uint8_t i, bool byPassEffectCol, bool RxxRetrig = false);

		void ChkNote(Note thisNote, uint8_t i, bool byPassDelayChk, bool RxxRetrig = false);

		void ChkEffectTick(uint8_t i, Note thisNote);

		void NextRow();

		void NextTick();

		void SetPosInternal();

		inline void MixAudio(int16_t *buffer, uint32_t pos, int32_t samples);

		void FillBuffer(int16_t *buffer);
#ifdef _SDL2
		void CreateAudioDevice();

		struct CallbackObject
		{
			XMPlayer *parent;

			void WriteBufferCallback(uint8_t *buffer, int length);

			static void ForwardCallback(void *userdata, Uint8 *stream, int len);
		};

		CallbackObject callbackObject;
#endif
#ifdef _SFML

		void CreateSoundStream();

		/* Inner Class for Sound Streaming (SFML) */

		class CustomSoundStream : public sf::SoundStream
		{
		private:
			int sampleRate;
			int bufferSize;
			XMPlayer *parent;
			void (XMPlayer::*FillBufferCallback)(int16_t *buffer);

			sf::Int16 *buffer;

			bool onGetData(Chunk &data) override;

			void onSeek(sf::Time timeOffset) override;

		public:
			CustomSoundStream(int initSampleRate, int initBufferSize, void (XMPlayer:: *initFillBufferCallback)(int16_t *buffer), XMPlayer *initParent);

			~CustomSoundStream();
		};

		CustomSoundStream *customStream;
#endif

	public:
		bool IsLoaded();
		bool IsPlaying();

		bool LoadModule(uint8_t *SongDataOrig, uint32_t SongDataLeng, bool UsingInterpolation = true, bool UseStereo = true, bool LoopSong = true, int BufSize = BUFFER_SIZE, int SmpRate = SMP_RATE);
		bool PlayModule();
		bool StopModule();
		void ResetModule();
		void PlayPause(bool play);

		void UnmuteAll();

		void CleanUp();


		void SetAmp(float value);
		void SetPos(int16_t pos);
		Note GetNoteExt(int16_t pos, int16_t row, uint8_t col);
		int16_t GetPatternLength(int patternNum);
		void GetCurrentStat(Stat &dest);

		/* Legacy Functions */

		/*
		bool IsPlaying();
		void SetPos(int16_t Pos);
		void SetVolume(uint8_t Volume);
		void SetAmp(float Value);
		void SetInterpolation(bool UseInterpol = true);
		void SetStereo(bool UseStereo = true);
		void SetLoop(bool LoopSong = true);
		void SetPanMode(int8_t Mode = 0);
		void SetIgnoreF00(bool True);

		int16_t GetSpd();
		int32_t GetPos();
		int32_t GetSongInfo();
		int16_t GetPatLen(uint8_t PatNum);
		//Note GetNoteExt(int16_t pos, int16_t row, uint8_t col);
		uint8_t *GetPatternOrder();
		char *GetSongName();
		long GetExcuteTime();
		uint8_t GetActiveChannels();
		*/

		~XMPlayer();
	};
}

#endif
