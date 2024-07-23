//Glacc XM Module Player
//Glacc 2024-07-14
//
//      2024-07-14  Moved code into class Glacc::XMPlayer
//                  
//      2024-07-13  Updated coding style
//                  Added SFML/Audio support
//                  Check offset while loading module
//
//      2021-01-10  Fixed arpeggio note not update at last tick per row
//
//      2020-10-24  Changed name to "Glacc XM Player"
//
//      2020-07-21  Fixed SegmentFault caused by unsynced SDL_AudioSpec
//
//      2020-06-05  Rewrite mixer timing part
//
//      2020-05-17  Fixed borken Dxx effect algorithm
//                  Fixed delay algorithm
//
//      2020-05-04  Improved volume ramping
//
//      2020-05-01  Fixed Rxx effect don't work in some conditions
//
//      2020-04-30  Added GetActiveChannels()
//                  Added SetPanMode()
//                  Compatibility improvement of effect EDx
//                  Fixed F00 command bug and added SetIgnoreF00()
//                  Fixed Lx/Rx is using wrong parameter
//
//      2020-04-29  Less 64-bit and floating calculation in audio mixer
//
//      2020-04-28  Performance optimization
//                  Fixed wrong effect type number (Xxx) in ChkEffectRow
//
//      2020-04-26  Added GetExcuteTime()
//
//      2020-04-25  Changed audio library from winmm to SDL2
//
//      2020-04-24  Put all code into a namespace
//
//      2020-04-22  Playback compatibility improvement
//
//      2020-04-21  Full volume ramping
//                  FT2 square root panning law
//
//      2020-04-18  Auto vibrato
//
//      2020-04-17  Unroll bidi loop samples
//
//      2020-04-14  Performance optimization
//
//  Missing effects:
//      E3x Glissando control
//      E4x Vibrato control
//      E7x Tremolo control

#include "xmPlayer.hpp"

#define VOLRAMPING_SAMPLES 40
#define VOLRAMP_VOLSET_SAMPLES 20
#define SMP_CHANGE_RAMP 20
#define VOLRAMP_NEW_INSTR 20

#define TOINT_SCL 65536.0
#define TOINT_SCL_RAMPING 1048576.0
#define INT_ACC 16
#define INT_ACC_RAMPING 20
#define INT_ACC_INTERPOL 15
#define INT_MASK 0xFFFF

#define NOTE_SIZE_XM 5
#define ROW_SIZE_XM NOTE_SIZE_XM * numOfChannels

#define ch channels[i]

#define PI 3.1415926535897932384626433832795

namespace Glacc
{
#ifdef _SFML
	void XMPlayer::CreateSoundStream()
	{
		if (customStream == NULL)
			customStream = new CustomSoundStream(sampleRate, bufferSize, &XMPlayer::FillBuffer, this);
	}

	bool XMPlayer::CustomSoundStream::onGetData(Chunk &data)
	{
		if (buffer == NULL)
			return false;

		data.samples = buffer;
		data.sampleCount = bufferSize * 2;

		(parent->*FillBufferCallback)(buffer);

		return true;
	}

	void XMPlayer::CustomSoundStream::onSeek(sf::Time timeOffset) {}

	XMPlayer::CustomSoundStream::CustomSoundStream(int initSampleRate, int initBufferSize, void (XMPlayer:: *initFillBufferCallback)(int16_t *buffer), XMPlayer *initParent)
	{
		sampleRate = initSampleRate;
		bufferSize = initBufferSize;
		parent = initParent;
		FillBufferCallback = initFillBufferCallback;

		initialize(2, sampleRate);
		buffer = (sf::Int16 *)calloc(sizeof(int16_t) * 2, bufferSize);
	}

	XMPlayer::CustomSoundStream::~CustomSoundStream()
	{
		if (buffer != NULL)
			free(buffer);
	}
#endif
#ifdef _SDL2
	void XMPlayer::CreateAudioDevice()
	{
		if (deviceID == 0)
		{
			SDL_zero(audioSpec);

			audioSpec.freq = sampleRate;
			audioSpec.format = AUDIO_S16;
			audioSpec.channels = 2;
			audioSpec.samples = bufferSize;
			audioSpec.callback = CallbackObject::ForwardCallback;
			audioSpec.userdata = &callbackObject;
			deviceID = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &actualSpec, 0);
		}
	}
