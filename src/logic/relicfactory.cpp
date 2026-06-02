#include "RelicFactory.h"
#include <QRandomGenerator>
#include <QDebug>
#include "GlobalSaveData.h"

// 引入所有 15 个遗物的头文件 (请根据你的实际路径微调)
#include "entities/relics/PenNibRelic.h"
#include "entities/relics/OrichalcumRelic.h"
#include "entities/relics/BagOfPreparationRelic.h"
#include "entities/relics/AnchorRelic.h"
#include "entities/relics/BurningBloodRelic.h"
#include "entities/relics/VajraRelic.h"
#include "entities/relics/SneckoEyeRelic.h"
#include "relics/LanternRelic.h"
#include "relics/HappyFlowerRelic.h"
#include "relics/ShurikenRelic.h"
#include "relics/SmoothStoneRelic.h"
#include "relics/MercuryHourglassRelic.h"
#include "relics/ToriiRelic.h"         // ⛩️ 鸟居
#include "relics/PrayerWheelRelic.h"   // 📿 念珠手镯
#include "relics/IceCreamRelic.h"      // 🍦 冰淇淋

// ========================================================
// 📜 【全图鉴总表】：唯一真理！摇奖池直接从这里读取！
// 以后加新遗物，只需在这里加 ID，不用再去改 generateRandomRelic 喵！
// ========================================================
static const QStringList ALL_REGISTERED_RELICS = {
    "relic_pen_nib", "relic_orichalcum", "relic_bag_of_preparation",
    "relic_anchor", "relic_burning_blood", "relic_vajra", "relic_snecko_eye",
    "relic_lantern", "relic_happy_flower", "relic_shuriken",
    "relic_smooth_stone", "relic_mercury_hourglass",
    "relic_torii", "relic_prayer_wheel", "relic_ice_cream"
};

Relic* RelicFactory::createRelic(const QString& relicId, QObject* parent) {

    Relic* relic = nullptr;

    // 1. 🏭 肉身锻造：必须使用 else if 串联，且统统赋值给 relic 变量！
    if (relicId == "relic_pen_nib") relic = new PenNibRelic(parent);
    else if (relicId == "relic_orichalcum") relic = new OrichalcumRelic(parent);
    else if (relicId == "relic_bag_of_preparation") relic = new BagOfPreparationRelic(parent);
    else if (relicId == "relic_anchor") relic = new AnchorRelic(parent);
    else if (relicId == "relic_burning_blood") relic = new BurningBloodRelic(parent);
    else if (relicId == "relic_vajra") relic = new VajraRelic(parent);
    else if (relicId == "relic_snecko_eye") relic = new SneckoEyeRelic(parent);
    else if (relicId == "relic_lantern") relic = new LanternRelic(parent);
    else if (relicId == "relic_happy_flower") relic = new HappyFlowerRelic(parent);
    else if (relicId == "relic_shuriken") relic = new ShurikenRelic(parent);
    else if (relicId == "relic_smooth_stone") relic = new SmoothStoneRelic(parent);
    else if (relicId == "relic_mercury_hourglass") relic = new MercuryHourglassRelic(parent);
    else if (relicId == "relic_torii") relic = new ToriiRelic(parent);
    else if (relicId == "relic_prayer_wheel") relic = new PrayerWheelRelic(parent);
    else if (relicId == "relic_ice_cream") relic = new IceCreamRelic(parent);

    // 2. 🔴 灵魂灌注：恢复计数器
    if (relic) {
        GlobalSaveData* save = GlobalSaveData::getInstance();
        if (save->relicCounters.contains(relicId)) {
            relic->setCounter(save->relicCounters[relicId]);
        }

        // 🔴 救命补丁：成功创建并灌注后，立刻返回！阻止警告发生！
        return relic;
    }

    // 3. 兜底报错：只有真正的黑户才会走到这里
    qWarning() << "[RelicFactory] 找不到遗物 ID:" << relicId;
    return nullptr;
}

QString RelicFactory::generateRandomRelic(const QStringList& ownedRelicIds) {
    // 1. 查戶口：直接使用顶部的全局总表剔除玩家已經擁有的遺物
    QStringList availableRelics;
    for (const QString& id : ALL_REGISTERED_RELICS) {
        if (!ownedRelicIds.contains(id)) {
            availableRelics.append(id);
        }
    }

    // 2. 如果遺物池被抽乾了，返回空字串
    if (availableRelics.isEmpty()) {
        return "";
    }

    // 3. 隨機搖出一個返回
    int randomIndex = QRandomGenerator::global()->bounded(availableRelics.size());
    return availableRelics[randomIndex];
}