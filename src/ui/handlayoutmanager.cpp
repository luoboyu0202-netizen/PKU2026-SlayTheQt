#include "HandLayoutManager.h"
#include <QPropertyAnimation>
#include <BattleEngine.h>

HandLayoutManager::HandLayoutManager(BattleScene* scene, Player* player, CardManager* cardManager, QObject* parent)
    : QObject(parent), m_scene(scene), m_player(player), m_cardManager(cardManager) {

    BattleEngine* engine = BattleEngine::getInstance();

    // 1. 监听底层抽牌
    connect(cardManager, &CardManager::cardDrawn, this, &HandLayoutManager::onCardDrawn);
    // 2. 监听玩家费用变化
    connect(player, &Player::energyChanged, this, &HandLayoutManager::onEnergyChanged);
    // 1. 在构造函数里增加监听绑定的代码喵：
    connect(cardManager, &CardManager::cardDiscarded, this, &HandLayoutManager::onCardDiscarded);

    // 🔴【喵娘的魔法连线】：把大脑的烧牌信号，接到我们刚刚写的槽函数上！
    if (m_cardManager) {
        connect(m_cardManager, &CardManager::cardExhausted, this, &HandLayoutManager::onCardExhausted);
    }

    // 在构造函数里：
    connect(engine, &BattleEngine::selectionModeEnded, this, &HandLayoutManager::onSelectionModeEnded);

    // ========================================================
    // 🌌【能力牌离场】：安静地抹杀原版肉体！
    // ========================================================
    connect(cardManager, &CardManager::cardMovedToPower, this, [this](Card* logicCard) {

        for (int i = m_handItems.size() - 1; i >= 0; --i) {
            CardItem* cItem = m_handItems[i];

            // 找到那个留在手牌里的原版肉体
            if (cItem->getLogicCard() == logicCard) {

                // 1. 从手牌管家的名册里除名
                m_handItems.removeAt(i);

                // 2. 🔴 物理超度：不要播放任何离场动画，直接藏起来并销毁！
                // 因为替身已经在 BattleView 里大放异彩了喵！
                cItem->hide();
                cItem->deleteLater();

                // 3. 剩下的手牌立刻聚拢填补空位！
                recalculateLayout();

                break;
            }
        }
    });

}

void HandLayoutManager::onCardDrawn(Card* logicCard) {
    // 1. 创建 UI 肉体
    CardItem* cardItem = new CardItem(logicCard);

    // 初始位置设在左下角 (抽牌堆的位置)
    cardItem->setPos(-100, 1080);
    cardItem->setScale(0.1); // 初始很小
    cardItem->checkPlayability(m_player->getEnergy()); // 刚抽出来先确认能不能打

    m_scene->addItem(cardItem);
    m_handItems.append(cardItem);

    // 2. 将这件武器的扳机，连通到管家身上
    connect(cardItem, &CardItem::cardVisualDestroyed, this, &HandLayoutManager::onCardVisualDestroyed);
    connect(cardItem, &CardItem::cardPlayedRequest, this, &HandLayoutManager::cardPlayedRequest);

    // 3. 重新计算所有手牌的扇形家园坐标
    recalculateLayout();
}

void HandLayoutManager::onCardVisualDestroyed(CardItem* item) {
    m_handItems.removeOne(item);
    recalculateLayout(); // 一张牌消失了，剩下的牌立刻往中间聚拢合拢空缺！
}

void HandLayoutManager::onEnergyChanged(int current, int max) {
    Q_UNUSED(max);
    for (CardItem* item : m_handItems) {
        item->checkPlayability(current); // 同步置灰状态！
    }
}

