#pragma once
#include <filesystem>
#include "Buffer.h"

#ifdef CreateDirectory
#undef CreateDirectory
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef SetEnvironmentVariable
#undef GetEnvironmentVariable
#endif

namespace Epoch::Core
{
	class FileSystem
	{
	public:
		static bool Initialize();
		static void Shutdown();

		static bool CreateDirectory(const std::filesystem::path& aDirectory);
		static bool IsDirectory(const std::filesystem::path& aFilepath);
		static bool Exists(const std::filesystem::path& aFilepath);
		static bool Delete(const std::filesystem::path& aFilepath);
		static bool Move(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest);
		static bool Copy(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest);
		static bool Rename(const std::filesystem::path& aOldFilepath, const std::string& aNewName);

		static bool IsNewer(const std::filesystem::path& aFileA, const std::filesystem::path& aFileB);

		static bool ShowFileInExplorer(const std::filesystem::path& aPath);
		static bool OpenDirectoryInExplorer(const std::filesystem::path& aPath);
		static bool OpenExternally(const std::filesystem::path& aPath);

		static std::filesystem::path GetUniqueFileName(const std::filesystem::path& aFilepath);

		static Buffer ReadFile(const std::filesystem::path& aFilePath);

		struct FileDialogFilterItem
		{
			const char* name;
			const char* spec;
		};

		static std::filesystem::path OpenFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters = {}, const char* aInitialFolder = "");
		static std::vector<std::filesystem::path> OpenFileDialogMultiple(const std::initializer_list<FileDialogFilterItem> aFilters = {}, const char* aInitialFolder = "");
		static std::filesystem::path OpenFolderDialog(const char* aInitialFolder = "");
		static std::filesystem::path SaveFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters = {}, const char* aInitialFolder = "");

	private:
		static bool MoveInternal(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
		static bool CopyInternal(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
		static bool RenameInternal(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath);
	};
}
