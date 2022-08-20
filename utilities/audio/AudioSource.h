#pragma once

#include <v4d.h>

// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/

namespace v4d::audio {	
	class V4DLIB AudioSource {
	
	#pragma region Helpers
		
		static std::int32_t convert_to_int(char* buffer, std::size_t len) {
			std::int32_t a = 0;
			if(std::endian::native == std::endian::little)
				std::memcpy(&a, buffer, len);
			else
				for(std::size_t i = 0; i < len; ++i)
					reinterpret_cast<char*>(&a)[3 - i] = buffer[i];
			return a;
		}

		static bool load_wav_file_header(std::ifstream& file,
								uint8_t& channels,
								int32_t& sampleRate,
								uint8_t& bitsPerSample,
								int32_t& size);

		static bool load_wav(const std::string& filename,
							uint8_t& channels,
							int32_t& sampleRate,
							uint8_t& bitsPerSample,
							std::vector<char>& data);
		
		class V4DLIB AudioBuffer {
			uint8_t channels;
			int32_t sampleRate;
			uint8_t bitsPerSample;
			int32_t format;
			uint32_t buffer = 0;
		public:
			AudioBuffer(const char* waveFilePath);
			~AudioBuffer();
			AudioBuffer& operator=(AudioBuffer&& other) {
				if (&other != this) {
					channels = other.channels;
					sampleRate = other.sampleRate;
					bitsPerSample = other.bitsPerSample;
					format = other.format;
					buffer = other.buffer;
					other.buffer = 0;
				}
				return *this;
			}
			AudioBuffer(AudioBuffer&& other) {
				*this = std::move(other);
			}
			AudioBuffer(const AudioBuffer& other) = delete;
			uint32_t GetHandle() const {
				return buffer;
			}
		};
		
	#pragma endregion
	
	protected:

		std::vector<std::shared_ptr<AudioBuffer>> buffers;
		uint32_t source = 0;
		int32_t state;
		
		static std::shared_ptr<AudioSource::AudioBuffer> GetAudioBuffer(const char* waveFilePath);
		
	public:
		
		// SINGLETON
		class V4DLIB AudioSystem {
		public:
			AudioSystem();
			~AudioSystem();
		};
		
		AudioSource(const char* waveFilePath);
		AudioSource(const std::vector<const char*>& waveFiles);
		AudioSource(const AudioSource&) = delete;
		AudioSource(AudioSource&&) = delete;
		
		virtual ~AudioSource();
		
		void Play();
		void Stop();
		void Select(uint index);
		void SetPitch(float pitch);
		void SetGain(float gain);
		void SetPosition(float x, float y, float z);
		void SetPosition(const glm::vec3& position);
		void SetVelocity(float x, float y, float z);
		void SetVelocity(const glm::vec3& velocity);
		void SetLooping(bool looping);
	};
}
