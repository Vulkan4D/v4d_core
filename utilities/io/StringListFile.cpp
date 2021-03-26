#include "StringListFile.h"
#include <fstream>

using namespace v4d::io;

std::vector<std::string>& StringListFile::Load() {
	ASCIIFile::Load([](ASCIIFile*){});
	return lines;
}

void StringListFile::ReadFromFile() {
	std::ifstream file(filePath);
	std::string line;
	while (std::getline(file, line)) {
		v4d::String::Trim(line);
		if (line.length() == 0) continue;
		lines.push_back(line);
	}
	file.close();
}

void StringListFile::WriteToFile() {
	std::ofstream file(filePath);
	for (auto line : lines) {
		file << line << std::endl;
	}
	file.close();
}
