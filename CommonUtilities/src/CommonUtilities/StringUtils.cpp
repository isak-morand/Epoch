#include "StringUtils.h"
#include <fstream>

namespace CU
{
	std::wstring ToWString(const std::string& aString)
	{
		return std::wstring(aString.begin(), aString.end());
	}

	std::string ToString(const std::wstring& aWString)
	{
		std::string str(aWString.length(), 0);
		std::transform(aWString.begin(), aWString.end(), str.begin(), [](wchar_t c) { return static_cast<char>(c); });
		return str;
	}

	std::string ToString(const char* aCString)
	{
		return std::string(aCString);
	}

	std::string ToString(const wchar_t* aWCString)
	{
		const std::wstring ws(aWCString);
		return ToString(ws);
	}

	std::string ToLower(const std::string& aString)
	{
		std::string string = aString;
		std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return string;
	}

	std::string ToUpper(const std::string& aString)
	{
		std::string string = aString;
		std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
		return string;
	}


	void Erase(std::string& aStr, const char* aChars)
	{
		for (size_t i = 0; i < strlen(aChars); i++)
		{
			aStr.erase(std::remove(aStr.begin(), aStr.end(), aChars[i]), aStr.end());
		}
	}

	void Erase(std::string& aStr, const std::string& aChars)
	{
		Erase(aStr, aChars.c_str());
	}


	std::vector<std::string> SplitString(const std::string_view aString, const std::string_view& aDelimiters)
	{
		size_t first = 0;
		
		std::vector<std::string> result;
		
		while (first <= aString.size())
		{
			const auto second = aString.find_first_of(aDelimiters, first);
		
			if (first != second)
			{
				result.emplace_back(aString.substr(first, second - first));
			}
		
			if (second == std::string_view::npos)
			{
				break;
			}
		
			first = second + 1;
		}
		
		return result;
	}

	std::vector<std::string> SplitString(const std::string_view aString, const char aDelimiter)
	{
		return SplitString(aString, std::string(1, aDelimiter));
	}

	std::string SplitAtUpperCase(std::string_view aString, std::string_view aDelimiter, bool aIfLowerCaseOnTheRight)
	{
		std::string str(aString);
		const auto rightIsLower = [&](int i) { return i < (int)aString.size() && std::islower(str[i + 1]); };
		
		for (int i = (int)aString.size() - 1; i > 0; --i)
		{
			if (std::isupper(str[i]) && (!aIfLowerCaseOnTheRight || rightIsLower(i)))
			{
				str.insert(i, aDelimiter);
			}
		}

		return str;
	}

	std::string SubStr(const std::string& aString, size_t aOffset, size_t aCount)
	{
		if (aOffset == std::string::npos)
		{
			return aString;
		}

		if (aOffset >= aString.length())
		{
			return aString;
		}

		return aString.substr(aOffset, aCount);
	}

	std::string RemoveExtension(const std::string& aFilename)
	{
		return aFilename.substr(0, aFilename.find_last_of('.'));
	}

	std::string RemoveWhitespaces(const std::string& aString)
	{
		std::string string = aString;
		string.erase(std::remove(string.begin(), string.end(), ' ' ), string.end());
		// string.erase(std::remove_if(string.begin(), string.end(), std::isspace), string.end());
		return string;
	}


	int SkipBOM(std::istream& aIn)
	{
		char test[4] = { 0 };
		aIn.seekg(0, std::ios::beg);
		aIn.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			aIn.seekg(3, std::ios::beg);
			return 3;
		}
		aIn.seekg(0, std::ios::beg);
		return 0;
	}

	std::string ReadFileAndSkipBOM(const std::filesystem::path& aFilepath)
	{
		std::string result;
		std::ifstream in(aFilepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			auto fileSize = in.tellg();
			const int skippedChars = SkipBOM(in);

			fileSize -= skippedChars - 1;
			result.resize(fileSize);
			in.read(result.data() + 1, fileSize);
			// Add a dummy tab to beginning of file.
			result[0] = '\t';
		}
		in.close();
		return result;
	}

	void ReplaceToken(std::string& outStr, const char* aToken, const std::string& aValue)
	{
		size_t pos = 0;
		while ((pos = outStr.find(aToken, pos)) != std::string::npos)
		{
			outStr.replace(pos, strlen(aToken), aValue);
			pos += strlen(aToken);
		}
	}
}
