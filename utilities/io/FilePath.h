#pragma once

#define PATH_MAX_STRING_SIZE 256

namespace v4d::io {
	class V4DLIB FilePath {
	protected:
		std::filesystem::path filePath;

	public:

		FilePath(const std::string& filePath);

		virtual ~FilePath();

		operator std::string ();

		FilePath& AutoCreateFile();

		virtual bool Delete();

	public: // Static methods

		static bool CreateDirectory(const std::string& path);

		static bool CreateFile(const std::string& path);

		static bool DirectoryExists(const std::string& path);

		static bool FileExists(const std::string& path/*, bool supportLinks = false*/);

		static bool DeleteDirectory(const std::string& path, bool recursive = false);

	};
}