#endif

	void XMPlayer::RecalcAmp()
	{
		//amplifierFinal = amplifier * 0.5;// /(MIN(3, MAX(1, numOfChannels/10)))*.7;

		amplifierFinal = amplifier;
	}

	void XMPlayer::UpdateTimer()
	{
		timePerTick = 2.5 / tempo;
		samplePerTick = timePerTick / timePerSample;
	}

	void XMPlayer::ResetChannels()
	{
		int i = 0;
		while (i < numOfChannels)
		{
			ch.note = 0;
			ch.relNote = 0;
			ch.noteArpeggio = 0;
			ch.lastNote = 0;
			ch.pos = 0;
			ch.posL16 = 0;
			ch.delta = 0;
			ch.period = 0;
			ch.targetPeriod = 0;
			ch.fineTune = 0;
			ch.loop = 0;

			ch.samplePlaying = -1;
			ch.sample = -1;
			ch.instrument = 0;
			ch.nextInstrument = 0;
			ch.lastInstrument = 0;

			ch.volCmd = 0;
			ch.volume = 0;
			ch.lastVol = 0;
			ch.volTargetInst = 0;
			ch.volTargetL = 0;
			ch.volTargetR = 0;
			ch.volFinalL = 0;
			ch.volFinalR = 0;
			ch.volFinalInst = 0;
			ch.volRampSpdL = 0;
			ch.volRampSpdR = 0;
			ch.pan = 128;
			ch.panFinal = 128;

			ch.effect = 0;
			ch.parameter = 0;

			ch.vibratoPos = 0;
			ch.vibratoAmp = 0;
			ch.vibratoType = 0;
			ch.volVibrato = false;
			ch.tremorPos = 0;
			ch.tremorAmp = 0;
			ch.tremorType = 0;

			ch.slideUpSpd = 0;
			ch.slideDnSpd = 0;
			ch.slideSpd = 0;
			ch.vibratoPara = 0;
			ch.tremoloPara = 0;
			ch.volSlideSpd = 0;
			ch.fineProtaUp = 0;
			ch.fineProtaDn = 0;
			ch.fineVolUp = 0;
			ch.fineVolDn = 0;
			ch.globalVolSlideSpd = 0;
			ch.panSlideSpd = 0;
			ch.retrigPara = 0;
			ch.EFProtaUp = 0;
			ch.EFProtaDn = 0;

			ch.delay = 0;
			ch.RxxCounter = 0;
			ch.tremorTick = 0;
			ch.tremorMute = false;

			ch.startCount = 0;
			ch.endCount = SMP_CHANGE_RAMP;
			ch.prevSmp = 0;
			ch.endSmp = 0;

			ch.autoVibPos = 0;
			ch.autoVibSweep = 0;
			ch.volEnvelope = 0;
			ch.panEnvelope = 0;
			ch.envFlags = 0;

			ch.instTrig = false;
			ch.active = false;
			ch.fading = false;
			ch.keyOff = false;
			ch.fadeTick = 0;

			//ch.muted = false;

			i ++ ;
		}
	}

	void XMPlayer::ResetPatternEffects()
	{
		sampleToNextTick = 0;

		globalVol = 64;

		patBreak = -1;
		patJump = -1;
		patDelay = 0;

		patRepeat = 0;
		repeatPos = 0;
		repeatTo = -1;
	}

	void XMPlayer::ResetModule()
	{
#ifdef _SDL2
		SDL_PauseAudioDevice(deviceID, 0);
#endif
#ifdef _SFML
		if (customStream != NULL)
			customStream->stop();
#endif
		isPlaying = false;

		tempo = defaultTempo;
		speed = defaultSpd;
		tick = speed - 1;

		curRow = -1;
		curPos = 0;

		setPosReqTo = -255;

		ResetPatternEffects();

		RecalcAmp();

		ResetChannels();

		UpdateTimer();
	}

	bool XMPlayer::LoadModule(uint8_t *songDataOrig, uint32_t songDataLen, bool useInterpolation, bool stereoEnabled, bool loopSong, int bufSize, int smpRate)
	{
		try
		{
			class OffsetChecker
			{
			public:
				int maxLen = 0;

				int Check(int offset)
				{
					if (offset >= maxLen || offset < 0)
						throw false;

					return offset;
				}

				OffsetChecker(int initMaxLen)
				{
					maxLen = initMaxLen;
				}
			};

			OffsetChecker offsetChecker(songDataLen);

			loop = loopSong;
			stereo = stereoEnabled;
			interpolation = useInterpolation;

			panMode = 0;
			ignoreF00 = true;

			sampleRate = smpRate;
			bufferSize = bufSize;

			timePerSample = 1.0 / sampleRate;

			masterVolume = 255;

			int i, j, k;
			int32_t songDataOfs = 17;

			//Song data malloc
			if (songData != NULL)
				free(songData);
			songData = (uint8_t *)malloc(songDataLen);
			if (songData == NULL)
				return false;

			memcpy(songData, songDataOrig, songDataLen);

			//Song name
			i = 0;
			while (i < 20)
				songName[i++] = songData[offsetChecker.Check(songDataOfs++)];

			//Tracker name & version
			songDataOfs = 38;
			i = 0;
			while (i < 20)
				trackerName[i++] = songData[offsetChecker.Check(songDataOfs++)];

			trackerVersion = (((((uint16_t)songData[offsetChecker.Check(songDataOfs)]) << 8) & 0xFF00) | songData[offsetChecker.Check(songDataOfs + 1)]);

			songDataOfs = 60;

			//Song settings
			int32_t HeaderSize = *(int32_t *)(songData + offsetChecker.Check(songDataOfs)) + 60;
			songLength = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 4));
			rstPos = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 6));
			numOfChannels = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 8));
			numOfPatterns = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 10));
			numOfInstruments = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 12));
			useAmigaFreqTable = !(songData[offsetChecker.Check(songDataOfs + 14)] & 1);

			defaultSpd = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 16));
			defaultTempo = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 18));

			if (channels != NULL)
				free(channels);
			channels = (Channel *)malloc(sizeof(Channel) * numOfChannels);
			if (channels == NULL)
				return false;

			//Pattern order table
			songDataOfs = 80;
			i = 0;
			while (i < songLength)
				orderTable[i++] = songData[offsetChecker.Check(songDataOfs++)];

			//Pattern data size calc
			memset(patternAddr, 0, 256 * 4);
			totalPatSize = 0;

			songDataOfs = HeaderSize;
			int32_t patternOrig = songDataOfs;
			i = 0;
			while (i < numOfPatterns)
			{
				int32_t patHeaderSize = *(int32_t *)(songData + offsetChecker.Check(songDataOfs));
				int16_t patternLeng = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 5));
				int16_t patternSize = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 7));

				patternAddr[i++] = totalPatSize;

				songDataOfs += patHeaderSize + patternSize;

				totalPatSize += 2 + patternLeng * ROW_SIZE_XM;
			}

			//Pattern data init
			if (patternData != NULL)
				free(patternData);
			patternData = (uint8_t *)malloc(totalPatSize);
			if (patternData == NULL)
				return false;

			memset(patternData, 0, totalPatSize);

			//Patter data
			songDataOfs = patternOrig;
			i = 0;
			while (i < numOfPatterns)
			{
				int32_t patHeaderSize = *(int32_t *)(songData + offsetChecker.Check(songDataOfs));
				int16_t patternLeng = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 5));
				int16_t patternSize = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 7));

				int32_t PDIndex = patternAddr[i];
				patternData[PDIndex++] = patternLeng & 0xFF;
				patternData[PDIndex++] = (int8_t)((patternLeng >> 8) & 0xFF);

				songDataOfs += patHeaderSize;

				if (patternSize > 0)
				{
					j = 0;
					while (j < patternLeng)
					{
						k = 0;
						while (k < numOfChannels)
						{
							int8_t signByte = songData[offsetChecker.Check(songDataOfs++)];

							patternData[PDIndex] = 0;
							patternData[PDIndex + 1] = 0;

							if (signByte & 0x80)
							{
								if (signByte & 0x01) patternData[PDIndex] = songData[offsetChecker.Check(songDataOfs++)];
								if (signByte & 0x02) patternData[PDIndex + 1] = songData[offsetChecker.Check(songDataOfs++)];
								if (signByte & 0x04) patternData[PDIndex + 2] = songData[offsetChecker.Check(songDataOfs++)];
								if (signByte & 0x08) patternData[PDIndex + 3] = songData[offsetChecker.Check(songDataOfs++)];
								if (signByte & 0x10) patternData[PDIndex + 4] = songData[songDataOfs++];
							}
							else
							{
								patternData[PDIndex] = signByte;
								patternData[PDIndex + 1] = songData[offsetChecker.Check(songDataOfs++)];
								patternData[PDIndex + 2] = songData[offsetChecker.Check(songDataOfs++)];
								patternData[PDIndex + 3] = songData[offsetChecker.Check(songDataOfs++)];
								patternData[PDIndex + 4] = songData[offsetChecker.Check(songDataOfs++)];
							}
							PDIndex += 5;
							k ++;
						}
						j ++;
					}
				}
				i ++;
			}

			//instrument size calc
			if (sampleStartIndex != NULL)
				free(sampleStartIndex);
			sampleStartIndex = (int16_t *)malloc(numOfInstruments * 2);
			if (sampleStartIndex == NULL)
				return false;

			if (instruments != NULL)
				free(instruments);
			instruments = (Instrument *)malloc(numOfInstruments * sizeof(Instrument));
			if (instruments == NULL)
				return false;

			totalInstSize = totalSampleSize = totalSampleNum = 0;
			int instOrig = songDataOfs;
			i = 0;
			while (i < numOfInstruments)
			{
				int32_t instSize = *(int32_t *)(songData + offsetChecker.Check(songDataOfs));
				int16_t instSampleNum = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 27));
				instruments[i].sampleNum = instSampleNum;

				//name
				k = 0;
				while (k < 22)
				{
					instruments[i].name[k] = songData[offsetChecker.Check(songDataOfs + 4 + k)];
					k ++;
				}

				if (instSampleNum > 0)
				{
					//note mapping
					j = 0;
					while (j < 96)
					{
						instruments[i].sampleMap[j] = totalSampleNum + songData[offsetChecker.Check(songDataOfs + 33 + j)];
						j ++;
					}

					//Vol envelopes
					j = 0;
					while (j < 24)
					{
						instruments[i].volEnvelops[j] = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 129 + j * 2));
						j ++;
					}

					//pan envelopes
					j = 0;
					while (j < 24)
					{
						instruments[i].panEnvelops[j] = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 177 + j * 2));
						j ++;
					}

					instruments[i].volPoints = songData[offsetChecker.Check(songDataOfs + 225)];
					instruments[i].panPoints = songData[offsetChecker.Check(songDataOfs + 226)];
					instruments[i].volSustainPt = songData[offsetChecker.Check(songDataOfs + 227)];
					instruments[i].volLoopStart = songData[offsetChecker.Check(songDataOfs + 228)];
					instruments[i].volLoopEnd = songData[offsetChecker.Check(songDataOfs + 229)];
					instruments[i].panSustainPt = songData[offsetChecker.Check(songDataOfs + 230)];
					instruments[i].panLoopStart = songData[offsetChecker.Check(songDataOfs + 231)];
					instruments[i].panLoopEnd = songData[offsetChecker.Check(songDataOfs + 232)];
					instruments[i].volType = songData[offsetChecker.Check(songDataOfs + 233)];
					instruments[i].panType = songData[offsetChecker.Check(songDataOfs + 234)];
					instruments[i].vibratoType = songData[offsetChecker.Check(songDataOfs + 235)];
					instruments[i].vibratoSweep = songData[offsetChecker.Check(songDataOfs + 236)];
					instruments[i].vibratoDepth = songData[offsetChecker.Check(songDataOfs + 237)];
					instruments[i].vibratoRate = songData[offsetChecker.Check(songDataOfs + 238)];
					instruments[i].fadeOut = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 239));

					songDataOfs += instSize;

					int32_t subOfs = 0;
					int32_t sampleDataOfs = 0;
					j = 0;
					while (j < instSampleNum)
					{
						subOfs = j * 40;
						int32_t sampleLeng = *(int32_t *)(songData + offsetChecker.Check(songDataOfs + subOfs));
						int32_t loopStart = *(int32_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + 4));
						int32_t loopLeng = *(int32_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + 8));
						int8_t sampleType = songData[offsetChecker.Check(songDataOfs + subOfs + 14)] & 0x03;

						sampleDataOfs += sampleLeng;

						if (sampleType == 1)
							sampleLeng = loopStart + loopLeng;
						if ((songData[offsetChecker.Check(songDataOfs + subOfs + 14)] & 0x03) >= 2)
							sampleLeng = loopStart + (loopLeng + loopLeng);

						totalSampleSize += sampleLeng;
						j ++;
					}
					songDataOfs += instSampleNum * 40 + sampleDataOfs;
				}
				else songDataOfs += instSize;

				sampleStartIndex[i] = totalSampleNum;
				totalSampleNum += instSampleNum;

				i ++;
			}

			//sample convert
			if (sampleHeaderAddr != NULL)
				free(sampleHeaderAddr);
			sampleHeaderAddr = (int32_t *)malloc(totalSampleNum * 4);
			if (sampleHeaderAddr == NULL)
				return false;


			if (sampleData != NULL)
				free(sampleData);
			sampleData = (int8_t *)malloc(totalSampleSize);
			if (sampleData == NULL)
				return false;

			if (samples != NULL)
				free(samples);
			samples = (Sample *)malloc(totalSampleNum * sizeof(Sample));
			if (samples == NULL)
				return false;

			int16_t sampleNum = 0;
			int32_t sampleWriteOfs = 0;
			songDataOfs = instOrig;
			i = 0;
			while (i < numOfInstruments)
			{
				int32_t instSize = *(int32_t *)(songData + offsetChecker.Check(songDataOfs));
				int16_t instSampleNum = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + 27));

				songDataOfs += instSize;

				if (instSampleNum > 0)
				{
					int32_t subOfs = 0;
					int32_t sampleDataOfs = 0;
					j = 0;
					while (j < instSampleNum)
					{
						sampleNum = sampleStartIndex[i] + j;
						subOfs = j * 40;
						sampleHeaderAddr[sampleNum] = songDataOfs + subOfs;
						int32_t sampleLeng = *(int32_t *)(songData + offsetChecker.Check(songDataOfs + subOfs));
						int32_t loopStart = *(int32_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + 4));
						int32_t loopLeng = *(int32_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + 8));
						int8_t sampleType = songData[offsetChecker.Check(songDataOfs + subOfs + 14)];
						bool is16Bit = (sampleType & 0x10);

						int16_t sampleNum = sampleStartIndex[i] + j;

						samples[sampleNum].origInst = i + 1;
						samples[sampleNum].type = sampleType & 0x03;
						samples[sampleNum].is16Bit = is16Bit;

						samples[sampleNum].volume = songData[offsetChecker.Check(songDataOfs + subOfs + 12)];
						samples[sampleNum].pan = songData[offsetChecker.Check(songDataOfs + subOfs + 15)];
						samples[sampleNum].fineTune = *(int8_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + 13));
						samples[sampleNum].relNote = *(int8_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + 16));

						k = 0;
						while (k < 22)
						{
							samples[sampleNum].name[k] = songData[offsetChecker.Check(songDataOfs + subOfs + 18 + k)];
							k ++;
						}

						samples[sampleNum].data = sampleData + sampleWriteOfs;
						subOfs = 40 * instSampleNum + sampleDataOfs;
						sampleDataOfs += sampleLeng;

						if (is16Bit)
						{
							samples[sampleNum].length = sampleLeng >> 1;
							samples[sampleNum].loopStart = loopStart >> 1;
							samples[sampleNum].loopLength = loopLeng >> 1;
						}
						else
						{
							samples[sampleNum].length = sampleLeng;
							samples[sampleNum].loopStart = loopStart;
							samples[sampleNum].loopLength = loopLeng;
						}

						int32_t reversePoint = -1;
						if (samples[sampleNum].type == 1)
							sampleLeng = loopStart + loopLeng;
						else if (samples[sampleNum].type >= 2)
						{
							reversePoint = loopStart + loopLeng;
							sampleLeng = reversePoint + loopLeng;
						}

						bool reverse = false;
						int32_t readOfs = 0;
						int16_t oldPt = 0;
						k = 0;
						if (is16Bit)
						{
							int16_t newPt;
							sampleLeng >>= 1;
							reversePoint >>= 1;
							while (k < sampleLeng)
							{
								//int16_t newPt = (int16_t)SONGSEEK16(songDataOfs + subOfs, songData) + oldPt;
								if (k == reversePoint)
								{
									reverse = true;
									readOfs -= 2;
								}
								if (reverse)
								{
									newPt = -*(int16_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + readOfs)) + oldPt;
									readOfs -= 2;
								}
								else
								{
									newPt = *(int16_t *)(songData + offsetChecker.Check(songDataOfs + subOfs + readOfs)) + oldPt;
									readOfs += 2;
								}
								*(int16_t *)(sampleData + sampleWriteOfs) = newPt;
								sampleWriteOfs += 2;

								oldPt = newPt;
								k ++;
							}
						}
						else
						{
							int8_t newPt;
							while (k < sampleLeng)
							{
								if (k == reversePoint)
								{
									reverse = true;
									readOfs --;
								}
								if (reverse)
								{
									newPt = -songData[offsetChecker.Check(songDataOfs + subOfs + readOfs)] + oldPt;
									readOfs --;
								}
								else
								{
									newPt = songData[offsetChecker.Check(songDataOfs + subOfs + readOfs)] + oldPt;
									readOfs ++;
								}
								sampleData[sampleWriteOfs++] = newPt;

								oldPt = newPt;
								k ++;
							}
						}

						j ++;
						//sampleNum ++ ;
					}
					songDataOfs += 40 * instSampleNum + sampleDataOfs;
				}
				i ++;
			}

			if (sampleHeaderAddr != NULL)
			{
				free(sampleHeaderAddr);
				sampleHeaderAddr = NULL;
			}
			if (sampleStartIndex != NULL)
			{
				free(sampleStartIndex);
				sampleStartIndex = NULL;
			}

			if (songData != NULL)
			{
				free(songData);
				songData = NULL;
			}

			if (scopeInt.scopeData != NULL)
				free(scopeInt.scopeData);
			if (scopes.scopeData != NULL)
				free(scopes.scopeData);
			scopeInt.scopeData = (int8_t *)calloc(numOfChannels * bufferSize, sizeof(int8_t));
			scopes.scopeData = (int8_t *)calloc(numOfChannels * bufferSize, sizeof(int8_t));
			scopes.sizePerCh = scopeInt.sizePerCh = bufferSize;
			scopes.numOfCh = scopeInt.numOfCh = numOfChannels;
			if (scopeInt.scopeData == NULL || scopes.scopeData == NULL)
				return false;

			ResetModule();
			songLoaded = true;

			UnmuteAll();
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	XMPlayer::Note XMPlayer::GetNote(uint8_t pos, uint8_t row, uint8_t col)
	{
		Note thisNote = *(Note *)(patternData + patternAddr[pos] + ROW_SIZE_XM*row + col*NOTE_SIZE_XM + 2);

		return thisNote;
	}

	XMPlayer::EnvInfo XMPlayer::CalcEnvelope(Instrument inst, int16_t pos, bool calcPan)
	{
		EnvInfo retInfo;

		if (inst.sampleNum > 0)
		{
			int16_t *envData = calcPan ? inst.panEnvelops : inst.volEnvelops;
			int8_t numOfPoints = calcPan ? inst.panPoints : inst.volPoints;
			retInfo.maxPoint = envData[(numOfPoints - 1)*2];

			if (pos == 0)
			{
				retInfo.value = envData[1];
				return retInfo;
			}

			int i = 0;
			while (i < numOfPoints - 1)
			{
				if (pos < envData[i*2 + 2])
				{
					int16_t prevPos = envData[i*2];
					int16_t nextPos = envData[i*2 + 2];
					int16_t prevValue = envData[i*2 + 1];
					int16_t nextValue = envData[i*2 + 3];
					retInfo.value = (int8_t)((nextValue - prevValue)*(pos - prevPos)/(nextPos - prevPos) + prevValue);
					return retInfo;
				}
				i ++ ;
			}

			retInfo.value = envData[(numOfPoints - 1)*2 + 1];
			return retInfo;
		}
		else
		{
			retInfo.value = 0;
			retInfo.maxPoint = 0;
		}

		return retInfo;
	}

	void XMPlayer::CalcPeriod(uint8_t i)
	{
		int16_t realNote = (ch.noteArpeggio <= 119 && ch.noteArpeggio > 0 ? ch.noteArpeggio : ch.note) + ch.relNote - 1;
		int8_t fineTune = ch.fineTune;
		int16_t period;
		if (!useAmigaFreqTable)
			period = 7680 - realNote*64 - fineTune/2;
		else
		{
			//https://github.com/dodocloud/xmplayer/blob/master/src/xmlib/engine/utils.ts
			//function calcPeriod
			double fineTuneFrac = floor((double)fineTune / 16.0);
			uint16_t period1 = periodTab[8 + (realNote % 12)*8 + (int16_t)fineTuneFrac];
			uint16_t period2 = periodTab[8 + (realNote % 12)*8 + (int16_t)fineTuneFrac + 1];
			fineTuneFrac = ((double)fineTune / 16.0) - fineTuneFrac;
			period = (int16_t)round((1.0 - fineTuneFrac)*period1 + fineTuneFrac*period2) * (16.0 / pow(2, floor(realNote / 12) - 1));
		}

		ch.period = MAX(ch.period, 50);
		period = MAX(period, 50);

		if (ch.noteArpeggio <= 119 && ch.noteArpeggio > 0)
			ch.period = period;
		else
			ch.targetPeriod = period;
	}

	void XMPlayer::UpdateChannelInfo()
	{
		int i = 0;
		while (i < numOfChannels)
		{
			if (ch.active && ch.samplePlaying != -1)
			{
				//arpeggio
				if (ch.parameter != 0 && ch.effect == 0)
				{
					int8_t arpNote1 = ch.parameter >> 4;
					int8_t arpNote2 = ch.parameter & 0xF;
					int8_t arpeggio[3] = {0, arpNote2, arpNote1};
					if (useAmigaFreqTable) ch.noteArpeggio = ch.note + arpeggio[tick%3];
					else ch.periodOfs = -arpeggio[tick%3] * 64;
					//ch.period = ch.targetPeriod;
				}

				//Auto vibrato
				int32_t autoVibFinal = 0;
				if (ch.samplePlaying != -1)
				{
					Instrument smpOrigInst = instruments[samples[ch.samplePlaying].origInst - 1];

					if (smpOrigInst.sampleNum > 0)
					{
						ch.autoVibPos += smpOrigInst.vibratoRate;
						if (ch.autoVibSweep < smpOrigInst.vibratoSweep) ch.autoVibSweep ++ ;

						if (smpOrigInst.vibratoRate)
						{
							//https://github.com/milkytracker/MilkyTracker/blob/master/src/milkyplay/PlayerSTD.cpp
							//Line 568 - 599
							uint8_t vibPos = ch.autoVibPos >> 2;
							uint8_t vibDepth = smpOrigInst.vibratoDepth;

							int32_t value = 0;
							switch (smpOrigInst.vibratoType) {
								// sine (we must invert the phase here)
								case 0:
									value = ~vibTab[vibPos&31];
									break;
								// square
								case 1:
									value = 255;
									break;
								// ramp down (down being the period here - so ramp frequency up ;)
								case 2:
									value = ((vibPos & 31) *539087) >> 16;
									if ((vibPos & 63) > 31) value = 255 - value;
									break;
								// ramp up (up being the period here - so ramp frequency down ;)
								case 3:
									value = ((vibPos & 31) * 539087) >> 16;
									if ((vibPos & 63) > 31) value = 255 - value;
									value = -value;
									break;
							}

							autoVibFinal = ((value * vibDepth) >> 1);
							if (smpOrigInst.vibratoSweep) {
								autoVibFinal *= ((int32_t)ch.autoVibSweep << 8) / smpOrigInst.vibratoSweep;
								autoVibFinal >>= 8;
							}

							if ((vibPos & 63) > 31) autoVibFinal = -autoVibFinal;

							autoVibFinal >>= 7;
						}
					}
				}

				//delta calculation
				CalcPeriod(i);
				double freq;
				double realPeriod = MAX(ch.period + ch.periodOfs, 50) + ch.vibratoAmp*sin((ch.vibratoPos & 0x3F)*PI/32)*8 + autoVibFinal;
				if (!useAmigaFreqTable)
					freq = 8363*pow(2, (4608 - realPeriod)/768);
				else freq = 8363.0*1712/realPeriod;

				ch.delta = (uint32_t)(freq/sampleRate*TOINT_SCL);

				Instrument Inst = instruments[ch.instrument - 1];

				//volume
				ch.volume = MAX(MIN(ch.volume, 64), 0);
				int16_t realVol = ch.tremorMute ? 0 : (int8_t)MAX(MIN(ch.volume + ch.tremorAmp*sin((ch.tremorPos & 0x3F)*PI/32)*4, 64), 0);
				int16_t volTarget = realVol*globalVol/64;
				if (Inst.volType & 0x01)
				{
					EnvInfo volEnv = CalcEnvelope(Inst, ch.volEnvelope, false);

					if (Inst.volType & 0x02)
					{
						if (ch.volEnvelope != Inst.volEnvelops[Inst.volSustainPt*2] || ch.fading)
							ch.volEnvelope ++ ;
					}
					else ch.volEnvelope ++ ;

					if (Inst.volType & 0x04)
					{
						if (ch.volEnvelope >= Inst.volEnvelops[Inst.volLoopEnd*2])
							ch.volEnvelope = Inst.volEnvelops[Inst.volLoopStart*2];
					}

					if (ch.volEnvelope >= volEnv.maxPoint)
						ch.volEnvelope = volEnv.maxPoint;

					int16_t instFadeout = Inst.fadeOut;
					int32_t fadeOutVol;
					if (ch.fading && instFadeout > 0)
					{
						int16_t FadeOutLeng = 32768 / instFadeout;
						if (ch.fadeTick < FadeOutLeng) ch.fadeTick ++ ;
						else ch.active = false;
						fadeOutVol = 64*(FadeOutLeng - ch.fadeTick)/FadeOutLeng;
					}
					else fadeOutVol = 64;

					ch.volTargetInst = volEnv.value;
					//ch.volTarget = fadeOutVol*globalVol/64*realVol/64;
					//ch.volTarget = fadeOutVol*volEnv.value/64*globalVol/64*realVol/64;
					volTarget = fadeOutVol*globalVol/64*realVol/64;
				}
				else
				{
					ch.volTargetInst = 64;
					if (ch.fading) ch.volume = 0;
				}
				ch.volTargetInst <<= INT_ACC_RAMPING;

				volTarget = MAX(MIN(volTarget, 64), 0);

				//Panning
				ch.pan = MAX(MIN(ch.pan, 255), 0);
				if (Inst.panType & 0x01)
				{
					EnvInfo panEnv = CalcEnvelope(Inst, ch.panEnvelope, true);
					if (Inst.panType & 0x02)
					{
						if (ch.panEnvelope != Inst.panEnvelops[Inst.panSustainPt*2] || ch.fading)
							ch.panEnvelope ++ ;
					}
					else ch.panEnvelope ++ ;

					if (Inst.panType & 0x04)
					{
						if (ch.panEnvelope >= Inst.panEnvelops[Inst.panLoopEnd*2])
							ch.panEnvelope = Inst.panEnvelops[Inst.panLoopStart*2];
					}

					if (ch.panEnvelope >= panEnv.maxPoint)
						ch.panEnvelope = panEnv.maxPoint;

					ch.panFinal = ch.pan + (((panEnv.value - 32) * (128 - abs(ch.pan - 128))) >> 5);
				}
				else ch.panFinal = ch.pan;
				ch.panFinal = MAX(MIN(ch.panFinal, 255), 0);

				//sample info
				Sample curSample = samples[ch.samplePlaying];
				ch.data = curSample.data;
				ch.loopType = curSample.type;

				ch.is16Bit = curSample.is16Bit;
				ch.smpLeng = curSample.length;
				ch.loopStart = curSample.loopStart;
				ch.loopLeng = curSample.loopLength;
				ch.loopEnd = ch.loopStart + ch.loopLeng;

				if (ch.loopType >= 2)
				{
					ch.loopEnd += ch.loopLeng;
					ch.loopLeng <<= 1;
				}
				if (!ch.loopType) ch.loop = 0;

				//Set volume ramping
				double volRampSmps = samplePerTick;
				double instVolRampSmps = samplePerTick;
				if (ch.LxxEffect)
				{
					instVolRampSmps = VOLRAMP_NEW_INSTR;
					ch.LxxEffect = false;
				}

				if (((ch.volCmd & 0xF0) >= 0x10 &&
					(ch.volCmd & 0xF0) <= 0x50) ||
					(ch.volCmd & 0xF0) == 0xC0 ||
						ch.effect == 12 || ch.effect == 8 ||
					(ch.effect == 0x14 && (ch.parameter & 0xF0) == 0xC0))
					volRampSmps = VOLRAMP_VOLSET_SAMPLES;

				if (!(Inst.volType & 0x01) && ch.fading)
				{
					volTarget = 0;
					ch.fading = false;
					volRampSmps = VOLRAMP_VOLSET_SAMPLES;
				}

				if (ch.instTrig)
				{
					//ch.VolFinal = ch.volTarget;
					//ch.volFinalInst = ch.volTargetInst;

					volRampSmps = VOLRAMP_VOLSET_SAMPLES;
					instVolRampSmps = VOLRAMP_NEW_INSTR;

					//ch.volFinalL = ch.volFinalR = 0;
					ch.instTrig = false;
				}
				//else if (ch.effect == 0xA)
				//    volRampSmps = samplePerTick;

				if (stereo)
				{
					if (!panMode)
					{
						//https://modarchive.org/forums/index.php?topic=3517.0
						//FT2 square root panning law
						ch.volTargetL = (int32_t)(volTarget * sqrt((256 - ch.panFinal) / 256.0) / .707) << INT_ACC_RAMPING;
						ch.volTargetR = (int32_t)(volTarget * sqrt(ch.panFinal / 256.0) / .707) << INT_ACC_RAMPING;
					}
					else
					{
						//Linear panning
						if (ch.panFinal > 128)
						{
							ch.volTargetL = volTarget * (256 - ch.panFinal) / 128.0;
							ch.volTargetR = volTarget;
						}
						else
						{
							ch.volTargetL = volTarget;
							ch.volTargetR = volTarget * (ch.panFinal / 128.0);
						}
						ch.volTargetL <<= INT_ACC_RAMPING;
						ch.volTargetR <<= INT_ACC_RAMPING;
					}
				}
				else ch.volTargetL = ch.volTargetR = (volTarget << INT_ACC_RAMPING);

				ch.volRampSpdL = (ch.volTargetL - ch.volFinalL) / volRampSmps;
				ch.volRampSpdR = (ch.volTargetR - ch.volFinalR) / volRampSmps;
				ch.volRampSpdInst = (ch.volTargetInst - ch.volFinalInst) / instVolRampSmps;

				if (ch.volRampSpdL == 0) ch.volFinalL = ch.volTargetL;
				if (ch.volRampSpdR == 0) ch.volFinalR = ch.volTargetR;
				if (ch.volRampSpdInst == 0) ch.volFinalInst = ch.volTargetInst;
			}
			else ch.volFinalL = ch.volFinalR = 0;
			i ++ ;
		}
	}

	void XMPlayer::ChkEffectRow(Note thisNote, uint8_t i, bool byPassEffectCol, bool RxxRetrig)
	{
		uint8_t volCmd = thisNote.volCmd;
		uint8_t volPara = thisNote.volCmd & 0x0F;
		uint8_t effect = thisNote.effect;
		uint8_t para = thisNote.parameter;
		uint8_t subEffect = para & 0xF0;
		uint8_t subPara = para & 0x0F;

		//if ((effect != 0) || (para == 0))   //0XX
		//    ch.NoteOfs = 0;

		if (!byPassEffectCol)
		{
			//effect column
			switch (effect)
			{
			default:
				break;
			case 1: //1xx
				if (para != 0) ch.slideUpSpd = para;
				break;
			case 2: //2xx
				if (para != 0) ch.slideDnSpd = para;
				break;
			case 3: //3xx
				if (para != 0) ch.slideSpd = para;
				break;
			case 4: //4xx
				if ((para & 0xF) > 0)
					ch.vibratoPara = (ch.vibratoPara & 0xF0) + (para & 0xF);
				if (((para >> 4) & 0xF) > 0)
					ch.vibratoPara = (ch.vibratoPara & 0xF) + (para & 0xF0);
				break;
			case 7: //7xx
				if ((para & 0xF) > 0)
					ch.tremoloPara = (ch.tremoloPara & 0xF0) + (para & 0xF);
				if (((para >> 4) & 0xF) > 0)
					ch.tremoloPara = (ch.tremoloPara & 0xF) + (para & 0xF0);
				break;
			case 8: //8xx
				ch.pan = para;
				break;
				//case 9: //9xx
				//    //if (ch.active) ch.pos = para*256;
				//    if (thisNote.note < 97) ch.pos = para*256;
				//    break;
			case 5: //5xx
			case 6: //6xx
			case 10:    //Axx
				if (para != 0)
				{
					if (para & 0xF) ch.volSlideSpd = -(para & 0xF);
					else if (para & 0xF0) ch.volSlideSpd = (para >> 4) & 0xF;
				}
				break;
			case 11:    //Bxx
				patJump = para;
				break;
			case 12:    //Cxx
				ch.volume = para;
				break;
			case 13:    //Dxx
				patBreak = (para >> 4) * 10 + (para & 0xF);
				break;
			case 14:    //Exx
				switch (subEffect)
				{
				case 0x10:  //E1x
					if (subPara != 0) ch.fineProtaUp = subPara;
					if (ch.period > 1) ch.period -= ch.fineProtaUp * 4;
					break;
				case 0x20:  //E2x
					if (subPara != 0) ch.fineProtaDn = subPara;
					if (ch.period < 7680) ch.period += ch.fineProtaDn * 4;
					break;
				case 0x30:  //E3x
					break;
				case 0x40:  //E4x
					break;
				case 0x50:  //E5x
					ch.fineTune = fineTuneTable[(useAmigaFreqTable ? 0 : 16) + subPara];
					break;
				case 0x60:  //E6x
					if (subPara == 0 && repeatPos < curRow) {
						repeatPos = curRow;
						patRepeat = 0;
					}
					else if (patRepeat < subPara) {
						patRepeat ++;
						repeatTo = repeatPos;
					}
					break;
				case 0x70:  //E7x
					break;
				case 0x80:  //E8x
					break;
				case 0xA0:  //EAx
					if (subPara != 0) ch.fineVolUp = subPara;
					ch.volume += ch.fineVolUp;
					break;
				case 0xB0:  //EBx
					if (subPara != 0) ch.fineVolDn = subPara;
					ch.volume -= ch.fineVolDn;
					break;
				case 0x90:  //E9x
				case 0xC0:  //ECx
					ch.delay = subPara - 1;
					break;
				case 0xD0:  //EDx
					ch.delay = subPara;
					break;
				case 0xE0:
					patDelay = subPara;
					break;
				}
				break;
			case 15:    //Fxx
				if (!ignoreF00)
				{
					if (para < 32 && para != 0) speed = para;
					else tempo = para;
				}
				else if (para != 0)
				{
					if (para < 32) speed = para;
					else tempo = para;
				}
				break;
			case 16:    //Gxx
				globalVol = MAX(MIN(para, 64), 0);
				break;
			case 17:    //Hxx
				if (para != 0)
				{
					if (para & 0xF) ch.globalVolSlideSpd = -(para & 0xF);
					else if (para & 0xF0) ch.globalVolSlideSpd = (para >> 4) & 0xF;
				}
				break;
			case 20:    //Kxx
				if (para == 0) ch.keyOff = ch.fading = true;
				else ch.delay = para;
				break;
			case 21:    //Lxx
				if (ch.instrument && ch.instrument <= numOfInstruments)
				{
					if (instruments[ch.instrument - 1].volType & 0x02)
						ch.panEnvelope = para;

					ch.volEnvelope = para;
					ch.LxxEffect = true;
					ch.active = true;
				}
				break;
			case 25:    //Pxx
				if (para != 0)
				{
					if (para & 0xF) ch.panSlideSpd = -(para & 0xF);
					else if (para & 0xF0) ch.panSlideSpd = (para >> 4) & 0xF;
				}
				break;
			case 27:    //Rxx
				//ch.RxxCounter = 1;
				//ch.delay = subPara;
				//if ((para & 0xF0) != 0) ch.retrigPara = (para >> 4) & 0xF;
				if (((para >> 4) & 0xF) > 0)
					ch.retrigPara = (ch.retrigPara & 0xF) + (para & 0xF0);
				/*
				if ((ch.retrigPara < 6) || (ch.retrigPara > 7 && ch.retrigPara < 14))
					ch.volume += RxxVolSlideTable[ch.retrigPara];
				else
				{
					switch (ch.retrigPara)
					{
						case 6:
							ch.volume = (ch.volume << 1) / 3;
							break;
						case 7:
							ch.volume >>= 1;
							break;
						case 14:
							ch.volume = (ch.volume * 3) >> 1;
							break;
						case 15:
							ch.volume <<= 1;
							break;
					}
				}
				*/
				break;
			case 29:    //Txx
				if (para != 0) ch.tremorPara = para;
				break;
			case 33:    //Xxx
				if (subEffect == 0x10)  //X1x
				{
					if (subPara != 0) ch.EFProtaUp = subPara;
					ch.period -= ch.EFProtaUp;
				}
				else if (subEffect == 0x20) //X2x
				{
					if (subPara != 0) ch.EFProtaDn = subPara;
					ch.period += ch.EFProtaDn;
				}
				break;
			}
		}

		//volume column effects
		switch (volCmd & 0xF0)
		{
			case 0x80:  //Dx
				ch.volume -= volPara;
				break;
			case 0x90:  //Ux
				ch.volume += volPara;
				break;
			case 0xA0:  //Sx
				ch.vibratoPara = (ch.vibratoPara&0xF) + ((volPara << 4) & 0xF0);
				break;
			case 0xB0:  //Vx
				if (volPara != 0)
					ch.vibratoPara = (ch.vibratoPara&0xF0) + volPara;
				break;
			case 0xC0:  //Px
				ch.pan = volPara * 17;
				break;
			case 0xF0:  //Mx
				if (volPara != 0) ch.slideSpd = ((volPara << 4) & 0xF0) + volPara;
				break;
			default:    //Vxx
				if (volCmd >= 0x10 && volCmd <= 0x50 && !RxxRetrig)
					ch.lastVol = ch.volume = volCmd - 0x10;
				break;
		}
	}

	void XMPlayer::ChkNote(Note thisNote, uint8_t i, bool byPassDelayChk, bool RxxRetrig)
	{
		/*
		note thisNote;
		thisNote.note = ch.lastNote;
		thisNote.instrument = ch.lastInstrument;
		thisNote.volCmd = ch.volCmd | ch.volPara;
		thisNote.effect = ch.effect;
		thisNote.parameter = ch.parameter;
		*/

		//uint8_t note = thisNote.note;
		//if (thisNote.note > 96 || thisNote.note == 0) note = ch.note;
		//ch.note = note;

		Note thisNoteOrig = thisNote;
		if (byPassDelayChk)
		{
			thisNote.note = ch.lastNote;
			thisNote.instrument = ch.lastInstrument;
			thisNote.volCmd = ch.volCmd;
			thisNote.effect = ch.effect;
			thisNote.parameter = ch.parameter;
		}

		if ((ch.noteArpeggio <= 119 && ch.noteArpeggio > 0) || ch.periodOfs != 0/*&& (ch.effect != 0 || ch.parameter == 0)*/)
		{
			ch.periodOfs = 0;
			ch.noteArpeggio = 0;
			if (useAmigaFreqTable)
				ch.period = ch.targetPeriod;
		}

		bool porta = (thisNote.effect == 3 || thisNote.effect == 5 || ((thisNote.volCmd & 0xF0) == 0xF0));

		bool hasNoteDelay = (thisNote.effect == 14 && ((thisNote.parameter&0xF0) >> 4) == 13 && (thisNote.parameter & 0x0F) != 0);

		if (thisNote.effect != 4 && thisNote.effect != 6 && !ch.volVibrato) ch.vibratoPos = 0;

		if (!hasNoteDelay || byPassDelayChk)
		{
			//Reference: https://github.com/milkytracker/MilkyTracker/blob/master/src/milkyplay/PlayerSTD.cpp - PlayerSTD::progressRow()

			ch.delay = -1;

			int8_t noteNum = thisNote.note;
			uint8_t instNum = thisNote.instrument;
			//bool validNote = true;

			//if (instNum && instNum <= numOfInstruments)
			//{
			//    ch.instrument = instNum;
			//}

			uint8_t oldInst = ch.instrument;
			int16_t oldSamp = ch.sample;

			bool invalidInstr = true;
			if (instNum && instNum <= numOfInstruments && noteNum < 97)
			{
				if (instruments[instNum - 1].sampleNum > 0)
				{
					ch.nextInstrument = instNum;
					invalidInstr = false;
				}
			}

			if (instNum && invalidInstr) ch.nextInstrument = 255;

			bool validNote = true;
			bool trigByNote = false;
			if (noteNum && noteNum <97)
			{
				/*
				if (instNum && instNum <= numOfInstruments)
				{
					ch.instrument = instNum;
				}
				else instNum = 0;

				bool invalidInstr = true;
				if (ch.instrument && ch.instrument <= numOfInstruments)
				{
					if (instruments[ch.instrument - 1].sampleNum > 0)
						invalidInstr = false;
				}
				*/

				if (ch.nextInstrument == 255)
				{
					instNum = 0;
					ch.active = false;
					ch.volume = 0;
					ch.sample = -1;
					ch.instrument = 0;
				}
				else ch.instrument = ch.nextInstrument;

				if (ch.nextInstrument && ch.nextInstrument <= numOfInstruments)
				{
					ch.sample = instruments[ch.nextInstrument - 1].sampleMap[noteNum - 1];

					if (ch.sample != -1 && !porta)
					{
						Sample smp = samples[ch.sample];
						int8_t relNote = smp.relNote;
						int8_t finalNote = noteNum + relNote;

						if (finalNote >= 1 && finalNote <= 119)
						{
							ch.fineTune = smp.fineTune;
							ch.relNote = relNote;
						}
						else
						{
							noteNum = ch.note;
							validNote = false;
						}
					}

					if (validNote)
					{
						ch.note = noteNum;

						CalcPeriod(i);

						if (!porta)
						{
							ch.period = ch.targetPeriod;
							ch.samplePlaying = ch.sample;
							ch.autoVibPos = ch.autoVibSweep = 0;
							ch.loop = 0;

							ch.startCount = 0;

							if (thisNote.effect == 9) ch.pos = thisNote.parameter << 8;
							else ch.pos = 0;

							if (ch.pos > samples[ch.samplePlaying].length) ch.pos = samples[ch.samplePlaying].length;

							//NoteTrig = true;
							if (!ch.active && !instNum)
							{
								trigByNote = true;
								goto TrigInst;
							}
						}
					}
				}
			}

			//FT2 bug emulation
			if ((porta || !validNote) && instNum)
			{
				instNum = ch.instrument = oldInst;
				ch.sample = oldSamp;
			}

			if (instNum && ch.sample != -1)
			{
				TrigInst:
				if (!RxxRetrig && thisNoteOrig.instrument)
				{
					//ch.volFinalL = ch.volFinalR = 0;
					ch.volume = trigByNote ? ch.lastVol : ch.lastVol = samples[ch.samplePlaying].volume;
					ch.pan = samples[ch.samplePlaying].pan;
					ch.tremorMute = false;
					ch.tremorTick = 0;
					//ch.autoVibPos = ch.autoVibSweep = 0;
					ch.tremorPos = 0;
				}
				ch.volVibrato = false;

				ch.fadeTick = ch.volEnvelope = ch.panEnvelope = 0;

				ch.instTrig = ch.active = true;
				ch.keyOff = ch.fading = false;
			}

			if (noteNum == 97)
				ch.fading = true;

			ChkEffectRow(thisNote, i, byPassDelayChk, RxxRetrig);
		}
		else
		{
			ch.delay = ch.parameter & 0x0F;
			if (porta)
			{
				ch.period = ch.targetPeriod;
				if (ch.samplePlaying != -1) CalcPeriod(i);
			}
		}
	}

	void XMPlayer::ChkEffectTick(uint8_t i, Note thisNote)
	{
		uint8_t volCmd = thisNote.volCmd;
		uint8_t volPara = thisNote.volCmd & 0x0F;
		uint8_t effect = thisNote.effect;
		uint8_t para = thisNote.parameter;
		uint8_t subEffect = para & 0xF0;
		uint8_t subPara = para & 0x0F;

		if (effect != 4 && effect != 6 && !ch.volVibrato) ch.vibratoPos = 0;
		if (effect != 27) ch.RxxCounter = 0;

		uint8_t onTime, offTime;

		//effect column
		switch (effect)
		{
			case 1: //1xx
				ch.period -= ch.slideUpSpd * 4;
				break;
			case 2: //2xx
				ch.period += ch.slideDnSpd * 4;
				break;
			case 3: //3xx
				PortaEffect:
				if (ch.period > ch.targetPeriod)
					ch.period -= ch.slideSpd * 4;
				else if (ch.period < ch.targetPeriod)
					ch.period += ch.slideSpd * 4;
				if (abs(ch.period - ch.targetPeriod) < ch.slideSpd * 4)
					ch.period = ch.targetPeriod;
				break;
			case 4: //4xx
				VibratoEffect:
				ch.vibratoAmp = ch.vibratoPara & 0xF;
				ch.vibratoPos += (ch.vibratoPara >> 4) & 0x0F;
				break;
			case 5: //5xx
				ch.volume += ch.volSlideSpd;
				goto PortaEffect;
				break;
			case 6: //6xx
				ch.volume += ch.volSlideSpd;
				goto VibratoEffect;
				break;
			case 7: //7xx
				ch.tremorAmp = ch.tremoloPara & 0xF;
				ch.tremorPos += (ch.tremoloPara >> 4) & 0x0F;
				break;
			case 8: //8xx
				ch.pan = para;
				break;
			case 10:    //Axx
				ch.volume += ch.volSlideSpd;
				break;
			case 12:    //Cxx
				ch.volume = para;
				break;
			case 14:    //Exx
				switch (subEffect)
				{
					case 0x90:  //E9x
						if (ch.delay <= 0 && subPara > 0)
						{
							ChkNote(thisNote, i, true);
							ch.delay = subPara;
						}
						break;
					case 0xC0:  //ECx
						if (ch.delay <= 0)
							ch.volume = 0;
						break;
					case 0xD0:  //EDx
						if (ch.delay <= 1 && ch.delay != -1 && !ch.keyOff) ChkNote(thisNote, i, true);
						break;
				}
				break;
			case 17:    //Hxx
				globalVol = MAX(MIN(globalVol + ch.globalVolSlideSpd, 64), 0);
				break;
			case 20:    //Kxx
				if (ch.delay <= 0)
				{
					ch.keyOff = ch.fading = true;
				}
				break;
			case 25:    //Pxx
				ch.pan += ch.panSlideSpd;
				break;
			case 27:    //Rxx
				ch.RxxCounter ++ ;
				if (ch.RxxCounter >= (ch.retrigPara & 0x0F) - 1)
				{
					ChkNote(thisNote, i, true, true);
					if ((para & 0xF) > 0)
						ch.retrigPara = (ch.retrigPara&0xF0) + (para&0xF);

					uint8_t retrigVol = (ch.retrigPara >> 4) & 0x0F;
					if ((retrigVol < 6) || (retrigVol > 7 && retrigVol < 14))
						ch.volume += RxxVolSlideTable[retrigVol];
					else
					{
						switch (retrigVol)
						{
							case 6:
								ch.volume = (ch.volume + ch.volume) / 3;
								break;
							case 7:
								ch.volume >>= 1;
								break;
							case 14:
								ch.volume = (ch.volume * 3) >> 1;
								break;
							case 15:
								ch.volume <<= 1;
								break;
						}
					}
					ch.RxxCounter = 0;
				}
				break;
			case 29:    //Txx
				ch.tremorTick ++ ;
				onTime = ((ch.tremorPara >> 4) & 0xF) + 1;
				offTime = (ch.tremorPara & 0xF) + 1;
				ch.tremorMute = (ch.tremorTick > onTime);
				if (ch.tremorTick >= onTime + offTime) ch.tremorTick = 0;
				break;
		}

		if (ch.delay > -1) ch.delay -- ;

		//volume column effects
		switch (volCmd & 0xF0)
		{
			case 0x60:  //Dx
				ch.volume -= volPara;
				break;
			case 0x70:  //Ux
				ch.volume += volPara;
				break;
			case 0xB0:  //Vx
				ch.vibratoAmp = ch.vibratoPara & 0xF;
				ch.vibratoPos += (ch.vibratoPara >> 4) & 0x0F;
				ch.volVibrato = true;
				break;
			case 0xD0:  //Lx
				ch.pan -= volPara;
				break;
			case 0xE0:  //Rx
				ch.pan += volPara;
				break;
			case 0xF0:  //Mx
				if (ch.period > ch.targetPeriod)
					ch.period -= ch.slideSpd * 4;
				else if (ch.period < ch.targetPeriod)
					ch.period += ch.slideSpd * 4;
				if (abs(ch.period - ch.targetPeriod) < ch.slideSpd * 4)
					ch.period = ch.targetPeriod;
				break;
		}
	}

	void XMPlayer::NextRow()
	{
		if (patDelay <= 0)
		{
			curRow ++ ;
			if (patBreak >= 0 && patJump >= 0)
			{
				curRow = patBreak;
				curPos = patJump;
				patRepeat = repeatPos = 0;
				repeatTo = patBreak = patJump = -1;
			}
			if (patBreak >= 0)
			{
				curRow = patBreak;
				patRepeat = repeatPos = 0;
				patBreak = repeatTo = -1;
				curPos ++ ;
			}
			if (patJump >= 0)
			{
				curPos = patJump;
				curRow = patRepeat = repeatPos = 0;
				repeatTo = patJump = -1;
			}
			if (repeatTo >= 0)
			{
				curRow = repeatTo;
				repeatTo = -1;
			}
			if (curRow >= *(int16_t *)(patternData + patternAddr[orderTable[curPos]]))
			{
				//Pattern loop bug emulation
				curRow = repeatPos > 0 ? repeatPos : 0;
				patRepeat = repeatPos = 0;
				repeatTo = -1;
				curPos ++ ;
			}
			if (curPos >= songLength)
			{
				if (loop)
					curPos = rstPos;
				else
				{
					ResetModule();
					return;
				}
			}

			int i = 0;
			while (i < numOfChannels)
			{
				Note ThisNote = GetNote(orderTable[curPos], curRow, i);

				if (ThisNote.note != 0) ch.lastNote = ThisNote.note;
				if (ThisNote.instrument != 0) ch.lastInstrument = ThisNote.instrument;
				ch.volCmd = ThisNote.volCmd;
				ch.volPara = ThisNote.volCmd & 0x0F;
				ch.effect = ThisNote.effect;
				ch.parameter = ThisNote.parameter;

				ChkNote(ThisNote, i, false);
				//if (ch.delay > -1) ch.delay -- ;

				i ++ ;
			}
		}
		else patDelay -- ;
	}

	void XMPlayer::NextTick()
	{
		int i = 0;
		tick ++ ;

		if (tick >= speed) {
			tick = 0;
			NextRow();
			return;
		}

		if (curRow >= 0)
		{
			while (i < numOfChannels)
			{
				Note ThisNote = GetNote(orderTable[curPos], curRow, i);

				ChkEffectTick(i, ThisNote);

				i ++ ;
			}
		}
	}

	void XMPlayer::SetPosInternal()
	{
		int pos = setPosReqTo;

		if (pos < 0) pos = 0;
		if (pos >= songLength) pos = songLength - 1;

		ResetChannels();
		ResetPatternEffects();

		tick = speed - 1;
		curRow = -1;
		curPos = pos;
	}

	/* Old Audio Mixer */
	/*
	inline void MixAudio(int16_t *buffer, uint32_t pos)
	{
		int32_t outL = 0;
		int32_t outR = 0;
		double result = 0;

		int i = 0;
		while (i < numOfChannels)
		{
			if (ch.active)
			{
				if (ch.samplePlaying != -1 && ch.samplePlaying < totalSampleNum)
				{
					int32_t chPos = ch.pos;

					if (ch.startCount >= SMP_CHANGE_RAMP)
					{
						ch.posL16 += ch.delta;
						chPos += ch.posL16 >> INT_ACC;
						ch.posL16 &= INT_MASK;
					}

					//volume ramping
					if (ch.volFinalL != ch.volTargetL)
					{
						if (abs(ch.volFinalL - ch.volTargetL) >= abs(ch.volRampSpdL))
							ch.volFinalL += ch.volRampSpdL;
						else ch.volFinalL = ch.volTargetL;
					}

					if (ch.volFinalR != ch.volTargetR)
					{
						if (abs(ch.volFinalR - ch.volTargetR) >= abs(ch.volRampSpdR))
							ch.volFinalR += ch.volRampSpdR;
						else ch.volFinalR = ch.volTargetR;
					}

					if (ch.volFinalInst != ch.volTargetInst)
					{
						if (abs(ch.volFinalInst - ch.volTargetInst) >= abs(ch.volRampSpdInst))
							ch.volFinalInst += ch.volRampSpdInst;
						else ch.volFinalInst = ch.volTargetInst;
					}

					//Looping
					if (ch.loopType)
					{
						if (chPos < ch.loopStart)
							ch.loop = 0;
						else if (chPos >= ch.loopEnd)
						{
							chPos = ch.loopStart + (chPos - ch.loopStart) % ch.loopLeng;
							ch.loop = 1;
						}
					}
					else if (chPos >= ch.smpLeng)
					{
						ch.active = false;
						ch.endSmp = ch.is16Bit ? *(int16_t *)(ch.data + ((ch.smpLeng - 1) << 1)) : (int16_t)(ch.data[ch.smpLeng - 1] << 8);
						ch.samplePlaying = -1;
						ch.endCount = 0;
					}

					ch.pos = chPos;

					//Don't mix when there is no sample playing or the channel is muted
					if (ch.muted || ch.samplePlaying == -1) goto Continue;

					if (ch.startCount >= SMP_CHANGE_RAMP)
					{
						//interpolation
						if (interpolation)
						{
							int32_t prevPos = chPos;

							if (chPos > 0) prevPos -- ;

							if (ch.loop == 1 && chPos <= ch.loopStart)
								prevPos = ch.loopEnd - 1;

							int16_t prevData;
							int32_t dy;

							uint16_t ix = ch.posL16 >> 1;

							if (ch.is16Bit)
							{
								prevData = *(int16_t *)(ch.data + (prevPos << 1));
								dy = *(int16_t *)(ch.data + (chPos << 1)) - prevData;
							}
							else
							{
								prevData = ch.data[prevPos] << 8;
								dy = (ch.data[chPos] << 8) - prevData;
							}

							result = (prevData + ((dy * ix) >> INT_ACC_INTERPOL));
						}
						else
						{
							if (ch.is16Bit) result = *(int16_t *)(ch.data + (chPos << 1));
							else result = (int16_t)(ch.data[chPos] << 8);
						}

						if (ch.startCount < SMP_CHANGE_RAMP + SMP_CHANGE_RAMP)
						{
							result = result * (ch.startCount - SMP_CHANGE_RAMP) / SMP_CHANGE_RAMP;
							ch.startCount ++ ;
						}
						ch.prevSmp = result;
					}
					else
					{
						result = ch.prevSmp * (SMP_CHANGE_RAMP - ch.startCount) / SMP_CHANGE_RAMP;
						ch.startCount ++ ;
					}

					result *= amplifierFinal * masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);

					outL += result * (ch.volFinalL >> INT_ACC_RAMPING);
					outR += result * (ch.volFinalR >> INT_ACC_RAMPING);
				}
				else goto NotActived;
			}
			else
			{
				NotActived:

				ch.pos = ch.posL16 = 0;
				ch.prevSmp = 0;
				ch.startCount = 0;
			}

			Continue:

			if (ch.endCount < SMP_CHANGE_RAMP)
			{
				result = ch.endSmp * (SMP_CHANGE_RAMP - ch.endCount) / SMP_CHANGE_RAMP;
				ch.endCount ++ ;

				result *= amplifierFinal * masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);

				outL += result * (ch.volFinalL >> INT_ACC_RAMPING);
				outR += result * (ch.volFinalR >> INT_ACC_RAMPING);
			}

			i ++ ;
		}

		buffer[pos << 1] = outL >> 16;
		buffer[(pos << 1) + 1] = outR >> 16;
	}
	*/

