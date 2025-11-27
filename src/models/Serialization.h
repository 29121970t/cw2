#pragma once

#include "Entities.h"
#include <QDataStream>

namespace Models {

// QDataStream serialization operators
QDataStream& operator<<(QDataStream& out, const Models::Drug& d);
QDataStream& operator>>(QDataStream& in, Models::Drug& d);

QDataStream& operator<<(QDataStream& out, const Models::Pharmacy& p);
QDataStream& operator>>(QDataStream& in, Models::Pharmacy& p);

QDataStream& operator<<(QDataStream& out, const Models::Stock& s);
QDataStream& operator>>(QDataStream& in, Models::Stock& s);

} // namespace Models

