#ifndef CARDFACTORY_H
#define CARDFACTORY_H

#include <QString>
#include <QList>
#include "entities/cards/Card.h" // 🔴 确保这里的路径指向你的卡牌基类喵
#include <QRandomGenerator>

class CardFactory {
public:
    // ========================================================
    // 🛠️ 核心功能 1：精准锻造 (根据 ID 印卡)
    // 适用场景：怪物塞状态牌(如黏液)、商店买牌、开局发基础牌
    // ========================================================
    static Card* createCard(const QString& cardId, QObject* parent = nullptr);

    // ========================================================
    // 🎲 核心功能 2：虚空造物 (获取完全随机的卡牌)
    // 适用场景：【枯木树枝】、【发现】牌、随机卡牌奖励掉落
    // ========================================================
    static Card* generateRandomCard(QObject* parent = nullptr);

    // ========================================================
    // 📚 核心功能 3：获取全图鉴卡池 (可选，但极其推荐)
    // 适用场景：配合随机函数使用，或者以后做“卡牌图鉴UI”时遍历查阅
    // ========================================================
    static QList<QString> getAllAvailableCardIds();

    // ========================================================
    // 🃏 战利品发牌员：生成三选一的不重复卡牌列表
    // ========================================================
    static inline QList<QString> generateCardRewardIds(int count = 3) {
        // 1. 获取全图鉴卡池
        QList<QString> pool = getAllAvailableCardIds();

        // 🔴 细节优化：战利品通常不会掉落初始的“打击”和“防御”
        pool.removeAll("card_strike");
        pool.removeAll("card_defend");

        QList<QString> result;

        // 2. 经典去重抽卡算法
        for (int i = 0; i < count; ++i) {
            if (pool.isEmpty()) break; // 卡池抽干了就停手

            // 随机摇一个索引
            int randomIndex = QRandomGenerator::global()->bounded(pool.size());

            // 把抽到的卡塞进结果里
            result.append(pool[randomIndex]);

            // 🔴 极其关键：从卡池里把这张卡删掉，保证下一轮绝对不会抽到重复的！
            pool.removeAt(randomIndex);
        }

        return result;
    }

private:
    // 🛡️ 架构师的绝对防御：
    // 把构造函数私有化！工厂只是个工具箱，绝对不允许别人 new CardFactory() 喵！
    CardFactory() = default;
    ~CardFactory() = default;
};

#endif // CARDFACTORY_H
