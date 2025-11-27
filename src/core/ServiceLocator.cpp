#include "ServiceLocator.h"

namespace Core {

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map()
{
	static std::unordered_map<std::type_index, std::shared_ptr<void>> serviceMap;
	return serviceMap;
}

std::mutex& ServiceLocator::mtx()
{
	static std::mutex serviceMtx;
	return serviceMtx;
}

} // namespace Core


