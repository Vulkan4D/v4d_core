#include "AudioSource.h"

namespace v4d::audio {
	
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
		static AudioSystem audioSystem {};
		
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
}
