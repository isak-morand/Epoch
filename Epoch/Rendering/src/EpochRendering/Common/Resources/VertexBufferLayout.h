#pragma once
#include <string>
#include <EpochCore/Assert.h>

namespace Epoch::Rendering
{
	enum class ShaderDataType { Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UInt, UInt2, UInt3, UInt4 };
	enum class ShaderDataInputRate { PerVertex, PerInstance };

	inline uint32_t ShaderDataTypeSize(ShaderDataType aType)
	{
		switch (aType)
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::UInt:		return 4;
			case ShaderDataType::UInt2:		return 4 * 2;
			case ShaderDataType::UInt3:		return 4 * 3;
			case ShaderDataType::UInt4:		return 4 * 4;
		}

		EPOCH_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct VertexBufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;

		VertexBufferElement() = delete;
		VertexBufferElement(ShaderDataType aType, const std::string& aName, uint32_t aIndex = 0) :
			Name(aName), Type(aType), Size(ShaderDataTypeSize(aType)), Offset(0) {}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;
		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& aElements) : myElements(aElements) { CalculateOffsetsAndStride(); }
		VertexBufferLayout(ShaderDataInputRate aInputRate, const std::initializer_list<VertexBufferElement>& aElements) : myInputRate(aInputRate), myElements(aElements) { CalculateOffsetsAndStride(); }

		uint32_t GetStride() const { return myStride; }
		ShaderDataInputRate GetInputRate() const { return myInputRate; }
		bool Emtpy() const { return myElements.empty(); }
		uint32_t GetElementCount() const { return (uint32_t)myElements.size(); }

		std::vector<VertexBufferElement>::iterator begin() { return myElements.begin(); }
		std::vector<VertexBufferElement>::iterator end() { return myElements.end(); }
		std::vector<VertexBufferElement>::const_iterator begin() const { return myElements.begin(); }
		std::vector<VertexBufferElement>::const_iterator end() const { return myElements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			myStride = 0;
			for (auto& element : myElements)
			{
				element.Offset = offset;
				offset += element.Size;
				myStride += element.Size;
			}
		}
	private:
		std::vector<VertexBufferElement> myElements;
		uint32_t myStride = 0;
		ShaderDataInputRate myInputRate = ShaderDataInputRate::PerVertex;
	};
}
