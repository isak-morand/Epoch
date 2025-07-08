#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include <algorithm>
#include <vector>
#include "Math/CommonMath.hpp"

namespace CU
{
	std::wstring ToWString(const std::string& aString);
	std::string ToString(const std::wstring& aWString);
	std::string ToString(const char* aCString);
	std::string ToString(const wchar_t* aWCString);

	std::string ToLower(const std::string& aString);
	std::string ToUpper(const std::string& aString);

	void Erase(std::string& aStr, const char* aChars);
	void Erase(std::string& aStr, const std::string& aChars);

	template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
	std::string NumberFormat(T aNumber, std::string_view aSeparator = ".")
	{
		bool negative = CU::Math::Sign(aNumber) < 0.0f;

		if (negative)
		{
			aNumber *= -1;
		}

		std::stringstream fmt;
		fmt << aNumber;
		std::string s = fmt.str();
		s.reserve(s.length() + s.length() / 3);

		// loop until the end of the string and use j to keep track of every
		// third loop starting taking into account the leading x digits (this probably
		// can be rewritten in terms of just i, but it seems more clear when you use
		// a seperate variable)
		for (int i = 0, j = 3 - s.length() % 3; i < s.length(); ++i, ++j)
		{
			if (i != 0 && j % 3 == 0)
			{
				s.insert(i++, 1, *aSeparator.data());
			}
		}
		
		if (negative)
		{
			s.insert(0, 1, '-');
		}

		return s;
	}

	std::vector<std::string> SplitString(const std::string_view aString, const std::string_view& aDelimiters);
	std::vector<std::string> SplitString(const std::string_view aString, const char aDelimiter);

	std::string SplitAtUpperCase(std::string_view aString, std::string_view aDelimiter = " ", bool aIfLowerCaseOnTheRight = true);

	std::string SubStr(const std::string& aString, size_t aOffset, size_t aCount = std::string::npos);

	std::string RemoveExtension(const std::string& aFilename);
	std::string RemoveWhitespaces(const std::string& aString);

	int SkipBOM(std::istream& aIn);
	std::string ReadFileAndSkipBOM(const std::filesystem::path& aFilepath);

	void ReplaceToken(std::string& outStr, const char* aToken, const std::string& aValue);

	///=====================///
	/// Constexpr Utilities ///
	///=====================///

	constexpr bool StartsWith(std::string_view aString, std::string_view s)
	{
		const auto len = s.length();
		return aString.length() >= len && aString.substr(0, len) == s;
	}

	constexpr bool EndsWith(std::string_view aString, std::string_view s)
	{
		const auto len1 = aString.length(), len2 = s.length();
		return len1 >= len2 && aString.substr(len1 - len2) == s;
	}
}