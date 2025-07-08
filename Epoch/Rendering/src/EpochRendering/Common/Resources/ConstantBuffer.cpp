#include "epch.h"
#include "ConstantBuffer.h"
#include "EpochRendering/Common/RenderContext.h"

namespace Epoch::Rendering
{
	ConstantBuffer::ConstantBuffer(const ConstantBufferSpecification& aSpecification) : mySpecification(aSpecification)
	{
		auto bufferDesc = nvrhi::BufferDesc()
			.setByteSize(mySpecification.SizeInBytes)
			.setIsConstantBuffer(true)
			//.setIsVolatile(true)
			.setCpuAccess(nvrhi::CpuAccessMode::Write)
			.setDebugName(mySpecification.DebugName)
			.setInitialState(nvrhi::ResourceStates::ConstantBuffer)
			.setKeepInitialState(true); // enable fully automatic state tracking

		myHandle = RenderContext::Get().DeviceManager->GetDeviceHandle()->createBuffer(bufferDesc);

		if (mySpecification.InitialData)
		{
			EPOCH_ASSERT
			(
				mySpecification.InitialData.size <= mySpecification.SizeInBytes,
				"initialData is larger ({}) than buffer size ({})",
				mySpecification.InitialData.size,
				mySpecification.SizeInBytes
			);

			nvrhi::DeviceHandle device = RenderContext::Get().DeviceManager->GetDeviceHandle();
			nvrhi::CommandListHandle commandList = device->createCommandList();
			commandList->open();
			commandList->writeBuffer(myHandle, mySpecification.InitialData.data, mySpecification.InitialData.size);
			commandList->close();
			device->executeCommandList(commandList);
		}
	}

	ConstantBuffer::~ConstantBuffer()
	{
		if (mySpecification.InitialData)
		{
			mySpecification.InitialData.Release();
		}
	}

	void ConstantBuffer::SetData(Core::Buffer aBuffer, uint64_t aOffset)
	{
		EPOCH_ASSERT
		(
			aOffset + aBuffer.size <= mySpecification.SizeInBytes,
			"Write range exceeds buffer size (offset: {}, size: {}, buffer size: {}).",
			aOffset,
			aBuffer.size,
			mySpecification.SizeInBytes
		);

		nvrhi::DeviceHandle device = RenderContext::Get().DeviceManager->GetDeviceHandle();
		nvrhi::CommandListHandle commandList = device->createCommandList();
		commandList->open();
		commandList->writeBuffer(myHandle, aBuffer.data, aBuffer.size, aOffset);
		commandList->close();
		device->executeCommandList(commandList);
	}
}
