#pragma once

#include <v4d.h>
#include <string>
#include <optional>
#include <vector>
#include "utilities/io/FilePath.h"
#include "utilities/io/ASCIIFile.h"

namespace v4d::io {
	class V4DLIB StringListFile : public ASCIIFile {
	public: // Virtual methods
		using ASCIIFile::ASCIIFile;
		using ASCIIFile::Load;
	
		std::vector<std::string> lines {};

		std::vector<std::string>& Load();
		
		void ReadFromFile();
		void WriteToFile();
		
		static auto Instance(FilePath filePath, std::optional<int> autoReloadInterval = std::nullopt)
			STATIC_CLASS_INSTANCES((std::string)filePath, StringListFile, filePath, autoReloadInterval)

	};
}
