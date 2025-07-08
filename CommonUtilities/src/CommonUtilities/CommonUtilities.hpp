#pragma once

namespace CU
{
	template<typename T>
	void Swap(T& a0, T& a1)
	{
		const T temp = a0;
		a0 = a1;
		a1 = temp;
	}
}
