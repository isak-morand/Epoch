#pragma once
#include <cstdint>
#include <memory>
#include "Assert.h"

namespace Epoch::Core
{
	using byte = uint8_t;

	class Buffer
	{
	public:

		Buffer() : data(nullptr), size(0) {}
		Buffer(const void* data, uint64_t size = 0) : data((void*)data), size(size) {}

		void* data;
		uint64_t size;

		static Buffer Copy(const Buffer& aOther)
		{
			Buffer buffer;
			buffer.Allocate(aOther.size);
			memcpy(buffer.data, aOther.data, aOther.size);
			return buffer;
		}

		static Buffer Copy(const void* aData, uint64_t aSize)
		{
			Buffer buffer;
			buffer.Allocate(aSize);
			memcpy(buffer.data, aData, aSize);
			return buffer;
		}

		void Allocate(uint64_t aSize)
		{
			delete[](byte*)data;
			data = nullptr;

			if (aSize == 0)
			{
				return;
			}

			data = new byte[aSize];
			size = aSize;
		}

		void Release()
		{
			delete[](byte*)data;
			data = nullptr;
			size = 0;
		}

		void ZeroInitialize()
		{
			if (data)
			{
				memset(data, 0, size);
			}
		}

		template<typename T>
		T& Read(uint64_t aOffset = 0)
		{
			EPOCH_ASSERT(aOffset + sizeof(T) <= size, "Buffer overflow!");
			return *(T*)((byte*)data + aOffset);
		}

		template<typename T>
		const T& Read(uint64_t aOffset = 0) const
		{
			EPOCH_ASSERT(aOffset + sizeof(T) <= size, "Buffer overflow!");
			return *(T*)((byte*)data + aOffset);
		}

		byte* ReadBytes(uint64_t aSize, uint64_t aOffset) const
		{
			EPOCH_ASSERT(aOffset + aSize <= size, "Buffer overflow!");
			byte* buffer = new byte[aSize];
			memcpy(buffer, (byte*)data + aOffset, aSize);
			return buffer;
		}

		void Write(const void* aData, uint64_t aSize, uint64_t aOffset = 0)
		{
			EPOCH_ASSERT(aOffset + aSize <= size, "Buffer overflow!");
			memcpy((byte*)data + aOffset, aData, aSize);
		}

		operator bool() const
		{
			return data;
		}

		byte& operator[](int index)
		{
			return ((byte*)data)[index];
		}

		byte operator[](int index) const
		{
			return ((byte*)data)[index];
		}

		template<typename T>
		T* As() const
		{
			return (T*)data;
		}

		inline uint64_t GetSize() const { return size; }
	};
}
