#pragma once
#include <memory>
#include <list>
#include <array>
#include "Color.h"

namespace CU
{
	class Gradient
	{
	public:
		enum class KeyType { Color, Alpha };
		struct Key
		{
			KeyType type = KeyType::Color;
			float time = 0.0f;
			float alpha = 0.0f;
			Color color;

			Key(KeyType aType, float aTime, const Color& aColor) : type(aType), time(aTime), color(aColor) {}
			Key(KeyType aType, float aTime, float aAlpha) : type(aType), time(aTime), alpha(aAlpha) {}
		};

	public:
		std::list<std::shared_ptr<Key>> colorKeys;
		std::list<std::shared_ptr<Key>> alphaKeys;
		std::array<Color, 256> myCachedValues;
		
	public:
		Gradient(bool aAddDefaultKeys = true);
		~Gradient();

		CU::Gradient CreateCopy() const;
		void CopyTo(CU::Gradient& outGradient) const;

		void AddColorKey(float aTime, const Color& aColor = Color::White);
		void AddColorKey(std::shared_ptr<Key>& aColorKey);
		void RemoveColorKey(std::shared_ptr<Key> aColorKey);

		void AddAlphaKey(float aTime, float aAlpha = 1.0f);
		void AddAlphaKey(std::shared_ptr<Key>& aAlphaKey);
		void RemoveAlphaKey(std::shared_ptr<Key> aAlphaKey);

		Color GetColorAt(float aPosition);
		void RefreshCache();

	private:
		Color ComputeColorAt(float aTime) const;
		void ComputeColor(Color& outColor, const float aTime) const;
		void ComputeAlpha(float& outAlpha, const float aTime) const;
		void FindLeftAndRightKey(std::shared_ptr<Key>& outLeft, std::shared_ptr<Key>& outRight, float aTime, const std::list<std::shared_ptr<Key>>& aList) const;

	private:
	};
}