#ifndef _MACRO_MIXER
	inline void XMPlayer::MixAudio(int16_t *buffer, uint32_t pos, int32_t samples)
	{
		int i = 0;
		while (i < numOfChannels)
		{
			int32_t posFinal = pos << 1;

			int k = 0;
			while (k < samples)
			{
				int32_t outL = 0;
				int32_t outR = 0;
				double result = 0;

				if (ch.active)
				{
					if (ch.samplePlaying != -1 && ch.samplePlaying < totalSampleNum)
					{
						int32_t chPos = ch.pos;

						if (ch.startCount >= SMP_CHANGE_RAMP)
						{
							ch.posL16 += ch.delta;
							chPos += ch.posL16 >> INT_ACC;
							ch.posL16 &= INT_MASK;
						}

						if (ch.volFinalL != ch.volTargetL)
						{
							if (abs(ch.volFinalL - ch.volTargetL) >= abs(ch.volRampSpdL))
								ch.volFinalL += ch.volRampSpdL;
							else
								ch.volFinalL = ch.volTargetL;
						}

						if (ch.volFinalR != ch.volTargetR)
						{
							if (abs(ch.volFinalR - ch.volTargetR) >= abs(ch.volRampSpdR))
								ch.volFinalR += ch.volRampSpdR;
							else
								ch.volFinalR = ch.volTargetR;
						}

						if (ch.volFinalInst != ch.volTargetInst)
						{
							if (abs(ch.volFinalInst - ch.volTargetInst) >= abs(ch.volRampSpdInst))
								ch.volFinalInst += ch.volRampSpdInst;
							else
								ch.volFinalInst = ch.volTargetInst;
						}

						if (!ch.loopType)
						{
							if (chPos >= ch.smpLeng)
							{
								ch.active = false;
								ch.endSmp = ch.is16Bit ? *(int16_t *)(ch.data + ((ch.smpLeng - 1) << 1)) : (int16_t)(ch.data[ch.smpLeng - 1] << 8);
								ch.samplePlaying = -1;
								ch.endCount = 0;
							}

							ch.pos = chPos;
						}
						else
						{
							if (chPos < ch.loopStart)
								ch.loop = 0;
							else if (chPos >= ch.loopEnd)
							{
								chPos = ch.loopStart + (chPos - ch.loopStart) % ch.loopLeng;
								ch.loop = 1;
							}

							ch.pos = chPos;
						}

						if (ch.muted || ch.samplePlaying == -1)
							goto Continue;

						if (ch.startCount >= SMP_CHANGE_RAMP)
						{
							if (interpolation)
							{
								int32_t prevPos = chPos;

								if (chPos > 0)
									prevPos--;

								if (ch.loop == 1 && chPos <= ch.loopStart)
									prevPos = ch.loopEnd - 1;

								int16_t prevData;
								int32_t dy;

								uint16_t ix = ch.posL16 >> 1;

								if (ch.is16Bit)
								{
									prevData = *(int16_t *)(ch.data + (prevPos << 1));
									dy = *(int16_t *)(ch.data + (chPos << 1)) - prevData;
									result = (prevData + ((dy * ix) >> INT_ACC_INTERPOL));
								}
								else
								{
									prevData = ch.data[prevPos] << 8;
									dy = (ch.data[chPos] << 8) - prevData;
									result = (prevData + ((dy * ix) >> INT_ACC_INTERPOL));
								}
							}
							else
							{
								if (ch.is16Bit)
									result = *(int16_t *)(ch.data + (chPos << 1));
								else
									result = (int16_t)(ch.data[chPos] << 8);
							}

							if (ch.startCount < SMP_CHANGE_RAMP + SMP_CHANGE_RAMP)
							{
								result = result * (ch.startCount - SMP_CHANGE_RAMP) / SMP_CHANGE_RAMP;
								ch.startCount++;
							} ch.prevSmp = result;
						}
						else {
							result = ch.prevSmp * (SMP_CHANGE_RAMP - ch.startCount) / SMP_CHANGE_RAMP;
							ch.startCount++;
						}

						//result *= amplifierFinal * masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);
						result *= (float)masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);

						outL = result * (ch.volFinalL >> INT_ACC_RAMPING);
						outR = result * (ch.volFinalR >> INT_ACC_RAMPING);
					}
					else
						goto ChannelNotPlaying;
				}
				else
				{
				ChannelNotPlaying:

					ch.pos = ch.posL16 = 0;
					ch.prevSmp = 0;
					ch.startCount = 0;
				}

			Continue:
				if (ch.endCount < SMP_CHANGE_RAMP)
				{
					result = ch.endSmp * (SMP_CHANGE_RAMP - ch.endCount) / SMP_CHANGE_RAMP;
					ch.endCount++;
					//result *= amplifierFinal * masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);
					result *= (float)masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);
					outL = result * (ch.volFinalL >> INT_ACC_RAMPING);
					outR = result * (ch.volFinalR >> INT_ACC_RAMPING);
				}
				else if (!ch.active)
					break;

				if (!ch.muted)
				{
					if (scopeEnabled)
					{
						int offset = scopeInt.sizePerCh * i + pos + k;
						//if (offset > scopeInt.sizePerCh * scopeInt.numOfCh)
						//	offset = 0;
						int8_t *scopeData = scopeInt.scopeData + offset;

						int scopeOut = (int)((outL >> 16) + (outR >> 16)) >> 7;
						if (scopeOut > 127)
							scopeOut = 127;
						else if (scopeOut < -128)
							scopeOut = -128;

						*scopeData = (int8_t)scopeOut;
					}

					if (buffer[posFinal] != 32767 && buffer[posFinal] != -32768)
					{
						int bufferNewL = buffer[posFinal] + ((outL >> 16) * amplifierFinal);
						if (bufferNewL > 32767)
							bufferNewL = 32767;
						else if (bufferNewL < -32768)
							bufferNewL = -32768;
						
						buffer[posFinal] = bufferNewL;
					}
					posFinal ++;

					if (buffer[posFinal] != 32767 && buffer[posFinal] != -32768)
					{
						int bufferNewR = buffer[posFinal] + ((outR >> 16) * amplifierFinal);
						if (bufferNewR > 32767)
							bufferNewR = 32767;
						else if (bufferNewR < -32768)
							bufferNewR = -32768;
						
						buffer[posFinal] = bufferNewR;
					}
					posFinal ++;
				}

				k++;
			}

			i++;
		}
	}
