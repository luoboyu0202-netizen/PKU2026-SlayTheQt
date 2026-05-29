#include "RelicFactory.h"
#include <QRandomGenerator>
#include <QDebug>

#include "entities/relics/PenNibRelic.h"
#include "entities/relics/OrichalcumRelic.h"
#include "entities/relics/BagOfPreparationRelic.h"
#include "entities/relics/AnchorRelic.h"
#include "entities/relics/BurningBloodRelic.h"
#include "entities/relics/VajraRelic.h"
#include "entities/relics/SneckoEyeRelic.h"
#include "GlobalSaveData.h"

Relic* RelicFactory::createRelic(const QString& relicId, QObject* parent) {

    Relic* relic = nullptr;

    if (relicId == "relic_pen_nib") relic = new PenNibRelic(parent);
    if (relicId == "relic_orichalcum") relic = new OrichalcumRelic(parent);
    if (relicId == "relic_bag_of_preparation") relic = new BagOfPreparationRelic(parent);
    if (relicId == "relic_anchor") relic = new AnchorRelic(parent);
    if (relicId == "relic_burning_blood") relic = new BurningBloodRelic(parent);
    if (relicId == "relic_vajra") relic = new VajraRelic(parent);
    if (relicId == "relic_snecko_eye") relic = new SneckoEyeRelic(parent);

    // 2. 🔴 灵魂灌注：如果档案馆里存了这个遗物的计数，恢复它！
    if (relic) {
        GlobalSaveData* save = GlobalSaveData::getInstance();
        if (save->relicCounters.contains(relicId)) {
            relic->setCounter(save->relicCounters[relicId]);
        }
    }

    qWarning() << "[RelicFactory] 找不到遗物 ID:" << relicId;
    return relic;
}
