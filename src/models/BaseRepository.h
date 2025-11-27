#pragma once

#include <functional>
#include <QString>

namespace Models {

class BaseRepository {
public:
	explicit BaseRepository(const QString &fileName);
	virtual ~BaseRepository() = default;

protected:
	QString dataFilePath() const;
	bool readFromFile(const std::function<bool(QDataStream&)> &reader);
	bool writeToFile(const std::function<bool(QDataStream&)> &writer) const;

private:
	QString fileName;
};

} // namespace Models