#else
#define MIXPREFIX\
	int k = 0;\
	while (k < samples)\
	{\
		int32_t outL = 0;\
		int32_t outR = 0;\
		double result = 0;\
\
		if (ch.active)\
		{\
			if (ch.samplePlaying != -1 && ch.samplePlaying < totalSampleNum)\
			{\
				int32_t chPos = ch.pos;\
\
				if (ch.startCount >= SMP_CHANGE_RAMP)\
				{\
					ch.posL16 += ch.delta;\
					chPos += ch.posL16 >> INT_ACC;\
					ch.posL16 &= INT_MASK;\
				}\
\
				if (ch.volFinalL != ch.volTargetL)\
				{\
					if (abs(ch.volFinalL - ch.volTargetL) >= abs(ch.volRampSpdL))\
						ch.volFinalL += ch.volRampSpdL;\
					else ch.volFinalL = ch.volTargetL;\
				}\
\
				if (ch.volFinalR != ch.volTargetR)\
				{\
					if (abs(ch.volFinalR - ch.volTargetR) >= abs(ch.volRampSpdR))\
						ch.volFinalR += ch.volRampSpdR;\
					else ch.volFinalR = ch.volTargetR;\
				}\
\
				if (ch.volFinalInst != ch.volTargetInst)\
				{\
					if (abs(ch.volFinalInst - ch.volTargetInst) >= abs(ch.volRampSpdInst))\
						ch.volFinalInst += ch.volRampSpdInst;\
					else ch.volFinalInst = ch.volTargetInst;\
				}

#define MIXNOLOOP\
	if (chPos >= ch.smpLeng)\
	{\
		ch.active = false;\
		ch.endSmp = ch.is16Bit ? *(int16_t *)(ch.data + ((ch.smpLeng - 1) << 1)) : (int16_t)(ch.data[ch.smpLeng - 1] << 8);\
		ch.samplePlaying = -1;\
		ch.endCount = 0;\
	}\
	ch.pos = chPos;

#define MIXLOOP\
	if (chPos < ch.loopStart)\
		ch.loop = 0;\
	else if (chPos >= ch.loopEnd)\
	{\
		chPos = ch.loopStart + (chPos - ch.loopStart) % ch.loopLeng;\
		ch.loop = 1;\
	}\
	ch.pos = chPos;

#define MIXPART1(A)\
	if (ch.muted || ch.samplePlaying == -1) goto A;\
\
	if (ch.startCount >= SMP_CHANGE_RAMP)\
	{

#define MIXINTERPOLINIT\
	int32_t prevPos = chPos;\
\
	if (chPos > 0) prevPos -- ;\
\
	if (ch.loop == 1 && chPos <= ch.loopStart)\
		prevPos = ch.loopEnd - 1;\
\
	int16_t prevData;\
	int32_t dy;\
\
	uint16_t ix = ch.posL16 >> 1;

#define MIXINTERPOL16BIT\
	prevData = *(int16_t *)(ch.data + (prevPos << 1));\
	dy = *(int16_t *)(ch.data + (chPos << 1)) - prevData;\
	result = (prevData + ((dy * ix) >> INT_ACC_INTERPOL));

#define MIXINTERPOL8BIT\
	prevData = ch.data[prevPos] << 8;\
	dy = (ch.data[chPos] << 8) - prevData;\
	result = (prevData + ((dy * ix) >> INT_ACC_INTERPOL));

#define MIXNEAREST16BIT\
	result = *(int16_t *)(ch.data + (chPos << 1));

#define MIXNEAREST8BIT\
	result = (int16_t)(ch.data[chPos] << 8);

#define MIXSUFFIX(A,B)\
					if (ch.startCount < SMP_CHANGE_RAMP + SMP_CHANGE_RAMP)\
					{\
						result = result * (ch.startCount - SMP_CHANGE_RAMP) / SMP_CHANGE_RAMP;\
						ch.startCount ++ ;\
					}\
					ch.prevSmp = result;\
				}\
				else\
				{\
					result = ch.prevSmp * (SMP_CHANGE_RAMP - ch.startCount) / SMP_CHANGE_RAMP;\
					ch.startCount ++ ;\
				}\
\
				result *= amplifierFinal * masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);\
\
				outL = result * (ch.volFinalL >> INT_ACC_RAMPING);\
				outR = result * (ch.volFinalR >> INT_ACC_RAMPING);\
			}\
			else goto B;\
		}\
		else\
		{\
			B:\
\
			ch.pos = ch.posL16 = 0;\
			ch.prevSmp = 0;\
			ch.startCount = 0;\
		}\
