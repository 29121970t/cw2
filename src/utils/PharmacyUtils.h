#pragma once

#include <QDateTime>
#include "../models/Entities.h"

namespace Utils {

// Returns true if pharmacy is open at 'when' (defaults to current local time)
inline bool isPharmacyOpenNow(const Models::Pharmacy &p, const QDateTime &when = QDateTime::currentDateTime())
{
	if (p.hours.size() < 7) return false;
	const int weekday = when.date().dayOfWeek(); // 1..7
	if (weekday < 1 || weekday > p.hours.size()) return false;
	const auto pr = p.hours[weekday-1];
	if (!pr.first.isValid() || !pr.second.isValid()) return false;
	const QTime nowT = when.time();
	return nowT >= pr.first && nowT <= pr.second;
}

}


