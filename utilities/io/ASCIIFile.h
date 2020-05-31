#pragma once
#include <v4d.h>

namespace v4d::io {
	
	class V4DLIB ASCIIFile : public FilePath {
		typedef std::function<void(ASCIIFile*)> ReloadCallbackFunc;
		
	private: // Class members
	
		int autoReloadInterval;
		ReloadCallbackFunc reloadCallback = [](ASCIIFile*){};

		static const int DEFAULT_AUTORELOAD_INTERVAL = 0; // in milliseconds, 0 = disabled
		static int globalAutoReloadInterval;

		mutable std::recursive_mutex mu;
		std::thread* autoReloadThread = nullptr;
		double lastWriteTimeCache = 0;

		void StartAutoReloadThread();

		virtual void ReadFromFile() = 0;
		virtual void WriteToFile() = 0;

	public: // methods
	
		ASCIIFile(const std::string& filePath, std::optional<int> autoReloadInterval = std::nullopt);
		virtual ~ASCIIFile();

		void SetAutoReloadInterval(int interval);

		bool FileHasChanged() const;

		virtual void Load(ReloadCallbackFunc&& callback);
		
	};
}
