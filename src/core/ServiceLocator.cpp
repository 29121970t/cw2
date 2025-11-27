#include "ServiceLocator.h"

namespace Core {

namespace {
const inline std::unordered_map<std::type_index, std::shared_ptr<void>> g_serviceMap{};
const inline std::mutex g_serviceMtx{};
}

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map()
{
	return g_serviceMap;
}

std::mutex& ServiceLocator::mtx()
{
	return g_serviceMtx;
}

} // namespace Core


