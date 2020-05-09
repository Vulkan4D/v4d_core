#pragma once
#include <v4d.h>

namespace v4d::io {
	
	class V4DLIB StringListFile : public FilePath {
	protected: // Class members
		typedef std::function<void(StringListFile*)> ReloadCallbackFunc;
	
		int autoReloadInterval;
		ReloadCallbackFunc reloadCallback = [](StringListFile*){};

	protected: // methods

		void StartAutoReloadThread();

	public: // Virtual methods
	
		std::vector<std::string> lines {};

		StringListFile(const std::string& filePath, std::optional<int> autoReloadInterval);
		virtual ~StringListFile();

		std::vector<std::string>& Load();
		void Load(ReloadCallbackFunc&& callback);

		void SetAutoReloadInterval(int interval);

		bool FileHasChanged() const;

		void ReadLinesFromFile();
		void WriteLinesToFile();

		static auto Instance(FilePath filePath, std::optional<int> autoReloadInterval = std::nullopt)
			STATIC_CLASS_INSTANCES((std::string)filePath, StringListFile, filePath, autoReloadInterval)

	private: // members

		static const int DEFAULT_AUTORELOAD_INTERVAL = 0; // in milliseconds, 0 = disabled
		static int globalAutoReloadInterval;

		mutable std::recursive_mutex mu;
		std::thread* autoReloadThread = nullptr;
		double lastWriteTimeCache = 0;

	};
}
