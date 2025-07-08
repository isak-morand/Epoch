#include "epch.h"
#include "NVRHIMessageCallback.h"

namespace Epoch::Rendering
{
	DefaultMessageCallback& DefaultMessageCallback::GetInstance()
	{
		static DefaultMessageCallback instance;
		return instance;
	}

	void DefaultMessageCallback::message(nvrhi::MessageSeverity severity, const char* messageText)
	{
		switch (severity)
		{
			case nvrhi::MessageSeverity::Info:
				LOG_INFO("{}", messageText);
				break;
			case nvrhi::MessageSeverity::Warning:
				LOG_WARNING("{}", messageText);
				break;
			case nvrhi::MessageSeverity::Error:
				LOG_ERROR("{}", messageText);
				break;
			case nvrhi::MessageSeverity::Fatal:
				LOG_FATAL("{}", messageText);
				break;
		}
	}
}