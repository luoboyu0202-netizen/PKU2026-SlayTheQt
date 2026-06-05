#ifndef GLOBALSAVEDATA_H
#define GLOBALSAVEDATA_H

#include <QString>
#include <QList>
#include <QMap>
// 🔴 新增：引入 JSON 和 文件操作必需的头文件
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
class GlobalSaveData {
public:
    // 单例模式获取全局唯一实例
    static GlobalSaveData* getInstance() {
        static GlobalSaveData instance;
        return &instance;
    }

    // ========================================================
    // 🎲 PRD 偽隨機系統的靈魂變數
    // ========================================================
    QStringList availableEvents; // 事件抽獎袋
    int questionMarkMonsterChance = 10; // 問號節點遇到怪物的初始機率 (10%)

    // 主角全局基础属性
    int currentHp = 80;
    int maxHp = 80;
    int gold = 999;
    int maxEnergy = 3;
    int cardRemovalCost = 75;

    // --- 问号事件：留给自己的讯息 (Note For Yourself) ---
    QString storedCardId = "card_strike"; // 首次默认为打击 (原作为铁斩波，此处用已有卡牌)
    bool isStoredCardUpgraded = false;

    // 核心安全设计：只存 ID 列表，绝不存实体指针！
    QList<QString> deckIds;
    QList<QString> relicIds;

    // 初始化新游戏的初始状态
    void initNewGame() {
        currentHp = 80;
        gold = 999;
        cardRemovalCost = 75;
        deckIds.clear();
        relicIds.clear();

        // 塞入初始卡牌 ID (参考你 CardFactory 的可用 ID)
        deckIds.append("card_test");

        // 初始遗物 ID (参考你 RelicFactory 的可用 ID)
        relicIds.append("relic_burning_blood");

    }

    // 🔴【新增】：记忆保险箱，记录 <遗物ID, 计数值>
    QMap<QString, int> relicCounters;
    // ========================================================
    // 💾 【新增】存档系统核心 API
    // ========================================================

    // 获取存档文件的绝对路径（默认保存在可执行文件同级目录下的 savegame.json）
    QString getSaveFilePath() const {
        return QCoreApplication::applicationDirPath() + "/savegame.json";
    }

    // 判断当前是否存在存档
    bool hasSaveFile() const {
        return QFile::exists(getSaveFilePath());
    }

    void deleteSaveFile() {
        QFile file(getSaveFilePath());
        if (file.exists()) {
            file.remove();
            qDebug() << "[存档系统] 💀 玩家阵亡，存档已被抹除！";
        }
    }

    // 写入存档到硬盘
    void saveToFile() {
        QJsonObject rootObj;
        rootObj["currentHp"] = currentHp;
        rootObj["maxHp"] = maxHp;
        rootObj["gold"] = gold;
        rootObj["maxEnergy"] = maxEnergy;

        // 转换卡牌 ID 列表
        QJsonArray deckArray;
        for (const QString& id : deckIds) { deckArray.append(id); }
        rootObj["deckIds"] = deckArray;

        // 转换遗物 ID 列表
        QJsonArray relicArray;
        for (const QString& id : relicIds) { relicArray.append(id); }
        rootObj["relicIds"] = relicArray;

        // 写入文件
        QJsonDocument doc(rootObj);
        QFile file(getSaveFilePath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
            qDebug() << "[存档系统] 游戏进度已成功保存至:" << getSaveFilePath();
        } else {
            qDebug() << "[存档系统] 🚨 无法创建存档文件!";
        }
    }

    // 从硬盘读取存档
    void loadFromFile() {
        QFile file(getSaveFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "[存档系统] 🚨 找不到存档文件!";
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject rootObj = doc.object();

        // 恢复数据
        currentHp = rootObj["currentHp"].toInt(80); // 找不到就默认80
        maxHp = rootObj["maxHp"].toInt(80);
        gold = rootObj["gold"].toInt(99);
        maxEnergy = rootObj["maxEnergy"].toInt(3);

        deckIds.clear();
        QJsonArray deckArray = rootObj["deckIds"].toArray();
        for (int i = 0; i < deckArray.size(); ++i) {
            deckIds.append(deckArray[i].toString());
        }

        relicIds.clear();
        QJsonArray relicArray = rootObj["relicIds"].toArray();
        for (int i = 0; i < relicArray.size(); ++i) {
            relicIds.append(relicArray[i].toString());
        }

        qDebug() << "[存档系统] 读档成功！当前HP:" << currentHp << "卡牌数量:" << deckIds.size();
    }

private:
    GlobalSaveData() {}
};

#endif // GLOBALSAVEDATA_H