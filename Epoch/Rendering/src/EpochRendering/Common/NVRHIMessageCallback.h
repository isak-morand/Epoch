#pragma once
#include <nvrhi/nvrhi.h>

namespace Epoch::Rendering
{
	struct DefaultMessageCallback : public nvrhi::IMessageCallback
	{
		static DefaultMessageCallback& GetInstance();

		void message(nvrhi::MessageSeverity severity, const char* messageText) override;
	};
}
