#include "Gradient.h"
#include <algorithm>
#include "Math/CommonMath.hpp"

namespace CU
{
	Gradient::Gradient(bool aAddDefaultKeys)
	{
		if (aAddDefaultKeys)
		{
			AddColorKey(0.0f);
			AddColorKey(1.0f);

			AddAlphaKey(0.0f);
			AddAlphaKey(1.0f);
		}
	}

	Gradient::~Gradient()
	{
		colorKeys.clear();
		alphaKeys.clear();
	}

	CU::Gradient Gradient::CreateCopy() const
	{
		CU::Gradient gradient(false);
		CopyTo(gradient);
		return gradient;
	}

	void Gradient::CopyTo(CU::Gradient& outGradient) const
	{
		if (outGradient.colorKeys.size() > 0)
		{
			outGradient.colorKeys.clear();
		}

		if (outGradient.alphaKeys.size() > 0)
		{
			outGradient.alphaKeys.clear();
		}

		for (const std::shared_ptr<Key>& key : colorKeys)
		{
			outGradient.AddColorKey(key->time, key->color);
		}

		for (const std::shared_ptr<Key>& key : alphaKeys)
		{
			outGradient.AddAlphaKey(key->time, key->alpha);
		}
	}

	void Gradient::AddColorKey(float aTime, const Color& aColor)
	{
		colorKeys.push_back(std::make_shared<Key>(KeyType::Color, aTime, aColor));
		RefreshCache();
	}

	void Gradient::AddColorKey(std::shared_ptr<Key>& aColorKey)
	{
		colorKeys.push_back(aColorKey);
		RefreshCache();
	}

	void Gradient::RemoveColorKey(std::shared_ptr<Key> aColorKey)
	{
		colorKeys.remove(aColorKey);
		RefreshCache();
	}

	void Gradient::AddAlphaKey(float aTime, float aAlpha)
	{
		alphaKeys.push_back(std::make_shared<Key>(KeyType::Alpha, aTime, aAlpha));
		RefreshCache();
	}

	void Gradient::AddAlphaKey(std::shared_ptr<Key>& aAlphaKey)
	{
		alphaKeys.push_back(aAlphaKey);
		RefreshCache();
	}

	void Gradient::RemoveAlphaKey(std::shared_ptr<Key> aAlphaKey)
	{
		alphaKeys.remove(aAlphaKey);
		RefreshCache();
	}

	Color Gradient::GetColorAt(float aPosition)
	{
		aPosition = Math::Clamp01(aPosition);
		int cachePos = (int)(aPosition * 255.0f);
		return myCachedValues[cachePos];
	}

	void Gradient::RefreshCache()
	{
		colorKeys.sort([](const std::shared_ptr<Key>& a, const std::shared_ptr<Key>& b) { return a->time < b->time; });
		alphaKeys.sort([](const std::shared_ptr<Key>& a, const std::shared_ptr<Key>& b) { return a->time < b->time; });

		const float divider = 1.0f / 255.0f;
		for (int i = 0; i < 256; ++i)
		{
			myCachedValues[i] = ComputeColorAt((float)i * divider);
		}
	}

	Color Gradient::ComputeColorAt(float aTime) const
	{
		aTime = Math::Clamp01(aTime);

		Color returnColor;

		if (colorKeys.size() == 0)
		{
			returnColor = Color::White;
		}
		else
		{
			ComputeColor(returnColor, aTime);
		}

		if (alphaKeys.size() == 0)
		{
			returnColor.a = 1.0f;
		}
		else
		{
			ComputeAlpha(returnColor.a, aTime);
		}

		return returnColor;
	}

	void Gradient::ComputeColor(Color& outColor, const float aTime) const
	{
		if (static_cast<int>(colorKeys.size()) == 1)
		{
			outColor = colorKeys.front()->color;
			return;
		}

		std::shared_ptr<Key> left = nullptr;
		std::shared_ptr<Key> right = nullptr;
		FindLeftAndRightKey(left, right, aTime, colorKeys);

		if (!left && !right)
		{
			outColor = Color::White;
			return;
		}

		if (left.get() == right.get())
		{
			outColor = left->color;
			return;
		}

		float lerpValue = Math::Remap01(aTime, left->time, right->time);

		outColor = Color::Lerp(left->color, right->color, lerpValue);

		//{
		//	CU::Color leftCol = left->color;
		//	CU::Color rightCol = right->color;
		//
		//	leftCol.r = std::sqrtf(leftCol.r);
		//	leftCol.g = std::sqrtf(leftCol.g);
		//	leftCol.b = std::sqrtf(leftCol.b);
		//
		//	rightCol.r = std::sqrtf(rightCol.r);
		//	rightCol.g = std::sqrtf(rightCol.g);
		//	rightCol.b = std::sqrtf(rightCol.b);
		//
		//	CU::Color mix = Color::Lerp(leftCol, rightCol, lerpValue);
		//	mix *= mix;
		//	outColor = mix;
		//}

		//{
		//	CU::Color leftCol = left->color;
		//	CU::Color rightCol = right->color;
		//
		//	leftCol *= leftCol;
		//	rightCol *= rightCol;
		//
		//	CU::Color mix = Color::Lerp(leftCol, rightCol, lerpValue);
		//
		//	mix.r = std::sqrtf(mix.r);
		//	mix.g = std::sqrtf(mix.g);
		//	mix.b = std::sqrtf(mix.b);
		//	
		//	mix.r = std::sqrtf(mix.r);
		//	mix.g = std::sqrtf(mix.g);
		//	mix.b = std::sqrtf(mix.b);
		//
		//	mix *= mix;
		//
		//	outColor = mix;
		//}
	}
	
	void Gradient::ComputeAlpha(float& outAlpha, const float aTime) const
	{
		if (static_cast<int>(alphaKeys.size()) == 1)
		{
			outAlpha = alphaKeys.front()->alpha;
			return;
		}

		std::shared_ptr<Key> left = nullptr;
		std::shared_ptr<Key> right = nullptr;
		FindLeftAndRightKey(left, right, aTime, alphaKeys);

		if (!left && !right)
		{
			outAlpha = 1.0f;
			return;
		}

		if (left.get() == right.get())
		{
			outAlpha = left->alpha;
			return;
		}

		float lerpValue = Math::Remap01(aTime, left->time, right->time);
		outAlpha = Math::Lerp(left->alpha, right->alpha, lerpValue);
	}

	void Gradient::FindLeftAndRightKey(std::shared_ptr<Key>& outLeft, std::shared_ptr<Key>& outRight, float aTime, const std::list<std::shared_ptr<Key>>& aList) const
	{
		for (const std::shared_ptr<Key>& key : aList)
		{
			if (key->time < aTime)
			{
				if (!outLeft || outLeft->time < key->time)
				{
					outLeft = key;
				}
			}

			if (key->time >= aTime)
			{
				if (!outRight || outRight->time > key->time)
				{
					outRight = key;
				}
			}
		}

		if (outRight && !outLeft)
		{
			outLeft = outRight;
		}
		else if (!outRight && outLeft)
		{
			outRight = outLeft;
		}
	}
}
