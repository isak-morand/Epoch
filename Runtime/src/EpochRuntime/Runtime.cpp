#include <EpochCore/EntryPoint.h>
#include <EpochEngine/Engine.h>
#include <EpochCore/Log.h>

namespace Epoch
{
	void Main(int argc, char** argv)
	{
		Epoch::EngineSpecification engineSpec;
		engineSpec.WindowProperties.Title = "Epoch Runtime";
		engineSpec.ImGuiEnabled = true;

		Epoch::Engine engine(engineSpec);

		engine.Run();
	}
}