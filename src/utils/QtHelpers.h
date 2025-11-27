#pragma once

#include <memory>
#include <utility>

namespace Utils::QtHelpers {

template <typename T, typename... Args>
T* makeOwned(Args&&... args)
{
	auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
	return ptr.release();
}

} // namespace Utils::QtHelpers


