#pragma once

#include <v4d.h>

#include <AL/al.h>
#include <AL/alc.h>

// https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/

namespace v4d::audio {
	class V4DLIB AudioSource {
	
	#pragma region Helpers
		
		#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
		static bool check_al_errors(const std::string& filename, const std::uint_fast32_t line) {
			ALenum error = alGetError();
			if(error != AL_NO_ERROR) {
				std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
				switch(error) {
				case AL_INVALID_NAME:
					std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
					break;
				case AL_INVALID_ENUM:
					std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
					break;
				case AL_INVALID_VALUE:
					std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
					break;
				case AL_INVALID_OPERATION:
					std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
					break;
				case AL_OUT_OF_MEMORY:
					std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
					break;
				default:
					std::cerr << "UNKNOWN AL ERROR: " << error;
				}
				std::cerr << std::endl;
				return false;
			}
			return true;
		}
		template<typename alFunction, typename... Params>
		static auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
		->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
		{
			auto ret = function(std::forward<Params>(params)...);
			check_al_errors(filename, line);
			return ret;
		}
		template<typename alFunction, typename... Params>
		static auto alCallImpl(const char* filename, const std::uint_fast32_t line, alFunction function, Params... params)
		->typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
		{
			function(std::forward<Params>(params)...);
			return check_al_errors(filename, line);
		}
		#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)
		static bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device) {
			ALCenum error = alcGetError(device);
			if(error != ALC_NO_ERROR) {
				std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n" ;
				switch(error) {
				case ALC_INVALID_VALUE:
					std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
					break;
				case ALC_INVALID_DEVICE:
					std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
					break;
				case ALC_INVALID_CONTEXT:
					std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
					break;
				case ALC_INVALID_ENUM:
					std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
					break;
				case ALC_OUT_OF_MEMORY:
					std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
					break;
				default:
					std::cerr << "UNKNOWN ALC ERROR: " << error;
				}
				std::cerr << std::endl;
				return false;
			}
			return true;
		}

		template<typename alcFunction, typename... Params>
		static auto alcCallImpl(const char* filename, const std::uint_fast32_t line, alcFunction function, ALCdevice* device, Params... params)
		->typename std::enable_if_t<std::is_same_v<void,decltype(function(params...))>,bool>
		{
			function(std::forward<Params>(params)...);
			return check_alc_errors(filename,line,device);
		}

		template<typename alcFunction, typename ReturnType, typename... Params>
		static auto alcCallImpl(const char* filename, const std::uint_fast32_t line, alcFunction function, ReturnType& returnValue, ALCdevice* device, Params... params)
		->typename std::enable_if_t<!std::is_same_v<void,decltype(function(params...))>,bool>
		{
			returnValue = function(std::forward<Params>(params)...);
			return check_alc_errors(filename,line,device);
		}

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
								std::uint8_t& channels,
								std::int32_t& sampleRate,
								std::uint8_t& bitsPerSample,
								ALsizei& size);

		static bool load_wav(const std::string& filename,
							std::uint8_t& channels,
							std::int32_t& sampleRate,
							std::uint8_t& bitsPerSample,
							std::vector<char>& data);
		
		class V4DLIB AudioBuffer {
			std::uint8_t channels;
			std::int32_t sampleRate;
			std::uint8_t bitsPerSample;
			ALenum format;
			ALuint buffer = 0;
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
			ALuint GetHandle() const {
				return buffer;
			}
		};
		
	#pragma endregion
	
	protected:

		std::vector<std::shared_ptr<AudioBuffer>> buffers;
		ALuint source = 0;
		ALint state = AL_INITIAL;
		
		static std::shared_ptr<AudioSource::AudioBuffer> GetAudioBuffer(const char* waveFilePath);
		
	public:
		
		class V4DLIB AudioSystem {
			ALCdevice* openALDevice = nullptr;
			ALCboolean contextMadeCurrent = false;
			ALCcontext* openALContext = nullptr;
		public:
			AudioSystem();
			~AudioSystem();
		};
		
		AudioSource(const char* waveFilePath);
		AudioSource(const std::vector<const char*>& waveFiles);
		AudioSource(const AudioSource&) = delete;
		AudioSource(AudioSource&&) = delete;
		
		virtual ~AudioSource();
		
		void Play() {
			if (source) {
				alCall(alSourceStop, source);
				alCall(alSourcePlay, source);
				state = AL_PLAYING;
			}
		}
		
		void Stop() {
			if (source) {
				alCall(alSourceStop, source);
				state = AL_STOPPED;
			}
		}
		
		void Select(uint index) {
			if (source) {
				assert(buffers.size());
				alCall(alSourcei, source, AL_BUFFER, buffers[index % buffers.size()]->GetHandle());
			}
		}
		
		void SetPitch(float pitch) {
			if (source) {
				alCall(alSourcef, source, AL_PITCH, pitch);
			}
		}
		
		void SetGain(float gain) {
			if (source) {
				alCall(alSourcef, source, AL_GAIN, gain);
			}
		}
		
		void SetPosition(float x, float y, float z) {
			if (source) {
				alCall(alSource3f, source, AL_POSITION, x, y, z);
			}
		}
		void SetPosition(const glm::vec3& position) {
			if (source) {
				SetPosition(position);
			}
		}
		
		void SetVelocity(float x, float y, float z) {
			if (source) {
				alCall(alSource3f, source, AL_VELOCITY, x, y, z);
			}
		}
		void SetVelocity(const glm::vec3& velocity) {
			if (source) {
				SetVelocity(velocity);
			}
		}
		
		void SetLooping(bool looping) {
			if (source) {
				alCall(alSourcei, source, AL_LOOPING, looping);
			}
		}
	};
}
