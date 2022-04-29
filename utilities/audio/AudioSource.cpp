#include "AudioSource.h"

namespace v4d::audio {
	AudioSource::AudioSource(const char* waveFilePath) {
		static AudioSystem audioSystem {};
		
		static std::unordered_map<std::string, std::weak_ptr<AudioBuffer>> audioBuffers {};
		if (!audioBuffers.contains(waveFilePath) || !(buffer = audioBuffers.at(waveFilePath).lock())) {
			audioBuffers.emplace(waveFilePath, buffer = std::make_shared<AudioBuffer>(waveFilePath));
		}
		
		alCall(alGenSources, 1, &source);
		alCall(alSourcef, source, AL_PITCH, 1.0f);
		alCall(alSourcef, source, AL_GAIN, 1.0f);
		alCall(alSource3f, source, AL_POSITION, 0, 0, 0);
		alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
		alCall(alSourcei, source, AL_LOOPING, AL_FALSE);
		alCall(alSourcei, source, AL_BUFFER, buffer->GetHandle());
	}

	AudioSource::~AudioSource() {
		while(source && state == AL_PLAYING) alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);
		if (source) alCall(alDeleteSources, 1, &source);
	}
}
