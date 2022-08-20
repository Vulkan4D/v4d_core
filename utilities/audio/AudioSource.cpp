#include "AudioSource.h"

namespace v4d::audio {
	
	AudioSource::AudioSystem::AudioSystem() {
		openALDevice = alcOpenDevice(nullptr);
		assert(openALDevice);
		alcCall(alcCreateContext, openALContext, openALDevice, openALDevice, nullptr);
		assert(openALContext);
		alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, openALContext);
		assert(contextMadeCurrent == ALC_TRUE);
	}
	AudioSource::AudioSystem::~AudioSystem() {
		alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
		alcCall(alcDestroyContext, openALDevice, openALContext);
		alcCloseDevice(openALDevice);
	}
	
	std::shared_ptr<AudioSource::AudioBuffer> AudioSource::GetAudioBuffer(const char* waveFilePath) {
		static std::unordered_map<std::string, std::weak_ptr<AudioBuffer>> audioBuffers {};
		std::shared_ptr<AudioSource::AudioBuffer> buffer = nullptr;
		if (!audioBuffers.contains(waveFilePath) || !(buffer = audioBuffers.at(waveFilePath).lock())) {
			audioBuffers.emplace(waveFilePath, buffer = std::make_shared<AudioBuffer>(waveFilePath));
		}
		return buffer;
	}
	
	AudioSource::AudioSource(const char* waveFilePath) : AudioSource(std::vector<const char*>{waveFilePath}) {}
	AudioSource::AudioSource(const std::vector<const char*>& waveFiles) {
		for (auto& waveFile : waveFiles) {
			buffers.push_back(GetAudioBuffer(waveFile));
		}
		if (buffers.size() > 0) {
			alCall(alGenSources, 1, &source);
			alCall(alSourcef, source, AL_PITCH, 1.0f);
			alCall(alSourcef, source, AL_GAIN, 1.0f);
			alCall(alSource3f, source, AL_POSITION, 0, 0, 0);
			alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
			alCall(alSourcei, source, AL_LOOPING, AL_FALSE);
			alCall(alSourcei, source, AL_BUFFER, buffers[0]->GetHandle());
		}
	}

	AudioSource::~AudioSource() {
		while(source && state == AL_PLAYING) alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);
		if (source) alCall(alDeleteSources, 1, &source);
	}
	
	bool AudioSource::load_wav_file_header(std::ifstream& file,
							std::uint8_t& channels,
							std::int32_t& sampleRate,
							std::uint8_t& bitsPerSample,
							ALsizei& size)
	{
		char buffer[4];
		if(!file.is_open())
			return false;

		// the RIFF
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read RIFF" << std::endl;
			return false;
		}
		if(std::strncmp(buffer, "RIFF", 4) != 0) {
			std::cerr << "ERROR: file is not a valid WAVE file (header doesn't begin with RIFF)" << std::endl;
			return false;
		}

		// the size of the file
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read size of file" << std::endl;
			return false;
		}

		// the WAVE
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read WAVE" << std::endl;
			return false;
		}
		if(std::strncmp(buffer, "WAVE", 4) != 0) {
			std::cerr << "ERROR: file is not a valid WAVE file (header doesn't contain WAVE)" << std::endl;
			return false;
		}

		// "fmt/0"
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read fmt/0" << std::endl;
			return false;
		}

		// this is always 16, the size of the fmt data chunk
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read the 16" << std::endl;
			return false;
		}

		// PCM should be 1?
		if(!file.read(buffer, 2)) {
			std::cerr << "ERROR: could not read PCM" << std::endl;
			return false;
		}

		// the number of channels
		if(!file.read(buffer, 2)) {
			std::cerr << "ERROR: could not read number of channels" << std::endl;
			return false;
		}
		channels = convert_to_int(buffer, 2);

		// sample rate
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read sample rate" << std::endl;
			return false;
		}
		sampleRate = convert_to_int(buffer, 4);

		// (sampleRate * bitsPerSample * channels) / 8
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read (sampleRate * bitsPerSample * channels) / 8" << std::endl;
			return false;
		}

		// ?? dafaq
		if(!file.read(buffer, 2)) {
			std::cerr << "ERROR: could not read dafaq" << std::endl;
			return false;
		}

		// bitsPerSample
		if(!file.read(buffer, 2)) {
			std::cerr << "ERROR: could not read bits per sample" << std::endl;
			return false;
		}
		bitsPerSample = convert_to_int(buffer, 2);

		// data chunk header "data"
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read data chunk header" << std::endl;
			return false;
		}
		if(std::strncmp(buffer, "data", 4) != 0) {
			std::cerr << "ERROR: file is not a valid WAVE file (doesn't have 'data' tag)" << std::endl;
			return false;
		}

		// size of data
		if(!file.read(buffer, 4)) {
			std::cerr << "ERROR: could not read data size" << std::endl;
			return false;
		}
		size = convert_to_int(buffer, 4);

		/* cannot be at the end of file */
		if(file.eof()) {
			std::cerr << "ERROR: reached EOF on the file" << std::endl;
			return false;
		}
		if(file.fail()) {
			std::cerr << "ERROR: fail state set on the file" << std::endl;
			return false;
		}

		return true;
	}

	bool AudioSource::load_wav(const std::string& filename,
						std::uint8_t& channels,
						std::int32_t& sampleRate,
						std::uint8_t& bitsPerSample,
						std::vector<char>& data)
	{
		std::ifstream in(filename, std::ios::binary);
		if(!in.is_open()) {
			std::cerr << "ERROR: Could not open \"" << filename << "\"" << std::endl;
			return false;
		}
		ALsizei size;
		if(!load_wav_file_header(in, channels, sampleRate, bitsPerSample, size)) {
			std::cerr << "ERROR: Could not load wav header of \"" << filename << "\"" << std::endl;
			return false;
		}
		data.resize(size);
		in.read(data.data(), size);
		return true;
	}
	
	
	AudioSource::AudioBuffer::AudioBuffer(const char* waveFilePath) {
		std::vector<char> soundData;
		if(!load_wav(waveFilePath, channels, sampleRate, bitsPerSample, soundData)) {
			LOG_ERROR("ERROR: Could not load wav file " << waveFilePath)
			return;
		}
		alCall(alGenBuffers, 1, &buffer);
		if(channels == 1 && bitsPerSample == 8)
			format = AL_FORMAT_MONO8;
		else if(channels == 1 && bitsPerSample == 16)
			format = AL_FORMAT_MONO16;
		else if(channels == 2 && bitsPerSample == 8)
			format = AL_FORMAT_STEREO8;
		else if(channels == 2 && bitsPerSample == 16)
			format = AL_FORMAT_STEREO16;
		else {
			LOG_ERROR("ERROR: unrecognised wave format: " << channels << " channels, " << bitsPerSample << " bps");
			return;
		}
		alCall(alBufferData, buffer, format, soundData.data(), soundData.size(), sampleRate);
	}

	AudioSource::AudioBuffer::~AudioBuffer() {
		if (buffer) alCall(alDeleteBuffers, 1, &buffer);
	}
}