\
		A:\
\
		if (ch.endCount < SMP_CHANGE_RAMP)\
		{\
			result = ch.endSmp * (SMP_CHANGE_RAMP - ch.endCount) / SMP_CHANGE_RAMP;\
			ch.endCount ++ ;\
\
			result *= amplifierFinal * masterVolume * ch.volFinalInst / (64.0 * TOINT_SCL_RAMPING);\
\
			outL = result * (ch.volFinalL >> INT_ACC_RAMPING);\
			outR = result * (ch.volFinalR >> INT_ACC_RAMPING);\
		}\
		else if (!ch.active) break;\
\
		buffer[posFinal++] += outL >> 16;\
		buffer[posFinal++] += outR >> 16;\
\
		k ++ ;\
	}

	inline void XMPlayer::MixAudio(int16_t *buffer, uint32_t pos, int32_t samples)
	{
		int i = 0;
		while (i < numOfChannels)
		{
			int32_t posFinal = pos << 1;

			if (!ch.loopType)
			{
				if (ch.is16Bit)
				{
					if (interpolation)
					{
						MIXPREFIX
						MIXNOLOOP
						MIXPART1(C1)
						MIXINTERPOLINIT
						MIXINTERPOL16BIT
						MIXSUFFIX(C1,N1)
					}
					else
					{
						MIXPREFIX
						MIXNOLOOP
						MIXPART1(C2)
						MIXNEAREST16BIT
						MIXSUFFIX(C2,N2)
					}
				}
				else
				{
					if (interpolation)
					{
						MIXPREFIX
						MIXNOLOOP
						MIXPART1(C3)
						MIXINTERPOLINIT
						MIXINTERPOL8BIT
						MIXSUFFIX(C3,N3)
					}
					else
					{
						MIXPREFIX
						MIXNOLOOP
						MIXPART1(C4)
						MIXNEAREST8BIT
						MIXSUFFIX(C4,N4)
					}
				}
			}
			else
			{
				if (ch.is16Bit)
				{
					if (interpolation)
					{
						MIXPREFIX
						MIXLOOP
						MIXPART1(C5)
						MIXINTERPOLINIT
						MIXINTERPOL16BIT
						MIXSUFFIX(C5,N5)
					}
					else
					{
						MIXPREFIX
						MIXLOOP
						MIXPART1(C6)
						MIXNEAREST16BIT
						MIXSUFFIX(C6,N6)
					}
				}
				else
				{
					if (interpolation)
					{
						MIXPREFIX
						MIXLOOP
						MIXPART1(C7)
						MIXINTERPOLINIT
						MIXINTERPOL8BIT
						MIXSUFFIX(C7,N7)
					}
					else
					{
						MIXPREFIX
						MIXLOOP
						MIXPART1(C8)
						MIXNEAREST8BIT
						MIXSUFFIX(C8,N8)
					}
				}
			}

			i ++ ;
		}
	}
