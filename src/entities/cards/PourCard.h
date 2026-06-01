#pragma once
#include "Card.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include <QDebug>
#include <QTimer>

class PourCard : public Card {
    Q_OBJECT
public:
    explicit PourCard(QObject* parent = nullptr)
        : Card("card_pour", QStringLiteral("倾泻"), 0, false, parent) {

        m_isXCost = true; // 🔴 UI 视觉欺骗：让它在左上角显示为 X 费

        // 🔴 坚决贯彻协议：使用魔法数字管理额外打出的次数！
        m_secondaryValue = 0; // 未升级时，没有额外加成

        // 🟢 智能模板注入：保留 !M! 标签，绝不在 upgrade 里写死字符串！
        m_rawDescription = QStringLiteral("消耗所有的能量。打出你抽牌堆顶部的 X+!M! 张牌。");
        m_description = m_rawDescription;

        m_type = CardType::Attack;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/pour.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();

            // 🔴 极致优雅：只操作数值，文本解析器全自动接管！
            m_secondaryValue += 1;

            qDebug() << "[Card]" << m_name << "升级完毕！当前额外打出数：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);

        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !source) return;

        // 1. 获取玩家当前真实能量，并榨干！
        int currentEnergy = source->getEnergy();
        source->useEnergy(currentEnergy);

        // 2. 🔴 逻辑与视觉完美统一：真实费用 + 魔法数字的加成！
        int playCount = currentEnergy + m_secondaryValue;

        qDebug() << "[Card Engine] 倾泻发动！榨干了" << currentEnergy << "费，外加" << m_secondaryValue << "次额外加成，总共拉出" << playCount << "张顶牌！";

        // 4. 🔴 枪林弹雨：加入阶梯式时间延迟，形成视觉上的连发感！
        int delayBetweenShots = 2000; // 每隔 600 毫秒射出一张牌，你可以根据动画长度自己调喵！

        for (int i = 0; i < playCount; ++i) {
            // 绝招：用 i * 延迟时间，让第一张 0ms 飞出，第二张 600ms 飞出，第三张 1200ms...
            QTimer::singleShot(i * delayBetweenShots, engine, [engine]() {

                // 🛡️ 极其重要的安全锁：如果在这个漫长的延迟期间，
                // 主角已经死了，或者怪物已经死光了（战斗结束），就立刻停止倾泻！
                if (!engine || engine->getPlayer()->isDead()) return;

                bool allEnemiesDead = true;
                for (auto e : engine->getEnemies()) {
                    if (e && !e->isDead()) {
                        allEnemiesDead = false;
                        break;
                    }
                }
                if (allEnemiesDead) {
                    qDebug() << "[Card Engine] 怪物已经死光光啦，停止倾泻鞭尸喵！";
                    return;
                }

                // 安全通过，强行抽出一张顶牌！
                engine->triggerPlayTopCard(false);
            });
        }
    }
};
