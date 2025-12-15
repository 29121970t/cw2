#pragma once

#include <QDateTime>

#include "../models/Entities.h"

namespace Utils {

inline bool isPharmacyOpenNow(const Models::Pharmacy& p, const QDateTime& when = QDateTime::currentDateTime()) {
    if (p.hours.size() < 7) return false;
    const int weekday = when.date().dayOfWeek();
    if (weekday < 1 || weekday > p.hours.size()) return false;
    const auto& [openTime, closeTime] = p.hours[weekday - 1];
    if (!openTime.isValid() || !closeTime.isValid()) return false;
    const QTime nowT = when.time();
    return nowT >= openTime && nowT <= closeTime;
}

}  // namespace Utils
