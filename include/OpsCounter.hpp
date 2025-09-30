#pragma once
#include <cstddef>

struct OpsCounter 
{
	std::size_t add = 0, sub = 0, mul = 0, div_ = 0, cmp = 0, swp = 0;

	void Reset() 
	{ 
		add = sub = mul = div_ = cmp = swp = 0; 
	}

	void IncAdd(std::size_t k = 1) 
	{ 
		add += k; 
	}

	void IncSub(std::size_t k = 1) 
	{ 
		sub += k; 
	}

	void IncMul(std::size_t k = 1) 
	{ 
		mul += k; 
	}

	void IncDiv(std::size_t k = 1) 
	{ 
		div_ += k; 
	}

	void IncCmp(std::size_t k = 1) 
	{ 
		cmp += k; 
	}

	void IncSwp(std::size_t k = 1) 
	{ 
		swp += k; 
	}
};