#endif

	/* Old FillBuffer function */
	/*
	void FillBufferOld(int16_t *buffer)
	{
		int i = 0;

		startTime = clock();

		while (i < bufferSize)
		{
			if (isPlaying)
				timer += timePerSample;

			if (timer >= timePerTick)
			{
				NextTick();
				UpdateChannelInfo();
				timer = fmod(timer, timePerTick);
				//timer -= timePerTick;
				UpdateTimer();
			}

			MixAudioOld(buffer, i);

			i ++ ;
		}

		endTime = clock();
		excuteTime = endTime - startTime;
	}
	*/

	void XMPlayer::FillBuffer(int16_t *buffer)
	{
		int i = 0;

		startTime = clock();

		if (scopeEnabled)
			memset(scopeInt.scopeData, 0, bufferSize * numOfChannels);
		memset(buffer, 0, bufferSize<<2);
		if (isPlaying)
		{
			while (i < bufferSize)
			{
				int mixLength = bufferSize;

				if (i + sampleToNextTick >= bufferSize)
				{
					mixLength = bufferSize - i;
					sampleToNextTick -= bufferSize - i;
				}
				else
				{
					if (sampleToNextTick)
					{
						mixLength = sampleToNextTick;
						sampleToNextTick = 0;
					}
					else
					{
						if (setPosReq)
						{
							SetPosInternal();
							setPosReq = false;
						}

						NextTick();
						UpdateChannelInfo();
						timer = fmod(timer, timePerTick);
						UpdateTimer();

						sampleToNextTick = samplePerTick;
						mixLength = sampleToNextTick;
					}

					if (i + sampleToNextTick >= bufferSize)
					{
						mixLength = bufferSize - i;
						sampleToNextTick -= bufferSize - i;
					}
					else sampleToNextTick = 0;
				}

				MixAudio(buffer, i, mixLength);

				i += mixLength;

			}
		}
		if (scopeEnabled)
			memcpy(scopes.scopeData, scopeInt.scopeData, bufferSize * numOfChannels);

		endTime = clock();
		excuteTime = endTime - startTime;
	}

