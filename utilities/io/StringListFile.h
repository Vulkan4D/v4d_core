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
		
		static std::shared_ptr<StringListFile> Instance(FilePath filePath, std::optional<int> autoReloadInterval = std::nullopt);
	};
}
