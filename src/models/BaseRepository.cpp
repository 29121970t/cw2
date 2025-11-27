#include "BaseRepository.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace Models {

BaseRepository::BaseRepository(const QString &fileName)
	: fileName(fileName)
{
}

QString BaseRepository::dataFilePath() const
{
	const auto base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir().mkpath(base);
	return base + QDir::separator() + fileName;
}

bool BaseRepository::readFromFile(const std::function<bool(QDataStream&)> &reader)
{
	QFile f(dataFilePath());
	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		return false;
	}
	QDataStream in(&f);
	in.setVersion(QDataStream::Qt_6_0);
	if (!reader) {
		return false;
	}
	return reader(in);
}

bool BaseRepository::writeToFile(const std::function<bool(QDataStream&)> &writer) const
{
	QFile f(dataFilePath());
	if (!f.open(QIODevice::WriteOnly)) {
		return false;
	}
	QDataStream out(&f);
	out.setVersion(QDataStream::Qt_6_0);
	if (!writer) {
		return false;
	}
	return writer(out);
}

} // namespace Models

