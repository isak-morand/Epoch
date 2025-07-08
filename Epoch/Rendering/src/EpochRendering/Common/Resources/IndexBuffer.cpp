#include "epch.h"
#include "IndexBuffer.h"
#include "EpochRendering/Common/RenderContext.h"

namespace Epoch::Rendering
{
	IndexBuffer::IndexBuffer(const IndexBufferSpecification& aSpecification) : mySpecification(aSpecification)
	{
		auto bufferDesc = nvrhi::BufferDesc()
			.setByteSize(mySpecification.SizeInBytes)
			.setIsIndexBuffer(true)
			.setDebugName(mySpecification.DebugName)
			.setCpuAccess(mySpecification.CpuWritable ? nvrhi::CpuAccessMode::Write : nvrhi::CpuAccessMode::None)
			.setInitialState(nvrhi::ResourceStates::IndexBuffer)
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

	IndexBuffer::~IndexBuffer()
	{
		if (mySpecification.InitialData)
		{
			mySpecification.InitialData.Release();
		}
	}

	void IndexBuffer::SetData(Core::Buffer aBuffer, uint64_t aOffset)
	{
		if (!mySpecification.CpuWritable)
		{
			LOG_ERROR("SetData called on non-CPU-writable IndexBuffer '{}'.", mySpecification.DebugName);
			return;
		}

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
