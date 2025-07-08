#pragma once
#include <memory>
#include <string>
#include <CommonUtilities/Math/Vector/Vector4.hpp>
#include <CommonUtilities/Color.h>

namespace Epoch::DataTypes
{
	enum class ShaderPropertyType { Unkown, Float, Range, Vector, Color, Tex2D };

	inline ShaderPropertyType StringToShaderPropertyType(const std::string& aString)
	{
		if (aString == "Float")		return ShaderPropertyType::Float;
		if (aString == "Range")		return ShaderPropertyType::Range;
		if (aString == "Vector")	return ShaderPropertyType::Vector;
		if (aString == "Color")		return ShaderPropertyType::Color;
		if (aString == "Tex2D")		return ShaderPropertyType::Tex2D;
		return ShaderPropertyType::Unkown;
	}

	struct ShaderProperty
	{
		std::string Name;
		std::string DisplayName;
		ShaderPropertyType Type;
		uint32_t OffsetInBuffer = 0u;
		uint32_t SizeInBuffer = 0u;

		virtual bool ToDefaultValue(const std::string& aDefaultValueString) = 0;
	};

	struct FloatProperty : ShaderProperty
	{
		float DefaultValue = 0.0f;

		bool ToDefaultValue(const std::string& aDefaultValueString) override
		{
			std::stringstream s(aDefaultValueString);

			float f;
			if (s >> f)
			{
				DefaultValue = f;
				return true;
			}

			return false;
		}
	};

	struct RangeProperty : ShaderProperty
	{
		float DefaultValue = 0.0f;
		float MinValue = 0.0f;
		float MaxValue = 1.0f;

		bool ToDefaultValue(const std::string& aDefaultValueString) override
		{
			std::stringstream s(aDefaultValueString);

			float f;
			if (s >> f)
			{
				DefaultValue = f;
				return true;
			}

			return false;
		}

		void ToMinValue(const std::string& aMinValueString)
		{
			std::stringstream s(aMinValueString);

			float f;
			if (s >> f)
			{
				MinValue = f;
			}
		}

		void ToMaxValue(const std::string& aMaxValueString)
		{
			std::stringstream s(aMaxValueString);

			float f;
			if (s >> f)
			{
				MaxValue = f;
			}
		}
	};

	struct VectorProperty : ShaderProperty
	{
		CU::Vector4f DefaultColor;

		bool ToDefaultValue(const std::string& aDefaultValueString) override
		{
			std::stringstream s(aDefaultValueString);

			CU::Vector4f vector;
			if (sscanf(aDefaultValueString.c_str(), "(%f, %f, %f, %f)", &vector.x, &vector.y, &vector.z, &vector.w) == 4)
			{
				DefaultColor = vector;
				return true;
			}

			return false;
		}
	};

	struct ColorProperty : ShaderProperty
	{
		CU::Color DefaultColor;

		bool ToDefaultValue(const std::string& aDefaultValueString) override
		{
			std::stringstream s(aDefaultValueString);

			CU::Color color;
			if (sscanf(aDefaultValueString.c_str(), "(%f, %f, %f, %f)", &color.r, &color.g, &color.b, &color.a) == 4)
			{
				DefaultColor = color;
				return true;
			}

			return false;
		}
	};

	struct Tex2DProperty : ShaderProperty
	{
		std::string defaultTexture; // "white", "black", etc.

		bool ToDefaultValue(const std::string& aDefaultValueString) override
		{
			if (!aDefaultValueString.empty())
			{
				defaultTexture = aDefaultValueString;
				return true;
			}
			return false;
		}
	};

	inline std::unique_ptr<ShaderProperty> CreateProperty(ShaderPropertyType aType)
	{
		switch (aType)
		{
			case ShaderPropertyType::Float:		return std::make_unique<DataTypes::FloatProperty>();
			case ShaderPropertyType::Range:		return std::make_unique<DataTypes::RangeProperty>();
			case ShaderPropertyType::Vector:	return std::make_unique<DataTypes::VectorProperty>();
			case ShaderPropertyType::Color:		return std::make_unique<DataTypes::ColorProperty>();
			case ShaderPropertyType::Tex2D:		return std::make_unique<DataTypes::Tex2DProperty>();
		}
		return nullptr;
	}

	inline uint32_t GetPropertyByteSize(ShaderPropertyType aType)
	{
		switch (aType)
		{
			case ShaderPropertyType::Float:
			case ShaderPropertyType::Range: return 4u;
			case ShaderPropertyType::Vector:
			case ShaderPropertyType::Color: return 16u;
		}
		return 0;
	}
}
