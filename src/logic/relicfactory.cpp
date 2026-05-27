#include "RelicFactory.h"
#include <QRandomGenerator>
#include <QDebug>

#include "entities/relics/PenNibRelic.h"
#include "entities/relics/OrichalcumRelic.h"
#include "entities/relics/BagOfPreparationRelic.h"
#include "entities/relics/AnchorRelic.h"

QList<QString> RelicFactory::getAllAvailableRelicIds() {
    return {
        "relic_pen_nib", "relic_orichalcum",
        "relic_bag_of_preparation", "relic_anchor"
    };
}

Relic* RelicFactory::createRelic(const QString& relicId, QObject* parent) {
    if (relicId == "relic_pen_nib") return new PenNibRelic(parent);
    if (relicId == "relic_orichalcum") return new OrichalcumRelic(parent);
    if (relicId == "relic_bag_of_preparation") return new BagOfPreparationRelic(parent);
    if (relicId == "relic_anchor") return new AnchorRelic(parent);

    qWarning() << "[RelicFactory] 找不到遗物 ID:" << relicId;
    return nullptr;
}

Relic* RelicFactory::generateRandomRelic(QObject* parent) {
    QList<QString> pool = getAllAvailableRelicIds();
    if (pool.isEmpty()) return nullptr;

    int randomIndex = QRandomGenerator::global()->bounded(pool.size());
    return createRelic(pool[randomIndex], parent);
}
