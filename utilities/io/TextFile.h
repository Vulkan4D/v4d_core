#pragma once

#include <v4d.h>
#include <optional>
#include <string>
#include <sstream>
#include "utilities/io/FilePath.h"
#include "utilities/io/ASCIIFile.h"

namespace v4d::io {
	class V4DLIB TextFile : public ASCIIFile {
	public: // Virtual methods
		using ASCIIFile::ASCIIFile;
		using ASCIIFile::Load;
	
		std::stringstream text {};

		std::string Load();
		
		void ReadFromFile();
		void WriteToFile();
		
		static std::shared_ptr<TextFile> Instance(FilePath filePath, std::optional<int> autoReloadInterval = std::nullopt);
	};
}
