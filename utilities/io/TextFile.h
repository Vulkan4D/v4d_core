#pragma once
#include <v4d.h>

namespace v4d::io {
	class V4DLIB TextFile : public ASCIIFile {
	public: // Virtual methods
		using ASCIIFile::ASCIIFile;
		using ASCIIFile::Load;
	
		std::stringstream text {};

		std::string Load();
		
		void ReadFromFile();
		void WriteToFile();
		
		static auto Instance(FilePath filePath, std::optional<int> autoReloadInterval = std::nullopt)
			STATIC_CLASS_INSTANCES((std::string)filePath, TextFile, filePath, autoReloadInterval)

	};
}
