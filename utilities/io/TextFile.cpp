#include <v4d.h>

using namespace v4d::io;

std::string TextFile::Load() {
	ASCIIFile::Load([](ASCIIFile*){});
	return text.str();
}

void TextFile::ReadFromFile() {
	std::ifstream file(filePath);
	std::string line;
	while (std::getline(file, line)) {
		if (line.length() == 0) continue;
		text << line << std::endl;
	}
	file.close();
}

void TextFile::WriteToFile() {
	// Write all lines to file
	std::ofstream file(filePath);
	std::string line;
	while (std::getline(text, line)) {
		file << line << std::endl;
	}
	file.close();
}