#ifdef _SDL2
	void XMPlayer::CallbackObject::WriteBufferCallback(uint8_t *buffer, int length)
	{
		parent->FillBuffer((int16_t *)buffer);
	}

	void XMPlayer::CallbackObject::ForwardCallback(void *userdata, Uint8 *stream, int len)
	{
		static_cast<CallbackObject *>(userdata)->WriteBufferCallback(stream, len);
	}
#endif

	int16_t XMPlayer::GetPatternLength(int patternNum)
	{
		if (patternNum < 0 || patternNum >= numOfPatterns)
			return -1;

		return *(int16_t *)(patternData + patternAddr[patternNum]);
	}

	void XMPlayer::GetCurrentStat(Stat &dest)
	{
		if (!songLoaded)
			return;

		dest.songName = songName;
		dest.trackerName = trackerName;
		dest.trackerVersion = trackerVersion;
		dest.songLength = songLength;
		dest.numOfPatterns = numOfPatterns;
		dest.numOfInstruments = numOfInstruments;
		dest.numOfChannels = numOfChannels;

		dest.speed = speed;
		dest.tempo = tempo;

		dest.pos = curPos;
		dest.row = curRow;

		int curPattern = orderTable[curPos];
		dest.pattern = curPattern;
		dest.patternLen = GetPatternLength(curPattern);
		dest.patternOrder = orderTable;

		dest.channels = channels;
		dest.instruments = instruments;
		dest.samples = samples;
	}

	bool XMPlayer::IsLoaded()
	{
		return songLoaded;
	}

	bool XMPlayer::IsPlaying()
	{
		return isPlaying;
	}

	void XMPlayer::UnmuteAll()
	{
		if (!songLoaded || channels == NULL)
			return;

		int i = 0;
		while (i < numOfChannels)
			channels[i++].muted = false;
	}

	void XMPlayer::SetAmp(float value)
	{
		if (value < 0.1F)
			value = 0.1F;
		if (value > 32.0F)
			value = 32.0F;

		amplifier = value;
		RecalcAmp();
	}

	void XMPlayer::SetPos(int16_t pos)
	{
		// Use this method to make sure it's thread safe...
		setPosReqTo = pos;

		if (!isPlaying)
		{
			SetPosInternal();
			setPosReq = false;
		}
		else
			setPosReq = true;
	}

	XMPlayer::Note XMPlayer::GetNoteExt(int16_t pos, int16_t row, uint8_t col)
	{
		Note thisNote;

		if (pos < songLength)
		{
			while (row >= *(int16_t *)(patternData + patternAddr[orderTable[pos]]))
			{
				row -= *(int16_t *)(patternData + patternAddr[orderTable[pos]]);
				pos ++;
				if (pos >= songLength) goto End;
			}
			while (row < 0)
			{
				pos --;
				if (pos < 0) goto End;
				row += *(int16_t *)(patternData + patternAddr[orderTable[pos]]);
			}
		}
		else
			goto End;

		if (col < numOfChannels)
		{
			thisNote = *(Note *)(patternData + patternAddr[orderTable[pos]] + ROW_SIZE_XM * row + col * NOTE_SIZE_XM + 2);
		}
		else
		{
		End:
			thisNote.note = 255;
			thisNote.instrument = thisNote.volCmd = thisNote.effect = thisNote.parameter = 0;
		}

		return thisNote;
	}

	void XMPlayer::PlayPause(bool play)
	{
		if (isPlaying != play)
		{
			isPlaying = play;
#ifdef _SDL2
			CreateAudioDevice();

			if (deviceID != 0)
				SDL_PauseAudioDevice(deviceID, !isPlaying);
#endif
#ifdef _SFML
			CreateSoundStream();

			if (customStream != NULL)
			{
				if (isPlaying)
					customStream->play();
				else
					customStream->stop();
			}
#endif
		}
	}

	bool XMPlayer::PlayModule()
	{
		if (!songLoaded)
			return false;

		/*
		int i = 0;
		while (i < 2)
		{
			if (SndBuffer[i] != nullptr)
				free SndBuffer[i]);

			SndBuffer[i] = (int16_t *)malloc(bufferSize);
			if (SndBuffer[i] == nullptr) return false;

			i ++ ;
		}
		*/

#ifdef _SDL2
		callbackObject.parent = this;

		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
			return false;

		CreateAudioDevice();

		if (deviceID != 0)
		{
			bufferSize = actualSpec.samples;
			SDL_PauseAudioDevice(deviceID, 0);
			isPlaying = true;
		}
		else
			return false;
#endif
#ifdef _SFML
		CreateSoundStream();
		
		if (!isPlaying)
		{
			isPlaying = true;
			customStream->play();
		}
#endif

		return true;
	}

	bool XMPlayer::StopModule()
	{
#ifdef _SDL2
		SDL_CloseAudioDevice(deviceID);
		deviceID = 0;
#endif
#ifdef _SFML
		if (customStream != NULL)
			customStream->stop();
#endif
		isPlaying = false;
		return true;
	}

	void XMPlayer::CleanUp()
	{
		isPlaying = false;
		songLoaded = false;
		StopModule();

#ifdef _SFML
		if (customStream != NULL)
		{
			delete customStream;
			customStream = NULL;
		}
#endif

		if (channels != NULL)
		{
			free(channels);
			channels = NULL;
		}

		if (patternData != NULL)
		{
			free(patternData);
			patternData = NULL;
		}

		if (instruments != NULL)
		{
			free(instruments);
			instruments = NULL;
		}

		if (samples != NULL)
		{
			free(samples);
			samples = NULL;
		}

		if (scopeInt.scopeData != NULL)
		{
			free(scopeInt.scopeData);
			scopeInt.scopeData = NULL;
		}
		if (scopes.scopeData != NULL)
		{
			free(scopes.scopeData);
			scopes.scopeData = NULL;
		}

		/*
		int i = 0;
		while (i < 2)
		{
			if (SndBuffer[i] != nullptr)
				free SndBuffer[i]);
		}
		*/
	}

	XMPlayer::~XMPlayer()
	{
		CleanUp();
	}
}
