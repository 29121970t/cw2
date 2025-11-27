#include "ServiceLocator.h"

namespace Core {

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map()
{
	return gServiceMap;
}

std::mutex& ServiceLocator::mtx()
{
	return gServiceMtx;
}

} // namespace Core


