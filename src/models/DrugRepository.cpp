#include "DrugRepository.h"

#include <algorithm>
#include <ranges>

namespace {
constexpr quint32 DRUG_DATA_VERSION = 1;
}

namespace Models {

DrugRepository::DrugRepository() : BaseRepository("drugs.bin") {}

bool DrugRepository::load() {
    return readFromFile([this](QDataStream& in) {
        quint32 version = 0;
        in >> version;
        if (version != DRUG_DATA_VERSION) {
            return false;
        }
        quint32 count = 0;
        in >> count;
        QVector<Drug> loaded;
        loaded.reserve(count);
        for (quint32 i = 0; i < count; ++i) {
            Drug d;
            in >> d;
            loaded.push_back(d);
        }
        drugs = loaded;
        return true;
    });
}

bool DrugRepository::save() const {
    return writeToFile([this](QDataStream& out) {
        out << DRUG_DATA_VERSION;
        out << quint32(drugs.size());
        for (const auto& d : drugs) {
            out << d;
        }
        return true;
    });
}

quint32 DrugRepository::addDrug(const Drug& d) {
    Drug copy = d;
    copy.id = nextDrugId();
    drugs.push_back(copy);
    return copy.id;
}

bool DrugRepository::updateDrug(const Drug& d) {
    for (auto& x : drugs) {
        if (x.id == d.id) {
            x = d;
            return true;
        }
    }
    return false;
}

bool DrugRepository::removeDrug(quint32 id) {
    const int before = drugs.size();
    const auto [first, last] = std::ranges::remove_if(drugs, [id](const Drug& drug) { return drug.id == id; });
    drugs.erase(first, last);
    return before != drugs.size();
}

Drug* DrugRepository::findDrug(quint32 id) {
    for (auto& x : drugs)
        if (x.id == id) return &x;
    return nullptr;
}

const Drug* DrugRepository::findDrugConst(quint32 id) const {
    for (const auto& x : drugs)
        if (x.id == id) return &x;
    return nullptr;
}

quint32 DrugRepository::nextDrugId() const {
    quint32 maxId = 0;
    for (const auto& d : drugs) {
        maxId = std::max(maxId, d.id);
    }
    return maxId + 1;
}

}  // namespace Models
