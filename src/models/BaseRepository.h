#pragma once

#include <QString>
#include <functional>

class QDataStream;

namespace Models {

class BaseRepository {
   public:
    explicit BaseRepository(const QString& fileName);
    virtual ~BaseRepository() = default;

   protected:
    QString dataFilePath() const;
    bool readFromFile(const std::function<bool(QDataStream&)>& reader) const;
    bool writeToFile(const std::function<bool(QDataStream&)>& writer) const;

   private:
    QString fileName;
};

}  // namespace Models
