#include "ServiceLocator.h"

namespace Core {

namespace {
inline std::unordered_map<std::type_index, std::shared_ptr<void>> serviceMap{};
inline std::mutex serviceMtx{};
}

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map()
{
	return serviceMap;
}

std::mutex& ServiceLocator::mtx()
{
	return serviceMtx;
}

} // namespace Core


