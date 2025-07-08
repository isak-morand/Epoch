#pragma once

#pragma warning(push, 0)
#include <yaml-cpp/yaml.h>
#pragma warning(pop)

#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Color.h>
#include <EpochCore/UUID.h>

namespace YAML
{
	template<>
	struct convert<Epoch::UUID>
	{
		static Node encode(const Epoch::UUID& aRhs)
		{
			return Node((uint64_t)aRhs);
		}

		static bool decode(const Node& node, Epoch::UUID& aRhs)
		{
			if (!node.IsScalar())
			{
				return false;
			}

			aRhs = (Epoch::UUID)node.as<uint64_t>();
			return true;
		}
	};

	template<typename T>
	struct convert<CU::Vector2<T>>
	{
		static Node encode(const CU::Vector2<T>& aRhs)
		{
			Node node;
			node.push_back(aRhs.x);
			node.push_back(aRhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CU::Vector2<T>& aRhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			aRhs.x = node[0].as<T>();
			aRhs.y = node[1].as<T>();
			return true;
		}
	};

	template<typename T>
	struct convert<CU::Vector3<T>>
	{
		static Node encode(const CU::Vector3<T>& aRhs)
		{
			Node node;
			node.push_back(aRhs.x);
			node.push_back(aRhs.y);
			node.push_back(aRhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CU::Vector3<T>& aRhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			aRhs.x = node[0].as<T>();
			aRhs.y = node[1].as<T>();
			aRhs.z = node[2].as<T>();
			return true;
		}
	};

	template<typename T>
	struct convert<CU::Vector4<T>>
	{
		static Node encode(const CU::Vector4<T>& aRhs)
		{
			Node node;
			node.push_back(aRhs.x);
			node.push_back(aRhs.y);
			node.push_back(aRhs.z);
			node.push_back(aRhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& aNode, CU::Vector4<T>& aRhs)
		{
			if (!aNode.IsSequence() || aNode.size() != 4)
			{
				return false;
			}

			aRhs.x = aNode[0].as<T>();
			aRhs.y = aNode[1].as<T>();
			aRhs.z = aNode[2].as<T>();
			aRhs.w = aNode[3].as<T>();
			return true;
		}
	};

	template<>
	struct convert<CU::Color>
	{
		static Node encode(const CU::Color& aRhs)
		{
			Node node;
			node.push_back(aRhs.r);
			node.push_back(aRhs.g);
			node.push_back(aRhs.b);
			node.push_back(aRhs.a);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& aNode, CU::Color& aRhs)
		{
			if (!aNode.IsSequence() || aNode.size() != 4)
				return false;

			aRhs.r = aNode[0].as<float>();
			aRhs.g = aNode[1].as<float>();
			aRhs.b = aNode[2].as<float>();
			aRhs.a = aNode[3].as<float>();
			return true;
		}
	};
}

namespace Epoch
{
	template<typename T>
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const UUID& aUUID)
	{
		out << YAML::Value << (uint64_t)aUUID;
		return out;
	}

	template<typename T>
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Vector2<T>& aVector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aVector.x << aVector.y << YAML::EndSeq;
		return out;
	}

	template<typename T>
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Vector3<T>& aVector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aVector.x << aVector.y << aVector.z << YAML::EndSeq;
		return out;
	}

	template<typename T>
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Vector4<T>& aVector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aVector.x << aVector.y << aVector.z << aVector.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const CU::Color& aColor)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << aColor.r << aColor.g << aColor.b << aColor.a << YAML::EndSeq;
		return out;
	}
}