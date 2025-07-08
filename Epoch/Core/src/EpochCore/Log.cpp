#include "epch.h"
#include "Log.h"
#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Epoch
{
	void Log::Init()
	{
		std::string logsDirectory = "logs";
		if (!std::filesystem::exists(logsDirectory))
		{
			std::filesystem::create_directories(logsDirectory);
		}

		std::vector<spdlog::sink_ptr> epochSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Epoch.log", true),
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		};

		epochSinks[0]->set_pattern("[%T] %l: %v%");
		epochSinks[1]->set_pattern("%^[%T] %v%$");

		staticLogger = std::make_shared<spdlog::logger>("EPOCH", epochSinks.begin(), epochSinks.end());
		staticLogger->enable_backtrace(32);

#ifdef _DEBUG
		staticLogger->set_level(spdlog::level::trace);
#elif _RELEASE
		staticLogger->set_level(spdlog::level::trace);
		//staticLogger->set_level(spdlog::level::warn);
#elif _DIST
		staticLogger->set_level(spdlog::level::err);
#endif
	}

	void Log::Shutdown()
	{
		spdlog::dump_backtrace();
		staticLogger.reset();
		spdlog::drop_all();
	}
}
