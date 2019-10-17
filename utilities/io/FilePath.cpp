#include <v4d.h>

using namespace v4d::io;


FilePath::FilePath(const std::string& filePath) : filePath(filePath) {}

FilePath::~FilePath() {}

FilePath::operator std::string () {
	return filePath.string();
}

FilePath& FilePath::AutoCreateFile() {
	if (CreateDirectory(filePath.parent_path().string())) {
		CreateFile(filePath.string());
	}
	return *this;
}

bool FilePath::Delete() {
	return std::filesystem::remove(filePath);
}

bool FilePath::CreateDirectory(const std::string& path) {
	if (!std::filesystem::exists(path)) {
		// Does not exist, create it
		return std::filesystem::create_directories(path);
	} else if (std::filesystem::is_directory(path)) {
		return true; // Directory already exists
	} else {
		return false; // Exists but Is Not a Directory
	}
}

bool FilePath::CreateFile(const std::string& path) {
	if (!std::filesystem::exists(path)) {
		// Does not exist, create the file
		std::ofstream newFile(path, std::fstream::out);
		bool created = newFile.is_open();
		if (created) newFile.close();
		return created;
	} else if (std::filesystem::is_regular_file(path)) {
		return true; // File already exists
	}
	return false; // Exists but is Not a regular file
}

bool FilePath::DirectoryExists(const std::string& path) {
	if (!std::filesystem::exists(path)) {
		return false;
	} else if (std::filesystem::is_directory(path)) {
		return true;
	} else {
		return false; // Is Not a Directory
	}
}

bool FilePath::FileExists(const std::string& path/*, bool supportLinks*/) {
	if (!std::filesystem::exists(path)) {
		return false; // Does not exist
	} else if (std::filesystem::is_regular_file(path)) {
		return true;
	// } else if (supportLinks && std::filesystem::is_symlink(path)) {
	// 	return true;
	} else {
		return false; // Exists, but Is Not a File
	}
}

bool FilePath::DeleteDirectory(const std::string& path, bool recursive) {
	if (!std::filesystem::exists(path)) {
		// Does not exist
		return false;
	} else if (std::filesystem::is_directory(path)) {
		// Exists and is a directory, Delete it !
		if (recursive) return std::filesystem::remove_all(path) > 0;
		else return std::filesystem::remove(path);
	}
	return false;
}

