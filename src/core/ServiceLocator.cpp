#include "ServiceLocator.h"

namespace Core {

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map()
{
	static auto serviceMap = std::unordered_map<std::type_index, std::shared_ptr<void>>{};
	return serviceMap;
}

std::mutex& ServiceLocator::mtx()
{
	static auto serviceMtx = std::mutex{};
	return serviceMtx;
}

} // namespace Core