void HandLayoutManager::recalculateLayout() {
    int count = m_handItems.size();
    if (count == 0) return;

    // ========================================================
    // 📏 1. 变宽：增加基础间距，减少拥挤惩罚
    // 原版：150.0 - (count * 5.0)
    // 升级：基础间距拉大到 180，每多一张牌只缩减 3 像素
    // ========================================================
    qreal spacing = 180.0 - (count * 3.0);
    qreal totalWidth = (count - 1) * spacing;
    qreal startX = 960.0 - totalWidth / 2.0;

    for (int i = 0; i < count; ++i) {
        CardItem* item = m_handItems[i];
        if (item->isSuspended()) continue;

        qreal targetX = startX + i * spacing;

        // ========================================================
        // 📐 2. 降低曲率：加大分母，让角度变平缓！
        // 原版：除以 12.0
        // 升级：除以 25.0（分母越大，两边倾斜的角度就越小，扇形越平）
        // ========================================================
        qreal angle = (targetX - 960.0) / 25.0;

        // ========================================================
        // 🎯 3. 减少下坠半径 & 适应大卡牌：
        // 原版：920.0 + qAbs(angle) * 3.0
        // 升级：因为卡牌变大了，基础 Y 轴向上提（改为 850）防止超出屏幕底边缘。
        // 下沉乘数改为 1.2，让最边缘的牌也不会掉下去太多。
        // ========================================================
        qreal targetY = 1000 + qAbs(angle) * 1.2;

        item->setHomeState(QPointF(targetX, targetY), angle);

        // ========================================================
        // 🔍 4. 增大卡牌图元 (不影响战斗和商店逻辑)
        // 直接在这里将手牌的缩放比例拉高（例如 1.2 倍）
        // ========================================================
        item->setBaseScale(1.1);

        item->animateToHome();
    }
}

// 2. 在文件末尾实现这个函数喵：
void HandLayoutManager::onCardDiscarded(Card* logicCard) {
    // 🔴【修正1】：必须先请出大脑指挥官，才能问他是不是在时停喵！
    BattleEngine* engine = BattleEngine::getInstance();

    // 采用逆向遍历，防止在循环中直接删除元素导致越界喵
    for (int i = m_handItems.size() - 1; i >= 0; --i) {
        CardItem* item = m_handItems[i];
        if (item->getLogicCard() == logicCard) {

            // ========================================================
            // 🔴【施法拦截】：这张牌是不是引发了时停结界？！
            // ========================================================
            if (engine && engine->isSelectingHandCard()) {
                qDebug() << "[UI] 施法牌滞空！燃烧契约悬浮于天际喵！";

                // 1. 让它飞到天上！(🔴【修正2】：这里是 item，不是 cItem 喵！)
                item->animateSuspendInCenter();

                // ========================================================
                // 🔴【全场觉醒广播】：结界已开！强制手牌里的所有人重新检查状态！
                // 这时 expensive 的牌就会啪地一下全部亮起来喵！
                // ========================================================
                for (CardItem* handItem : m_handItems) {
                    handItem->checkPlayability(m_player->getEnergy());
                }

                // 2. 极其重要：直接 break！不剔除，不销毁，把它“扣留”在手里！
                break;
            } else {
                // 🟢 正常打出：立刻从 UI 列表中剔除！这样重新排版时就不会影响它了！
                m_handItems.removeAt(i);

                // 让卡牌自己放心地飞向坟墓（弃牌堆）
                item->animatePlayAndDiscard();

                // 顺便让剩下的手牌丝滑地聚拢排版
                recalculateLayout();
                break;
            }
        }
    }
}

void HandLayoutManager::onCardExhausted(Card* logicCard) {
    // 1. 在管家自己保管的 m_handItems 列表里找！
    for (CardItem* cItem : m_handItems) {

        // 2. 找到了被大脑宣判死刑的那张牌！
        if (cItem->getLogicCard() == logicCard) {

            qDebug() << "[LayoutManager] 收到烧牌指令，开始播放化为灰烬动画喵：" << logicCard->getName();

            // 3. 播放烧毁特效！
            cItem->animateTrueExhaust();

            // 4. 直接呼叫管家自己的“卡牌视觉销毁”处理函数！
            // 它会立刻把这卡牌从 m_handItems 移除，并调用 recalculateLayout() 重新扇形排版！
            onCardVisualDestroyed(cItem);

            break; // 处决完毕，收工喵！
        }
    }
}

void HandLayoutManager::onSelectionModeEnded() {
    for (CardItem* cItem : m_handItems) {
        // 找到天上的施法牌！
        if (cItem->isSuspended()) {

            cItem->setSuspended(false); // 取消滞空
            cItem->setScale(1.0);       // 缩回原大小

            // 🔴 补上它本该承受的动画：飞向弃牌堆！
            cItem->animatePlayAndDiscard();

            // 🔴 视觉销毁！管家会把它移出手牌列表，并重新扇形排版！
            onCardVisualDestroyed(cItem);
        }
    }
    // ========================================================
    // 🔴【重回现实广播】：时停结束！让存活下来的手牌根据常规费用重新变暗！
    // ========================================================
    for (CardItem* handItem : m_handItems) {
        handItem->checkPlayability(m_player->getEnergy());
    }
}
