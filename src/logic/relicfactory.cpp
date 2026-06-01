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

QString RelicFactory::generateRandomRelic(const QStringList& ownedRelicIds) {
    // 1. 羅列所有可能掉落的遺物池
    QStringList allRelics = {
        "relic_pen_nib",
        "relic_orichalcum",
        "relic_bag_of_preparation",
        "relic_anchor",
        "relic_burning_blood",
        "relic_vajra",
        "relic_snecko_eye"
    };

    // 2. 查戶口：剔除玩家已經擁有的遺物
    QStringList availableRelics;
    for (const QString& id : allRelics) {
        if (!ownedRelicIds.contains(id)) {
            availableRelics.append(id);
        }
    }

    // 3. 如果遺物池被抽乾了，返回空字串
    if (availableRelics.isEmpty()) {
        return "";
    }

    // 4. 隨機搖出一個返回
    int randomIndex = QRandomGenerator::global()->bounded(availableRelics.size());
    return availableRelics[randomIndex];
}