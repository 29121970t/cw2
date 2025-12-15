#pragma once

#include <QVector>

#include "BaseRepository.h"
#include "Entities.h"

namespace Models {

class DrugRepository : public BaseRepository {
   public:
    DrugRepository();

    bool load();
    bool save() const;

    const QVector<Drug>& allDrugs() const { return drugs; }

    quint32 addDrug(const Drug& d);
    bool updateDrug(const Drug& d);
    bool removeDrug(quint32 id);
    Drug* findDrug(quint32 id);
    const Drug* findDrugConst(quint32 id) const;

   private:
    quint32 nextDrugId() const;

    QVector<Drug> drugs;
};

}  // namespace Models
