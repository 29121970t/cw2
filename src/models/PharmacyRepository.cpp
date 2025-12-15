#include "PharmacyRepository.h"

#include <algorithm>
#include <ranges>

namespace {
constexpr quint32 PHARMACY_DATA_VERSION = 1;
}

namespace Models {
PharmacyRepository::PharmacyRepository() : BaseRepository("pharmacies.bin") {}

bool PharmacyRepository::load() {
    return readFromFile([this](QDataStream& in) { return deserialize(in); });
}

bool PharmacyRepository::save() const {
    return writeToFile([this](QDataStream& out) { return serialize(out); });
}

quint32 PharmacyRepository::addPharmacy(const Pharmacy& p) {
    Pharmacy copy = p;
    copy.id = nextPharmacyId();
    if (copy.hours.size() != 7) {
        copy.hours = QVector<QPair<QTime, QTime>>(7);
    }
    pharmacies.push_back(copy);
    return copy.id;
}

bool PharmacyRepository::updatePharmacy(const Pharmacy& p) {
    for (auto& x : pharmacies) {
        if (x.id == p.id) {
            x = p;
            return true;
        }
    }
    return false;
}

bool PharmacyRepository::removePharmacy(quint32 id) {
    const int before = pharmacies.size();
    const auto [first, last] = std::ranges::remove_if(pharmacies, [id](const Pharmacy& pharmacy) { return pharmacy.id == id; });
    pharmacies.erase(first, last);
    const auto range = std::ranges::remove_if(stocks, [id](const Stock& s) { return s.pharmacyId == id; });
    stocks.erase(range.begin(), range.end());
    return before != pharmacies.size();
}

Pharmacy* PharmacyRepository::findPharmacy(quint32 id) {
    for (auto& x : pharmacies)
        if (x.id == id) return &x;
    return nullptr;
}

const Pharmacy* PharmacyRepository::findPharmacyConst(quint32 id) const {
    for (const auto& x : pharmacies)
        if (x.id == id) return &x;
    return nullptr;
}

bool PharmacyRepository::setStock(quint32 pharmacyId, quint32 drugId, double price) {
    for (auto& s : stocks) {
        if (s.pharmacyId == pharmacyId && s.drugId == drugId) {
            s.price = price;
            return true;
        }
    }
    Models::Stock s;
    s.pharmacyId = pharmacyId;
    s.drugId = drugId;
    s.price = price;
    stocks.push_back(s);
    return true;
}

bool PharmacyRepository::removeStock(quint32 pharmacyId, quint32 drugId) {
    const int before = stocks.size();
    const auto range =
        std::ranges::remove_if(stocks, [pharmacyId, drugId](const Stock& s) { return s.pharmacyId == pharmacyId && s.drugId == drugId; });
    stocks.erase(range.begin(), range.end());
    return before != stocks.size();
}

QVector<Stock> PharmacyRepository::stocksForDrug(quint32 drugId) const {
    QVector<Stock> res;
    for (const auto& s : stocks)
        if (s.drugId == drugId) res.push_back(s);
    return res;
}

QVector<Stock> PharmacyRepository::stocksForPharmacy(quint32 pharmacyId) const {
    QVector<Stock> res;
    for (const auto& s : stocks)
        if (s.pharmacyId == pharmacyId) res.push_back(s);
    return res;
}

void PharmacyRepository::removeStocksByDrug(quint32 drugId) {
    const auto range = std::ranges::remove_if(stocks, [drugId](const Stock& s) { return s.drugId == drugId; });
    stocks.erase(range.begin(), range.end());
}

bool PharmacyRepository::deserialize(QDataStream& in) {
    quint32 version = 0;
    in >> version;
    if (version != PHARMACY_DATA_VERSION) {
        return false;
    }

    quint32 pharmaciesCount = 0;
    in >> pharmaciesCount;
    QVector<Pharmacy> loadedPharmacies;
    loadedPharmacies.reserve(pharmaciesCount);
    for (quint32 i = 0; i < pharmaciesCount; ++i) {
        Pharmacy p;
        in >> p;
        loadedPharmacies.push_back(p);
    }

    quint32 stocksCount = 0;
    in >> stocksCount;
    QVector<Stock> loadedStocks;
    loadedStocks.reserve(stocksCount);
    for (quint32 i = 0; i < stocksCount; ++i) {
        Stock s;
        in >> s;
        loadedStocks.push_back(s);
    }

    pharmacies = loadedPharmacies;
    stocks = loadedStocks;
    return true;
}

bool PharmacyRepository::serialize(QDataStream& out) const {
    out << PHARMACY_DATA_VERSION;
    out << quint32(pharmacies.size());
    for (const auto& p : pharmacies) {
        out << p;
    }
    out << quint32(stocks.size());
    for (const auto& s : stocks) {
        out << s;
    }
    return true;
}

quint32 PharmacyRepository::nextPharmacyId() const {
    quint32 maxId = 0;
    for (const auto& p : pharmacies) {
        maxId = std::max(maxId, p.id);
    }
    return maxId + 1;
}

}  // namespace Models
