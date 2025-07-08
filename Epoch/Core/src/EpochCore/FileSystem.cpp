#include "epch.h"
#include "FileSystem.h"
#include <nfd.hpp>

namespace Epoch::Core
{
	bool FileSystem::Initialize()
	{
		bool succeeded = NFD::Init() == NFD_OKAY;
		EPOCH_ASSERT(succeeded, "Failed to init NFD_Extended!");
		return succeeded;
	}

	void FileSystem::Shutdown()
	{
		NFD::Quit();
	}

	bool FileSystem::CreateDirectory(const std::filesystem::path& aDirectory)
	{
		return std::filesystem::create_directories(aDirectory);
	}

	bool FileSystem::IsDirectory(const std::filesystem::path& aFilepath)
	{
		return std::filesystem::is_directory(aFilepath);
	}

	bool FileSystem::Exists(const std::filesystem::path& aFilepath)
	{
		return std::filesystem::exists(aFilepath);
	}

	bool FileSystem::Delete(const std::filesystem::path& aFilepath)
	{
		if (!FileSystem::Exists(aFilepath)) return false;

		if (std::filesystem::is_directory(aFilepath))
		{
			return std::filesystem::remove_all(aFilepath) > 0;
		}
		return std::filesystem::remove(aFilepath);
	}

	bool FileSystem::Move(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest)
	{
		return MoveInternal(aFilepath, aDest / aFilepath.filename());
	}

	bool FileSystem::Copy(const std::filesystem::path& aFilepath, const std::filesystem::path& aDest)
	{
		return CopyInternal(aFilepath, aDest / aFilepath.filename());
	}

	bool FileSystem::Rename(const std::filesystem::path& aOldFilepath, const std::string& aNewName)
	{
		std::filesystem::path newPath = aOldFilepath.parent_path() / std::filesystem::path(aNewName + aOldFilepath.extension().string());
		return RenameInternal(aOldFilepath, newPath);
	}

	bool FileSystem::IsNewer(const std::filesystem::path& aFileA, const std::filesystem::path& aFileB)
	{
		return std::filesystem::last_write_time(aFileA) > std::filesystem::last_write_time(aFileB);
	}

	bool FileSystem::ShowFileInExplorer(const std::filesystem::path& aPath)
	{
		return false;
	}

	bool FileSystem::OpenDirectoryInExplorer(const std::filesystem::path& aPath)
	{
		return false;
	}

	bool FileSystem::OpenExternally(const std::filesystem::path& aPath)
	{
		return false;
	}

	std::filesystem::path FileSystem::GetUniqueFileName(const std::filesystem::path& aFilepath)
	{
		if (!FileSystem::Exists(aFilepath))
		{
			return aFilepath;
		}

		int counter = 0;
		auto checkID = [&counter, aFilepath](auto checkID) -> std::filesystem::path
			{
				++counter;
				std::string newFileName = std::format("{} ({})", aFilepath.stem().string(), std::to_string(counter));

				if (aFilepath.has_extension())
				{
					newFileName = std::format("{}{}", newFileName, aFilepath.extension().string());
				}

				if (FileSystem::Exists(aFilepath.parent_path() / newFileName))
				{
					return checkID(checkID);
				}
				else
				{
					return aFilepath.parent_path() / newFileName;
				}
			};

		return checkID(checkID);
	}

	Buffer FileSystem::ReadFile(const std::filesystem::path& aFilePath)
	{
		Buffer buffer;

		std::ifstream stream(aFilePath, std::ios::binary | std::ios::ate);
		EPOCH_ASSERT(stream, "Failed to open stream!");

		auto end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		auto size = end - stream.tellg();
		if (size == 0)
		{
			LOG_WARNING("File empty while trying to read bytes");
			return Buffer();
		}

		buffer.Allocate((uint32_t)size);
		stream.read((char*)buffer.data, buffer.size);
		stream.close();

		return buffer;
	}

	std::filesystem::path FileSystem::OpenFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters, const char* aInitialFolder)
	{
		NFD::UniquePath filePath;
		nfdresult_t result = NFD::OpenDialog(filePath, (const nfdfilteritem_t*)aFilters.begin(), (nfdfiltersize_t)aFilters.size(), (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY: return filePath.get();
		case NFD_CANCEL: return "";
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return "";
		}
		}

		return std::filesystem::path();
	}

	std::vector<std::filesystem::path> FileSystem::OpenFileDialogMultiple(const std::initializer_list<FileDialogFilterItem> aFilters, const char* aInitialFolder)
	{
		NFD::UniquePathSet filePaths;
		nfdresult_t result = NFD::OpenDialogMultiple(filePaths, (const nfdfilteritem_t*)aFilters.begin(), (nfdfiltersize_t)aFilters.size(), (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY:
		{
			nfdpathsetsize_t numPaths;
			NFD::PathSet::Count(filePaths, numPaths);

			std::vector<std::filesystem::path> outPaths(numPaths);
			for (nfdpathsetsize_t i = 0; i < numPaths; ++i)
			{
				NFD::UniquePathSetPath path;
				NFD::PathSet::GetPath(filePaths, i, path);
				outPaths.push_back(path.get());
			}

			return outPaths;
		}
		case NFD_CANCEL: return std::vector<std::filesystem::path>();
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return std::vector<std::filesystem::path>();
		}
		}

		return std::vector<std::filesystem::path>();
	}

	std::filesystem::path FileSystem::OpenFolderDialog(const char* aInitialFolder)
	{
		NFD::UniquePath filePath;
		nfdresult_t result = NFD::PickFolder(filePath, (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY: return filePath.get();
		case NFD_CANCEL: return "";
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return "";
		}
		}

		return std::filesystem::path();
	}

	std::filesystem::path FileSystem::SaveFileDialog(const std::initializer_list<FileDialogFilterItem> aFilters, const char* aInitialFolder)
	{
		NFD::UniquePath filePath;
		nfdresult_t result = NFD::SaveDialog(filePath, (const nfdfilteritem_t*)aFilters.begin(), (nfdfiltersize_t)aFilters.size(), (const nfdchar_t*)aInitialFolder);

		switch (result)
		{
		case NFD_OKAY: return filePath.get();
		case NFD_CANCEL: return "";
		case NFD_ERROR:
		{
			LOG_ERROR("NFD-Extended threw an error: {}", NFD::GetError());
			return "";
		}
		}

		return std::filesystem::path();
	}

	bool FileSystem::MoveInternal(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath)
	{
		return RenameInternal(aOldFilepath, aNewFilepath);
	}

	bool FileSystem::CopyInternal(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath)
	{
		if (FileSystem::Exists(aNewFilepath)) return false;

		std::filesystem::copy(aOldFilepath, aNewFilepath);
		return true;
	}

	bool FileSystem::RenameInternal(const std::filesystem::path& aOldFilepath, const std::filesystem::path& aNewFilepath)
	{
		if (FileSystem::Exists(aNewFilepath)) return false;

		std::filesystem::rename(aOldFilepath, aNewFilepath);
		return true;
	}
}
