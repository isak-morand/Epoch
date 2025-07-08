#pragma once
#include <EpochCore/Log.h>

namespace Epoch
{
	void Main(int argc, char** argv);
}

extern void Epoch::Main(int aArgc, char** aArgv);

int main(int argc, char** argv)
{
	Epoch::Log::Init();

	Epoch::Main(argc, argv);

	Epoch::Log::Shutdown();
}
