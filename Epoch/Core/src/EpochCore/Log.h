#pragma once
#pragma warning(push)
#pragma warning(disable: 4996) // MSVC-specific warning code for deprecated functions
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace Epoch
{
	class Log
	{
	public:
		enum class Level : uint8_t { Debug, Info, Warn, Error, Fatal };

	public:
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger>& GetLogger() { return staticLogger; }

		template<typename... Args>
		static void PrintMessage(Log::Level aLevel, std::format_string<Args...> aFormat, Args&&... aArgs);
		template<typename... Args>
		static void PrintMessageTag(Log::Level aLevel, std::string_view aTag, std::format_string<Args...> aFormat, Args&&... aArgs);

		template<typename... Args>
		static void PrintAssertMessage(std::string_view aPrefix, std::format_string<Args...> aMessage, Args&&... aArgs);

	private:
		inline static std::shared_ptr<spdlog::logger> staticLogger;
	};
}

#define LOG_DEBUG(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Debug, __VA_ARGS__)
#define LOG_INFO(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Info, __VA_ARGS__)
#define LOG_WARNING(...)			::Epoch::Log::PrintMessage(::Epoch::Log::Level::Warn, __VA_ARGS__)
#define LOG_ERROR(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Error, __VA_ARGS__)
#define LOG_FATAL(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Fatal, __VA_ARGS__)

#define LOG_DEBUG_TAG(tag, ...)		::Epoch::Log::PrintMessageTag(::Epoch::Log::Level::Debug, tag, __VA_ARGS__)
#define LOG_INFO_TAG(tag, ...)		::Epoch::Log::PrintMessageTag(::Epoch::Log::Level::Info, tag, __VA_ARGS__)
#define LOG_WARNING_TAG(tag, ...)	::Epoch::Log::PrintMessageTag(::Epoch::Log::Level::Warn, tag, __VA_ARGS__)
#define LOG_ERROR_TAG(tag, ...)		::Epoch::Log::PrintMessageTag(::Epoch::Log::Level::Error, tag, __VA_ARGS__)
#define LOG_FATAL_TAG(tag, ...)		::Epoch::Log::PrintMessageTag(::Epoch::Log::Level::Fatal, tag, __VA_ARGS__)

namespace Epoch
{
	template<typename ...Args>
	inline void Log::PrintMessage(Log::Level aLevel, std::format_string<Args...> aFormat, Args && ...aArgs)
	{
		switch (aLevel)
		{
			case Level::Debug:
				GetLogger()->debug(aFormat, std::forward<Args>(aArgs)...);
				break;
			case Level::Info:
				GetLogger()->info(aFormat, std::forward<Args>(aArgs)...);
				break;
			case Level::Warn:
				GetLogger()->warn(aFormat, std::forward<Args>(aArgs)...);
				break;
			case Level::Error:
				GetLogger()->error(aFormat, std::forward<Args>(aArgs)...);
				break;
			case Level::Fatal:
				GetLogger()->critical(aFormat, std::forward<Args>(aArgs)...);
				break;
			default:
				break;
		}
	}

	template<typename... Args>
	void Log::PrintMessageTag(Level aLevel, std::string_view aTag, std::format_string<Args...> aFormat, Args&&... aArgs)
	{
		std::string formatted = std::format(aFormat, std::forward<Args>(aArgs)...);

		switch (aLevel)
		{
			case Level::Debug:
				GetLogger()->trace("[{}] {}", aTag, formatted);
				break;
			case Level::Info:
				GetLogger()->info("[{}] {}", aTag, formatted);
				break;
			case Level::Warn:
				GetLogger()->warn("[{}] {}", aTag, formatted);
				break;
			case Level::Error:
				GetLogger()->error("[{}] {}", aTag, formatted);
				break;
			case Level::Fatal:
				GetLogger()->critical("[{}] {}", aTag, formatted);
				break;
			default:
				break;
		}
	}

	template<typename ...Args>
	inline void Log::PrintAssertMessage(std::string_view aPrefix, std::format_string<Args...> aMessage, Args && ...aArgs)
	{
		auto formatted = std::format(aMessage, std::forward<Args>(aArgs)...);
		GetLogger()->error("{}: {}", aPrefix, formatted);

		MessageBoxA(nullptr, formatted.data(), "Epoch Assert", MB_OK | MB_ICONERROR);
	}
}
