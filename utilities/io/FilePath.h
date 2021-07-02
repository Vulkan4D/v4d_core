#pragma once

#include <v4d.h>
#include <filesystem>
#include <string>

#define PATH_MAX_STRING_SIZE 256

namespace v4d::io {
	class V4DLIB FilePath {
	protected:
		std::filesystem::path filePath;

	public:

		FilePath(const std::string& filePath);
		FilePath(const char* filePath);

		virtual ~FilePath();

		operator std::string () const;
		
		bool operator==(const v4d::io::FilePath &other) const;
		bool operator!=(const v4d::io::FilePath &other) const;

		FilePath& AutoCreateFile();

		virtual bool Delete();

		std::string GetExtension() const;
		std::string GetParentPath() const;
		double GetLastWriteTime() const;
		
		bool Exists() const {
			return FileExists(filePath.c_str());
		}

	public: // Static methods

		static bool CreateDirectory(const std::string& path);

		static bool CreateFile(const std::string& path);

		static bool DirectoryExists(const std::string& path);

		static bool FileExists(const std::string& path/*, bool supportLinks = false*/);

		static bool DeleteDirectory(const std::string& path, bool recursive = false);

	};
}

namespace std {
	template<> struct hash<v4d::io::FilePath> {
		std::size_t operator()(const v4d::io::FilePath& s) const noexcept {
			return std::hash<std::string>()((std::string)s);
		}
	};
}
