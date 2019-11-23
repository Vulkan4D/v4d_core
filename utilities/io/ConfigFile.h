#pragma once
#include <v4d.h>

namespace v4d::io {
	
	class V4DLIB ConfigFile : public FilePath {
	public:
		enum CONFTYPE {
			NONE,
			STRING,
			BOOL,
			LONG,
			INT,
			DOUBLE,
			FLOAT,
		};

		struct Conf {
			ConfigFile* configFile;
			std::string name;
			void* ptr;
			CONFTYPE type = NONE;

			void ReadValue(std::string value) {
				try {
					switch (type) {
						case STRING:
							*(std::string*)ptr = value;
						break;
						case BOOL:
							v4d::String::ToLowerCase(value);
							bool val;
							if 		(value == "yes" || value == "true"  || value == "1" || value == "on") val = true;
							else if (value == "no"  || value == "false" || value == "0" || value == "off") val = false;
							else val = value.length() > 0;
							*(bool*)ptr = val;
						break;
						case LONG:
							*(long*)ptr = std::stol(value);
						break;
						case INT:
							*(int*)ptr = std::stoi(value);
						break;
						case DOUBLE:
							*(double*)ptr = std::stod(value);
						break;
						case FLOAT:
							*(float*)ptr = std::stof(value);
						break;
						case NONE:
							LOG_ERROR("Invalid Conf type")
						break;
					}
				} catch (...) {
					LOG_WARN_VERBOSE("Unable to parse field '" << name << "' in config file " << configFile->filePath)
				}
			}
			std::string WriteValue() {
				switch (type) {
					case STRING:
						return *(std::string*)ptr;
					break;
					case BOOL:
						return (*(bool*)ptr)? "yes" : "no";
					break;
					case LONG:
						return std::to_string(*(long*)ptr);
					break;
					case INT:
						return std::to_string(*(int*)ptr);
					break;
					case DOUBLE:
						return std::to_string(*(double*)ptr);
					break;
					case FLOAT:
						return std::to_string(*(float*)ptr);
					break;
					case NONE:
						LOG_ERROR("Invalid Conf type")
					break;
				}
				return "";
			}
		};

		template<typename T>
		CONFTYPE constexpr ConfType() const {
			if constexpr (std::is_same_v<T, std::string>) 
									 return STRING;
			if constexpr (std::is_same_v<T, bool>) 
									 return BOOL;
			if constexpr (std::is_same_v<T, long>) 
									 return LONG;
			if constexpr (std::is_same_v<T, int>) 
									 return INT;
			if constexpr (std::is_same_v<T, double>) 
									 return DOUBLE;
			if constexpr (std::is_same_v<T, float>) 
									 return FLOAT;
			return NONE;
		}

	protected: // Class members

		int autoReloadInterval;

	public: // Virtual methods

		virtual ~ConfigFile();

	private: // Pure-Virtual methods

		virtual void ReadConfig() = 0;
		virtual void WriteConfig() = 0;

	public: // non-virtual methods

		void Load();

		void SetAutoReloadInterval(int interval);

		bool FileHasChanged() const;

	private: // members

		static const int DEFAULT_AUTORELOAD_INTERVAL = 0; // in milliseconds, 0 = disabled
		static int globalAutoReloadInterval;

		mutable std::recursive_mutex mu;
		std::thread* autoReloadThread = nullptr;
		double lastWriteTimeCache = 0;

	protected: // methods

		ConfigFile(const std::string& filePath, std::optional<int> autoReloadInterval);

		void StartAutoReloadThread();

		const std::regex INI_REGEX_COMMENT	{R"(^(;|#|//).*$)"};
		const std::regex INI_REGEX_SECTION	{R"(^\[(\w+)\]$)"};
		const std::regex INI_REGEX_CONF		{R"(^(\w+)\s*[=:]\s*(.*)$)"};

		void ReadFromINI(const std::string& section, std::vector<Conf> configs, bool writeIfNotExists = false);
		void WriteToINI(const std::string& section, std::vector<Conf> configs);

	};
}

#define __CONFIGFILE_RW_INI_EACH_ARG(x) Conf{this, #x, &x, ConfType<decltype(x)>()},
#define CONFIGFILE_READ_FROM_INI(section, ...) ReadFromINI(section, { FOR_EACH(__CONFIGFILE_RW_INI_EACH_ARG, __VA_ARGS__) }, false);
#define CONFIGFILE_READ_FROM_INI_WRITE(section, ...) ReadFromINI(section, { FOR_EACH(__CONFIGFILE_RW_INI_EACH_ARG, __VA_ARGS__) }, true);
#define CONFIGFILE_WRITE_TO_INI(section, ...) WriteToINI(section, { FOR_EACH(__CONFIGFILE_RW_INI_EACH_ARG, __VA_ARGS__) } );
#define CONFIGFILE_STRUCT(className) \
	className(const std::string& filePath, std::optional<int> autoReloadInterval) : ConfigFile(filePath, autoReloadInterval) {} \
	static auto Instance(FilePath filePath, std::optional<int> autoReloadInterval = std::nullopt) \
		STATIC_CLASS_INSTANCES((std::string)filePath, className, filePath, autoReloadInterval)

