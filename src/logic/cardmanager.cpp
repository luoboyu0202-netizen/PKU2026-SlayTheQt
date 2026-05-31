#include "CardManager.h"
#include <random>    // 引入现代 C++ 随机数库
#include <algorithm> // 引入洗牌算法 std::shuffle
#include <chrono>    // 引入时间库作为随机种子
#include <QDebug>
#include <QRandomGenerator>
#include <BattleEngine.h>

CardManager::CardManager(QObject* parent) : QObject(parent) {}

// ==========================================================
// 1. 接管系统组数据，初始化战斗牌库
// ==========================================================
void CardManager::initializeDeck(const QList<Card*>& masterDeck) {
    m_masterDeck = masterDeck; // 把其他小组传来的数据存底备用

    // 战斗开始，将总牌库【浅拷贝】一份给抽牌堆
    m_drawPile = m_masterDeck;

    // 🔴【核心随机算法】：使用当前系统时间的时间戳作为高精度种子
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed); // 启动随机引擎

    // 将抽牌堆彻底打乱！
    std::shuffle(m_drawPile.begin(), m_drawPile.end(), rng);

    emitPileCounts(); // 通知 UI 抽牌堆数字变了
}

// ==========================================================
// 2. 真·抽牌逻辑
// ==========================================================
void CardManager::drawCards(int count) {
    BattleEngine* engine = BattleEngine::getInstance();
    if (!engine || !engine->getPlayer()) return;

    // 🔴【新增】：连击计数器！用来记录在这一次抽牌批次里，有几张“打击”被截获了
    int hellFiendTriggerCount = 0;
    int staggerDelay = 1200; // 每张牌之间间隔 600 毫秒飞出

    for (int i = 0; i < count; ++i) {
        // 如果抽牌堆空了，需要洗牌
        if (m_drawPile.isEmpty()) {
            // 如果连弃牌堆都空了，说明真的没牌可抽了，直接打断施法
            if (m_discardPile.isEmpty()) {
                break;
            }
            shuffleDiscardToDraw(); // 触发洗牌！
        }

        // 从打乱后的抽牌堆尾部（相当于牌堆顶）拿走一张牌
        Card* drawnCard = m_drawPile.takeLast();

        // ========================================================
        // 🚨【地狱狂徒雷达检测】：拦截名字里含有“打击”的牌！
        // ========================================================

        // (🗑️ 喵娘帮你删掉了这里重复获取 engine 的那行代码喵！)

        int hellFiendLevel = engine->getPlayer()->getStatusManager()->getStatus(StatusType::HellFiend);

        if (hellFiendLevel > 0 && drawnCard->getName().contains(QStringLiteral("打击"))) {
            qDebug() << "[CardManager] 🚨 地狱狂徒雷达响应！截获第" << (hellFiendTriggerCount + 1) << "张卡牌：" << drawnCard->getName();

            // 🔴 1. 核心魔法：算出这张牌该等多久？
            // 第一张 0ms，第二张 600ms，第三张 1200ms...
            int currentDelay = hellFiendTriggerCount * staggerDelay;

            // 🔴 2. 用定时器把信号发射器给“延时悬挂”起来！
            QTimer::singleShot(currentDelay, engine, [engine, drawnCard]() {
                // 🛡️ 安全锁：如果到了要飞出卡牌的时候，主角已经被反伤刺甲弹死了，就赶紧停下！
                if (!engine || engine->getPlayer()->isDead()) return;

                // 真正呼叫 UI，把卡牌甩到屏幕中央！
                emit engine->topCardRevealed(drawnCard, false);
            });

            // 🔴 3. 计数器加 1，让下一张不幸被抽到的打击去后面排队！
            hellFiendTriggerCount++;

            // 4. 结束本次单张抽牌循环，继续抽下一张 (坚决不让它进手牌！)
            continue;
        }

        m_hand.append(drawnCard);
        emit cardDrawn(drawnCard); // 通知大管家生成视觉卡牌
    }
    emitPileCounts();
}

// ==========================================================
// 3. 洗牌逻辑 (弃牌堆 -> 抽牌堆)
// ==========================================================
void CardManager::shuffleDiscardToDraw() {
    // 把弃牌堆的牌全部塞进抽牌堆
    m_drawPile.append(m_discardPile);
    m_discardPile.clear();

    // 再次调用高精度随机打乱！
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(m_drawPile.begin(), m_drawPile.end(), rng);

    emit deckShuffled(); // 触发 UI 的漫天流金粒子动画！
    emitPileCounts();
}

// ==========================================================
// 4. 其他常规逻辑 (保持不变，确保 emitPileCounts() 存在)
// ==========================================================
void CardManager::endTurnProcess() {
    // 采用逆向遍历，防止边遍历边删除导致越界喵！
    for (int i = m_hand.size() - 1; i >= 0; --i) {
        Card* card = m_hand[i];

        // ========================================================
        // 🔴 1. 触发回合结束的卡牌效果（比如【灼伤】扣血）
        // ========================================================
        card->triggerOnEndOfTurn();

        // ========================================================
        // 🔴 2. 虚无判定：是化为灰烬，还是进入弃牌堆？
        // ========================================================
        if (card->isEthereal()) {

            // 从手牌中移除，送入墓地！
            m_hand.removeAt(i);
            m_exhaustPile.append(card);

            qDebug() << "[CardManager] 虚无生效，卡牌消散：" << card->getName();

            // 💥 发射消耗信号！
            // UI层的管家听到后，会自动拦截并播放你上回合写好的“冥灰崩解”神级特效喵！
            emit cardExhausted(card);

        } else {
            // 普通卡牌，正常进入弃牌堆
            m_hand.removeAt(i);
            m_discardPile.append(card);
            emit cardDiscarded(card);
        }
    }
    emitPileCounts(); // 更新 UI 牌堆数字
}

void CardManager::moveToDiscard(Card* card) {
    if (m_hand.removeOne(card)) {
        m_discardPile.append(card);
        emit cardDiscarded(card);
        emitPileCounts();
    }
}

void CardManager::moveToExhaust(Card* card) {

    if (!card) {
        qDebug() << "🛑 [CardManager] 错误：尝试移动空指针到消耗堆！";
        return;
    }

    // 🔴 监控点：确认进没进堆
    qDebug() << "📦 [CardManager] 正在将卡牌移入消耗堆:" << card->getName();

    if (m_hand.removeOne(card)) {
        m_exhaustPile.append(card);
        emit cardExhausted(card);
        emitPileCounts();
    }
}

void CardManager::emitPileCounts() {
    emit pileCountsChanged(m_drawPile.size(), m_discardPile.size(), m_exhaustPile.size());
}

// 🟢【实现消耗逻辑】：从手牌剥离，扔进墓地！
void CardManager::exhaustCard(Card* card) {
    if (!card) return;

    // 1. 尝试从手牌列表中移除它
    if (m_hand.removeOne(card)) {
        // 2. 扔进墓地
        m_exhaustPile.append(card);

        qDebug() << "[CardManager] 灵魂燃烧！卡牌被消耗：" << card->getName();

        // 3. 通知 UI 播放燃烧动画并销毁那个 CardItem
        emit cardExhausted(card);

    }
}

void CardManager::removeCardPermanently(Card* card) {
    if (!card) return;
    m_drawPile.removeOne(card);
    m_hand.removeOne(card);
    m_discardPile.removeOne(card);
    m_exhaustPile.removeOne(card);
    m_masterDeck.removeOne(card);
    emit pileCountsChanged(m_drawPile.size(), m_discardPile.size(), m_exhaustPile.size());
}

void CardManager::upgradeRandomCards(int count) {
    QList<Card*> pool = getUpgradableCards();
    if (pool.isEmpty()) return;

    int actualCount = std::min(count, (int)pool.size());
    for (int i = 0; i < actualCount; ++i) {
        int idx = QRandomGenerator::global()->bounded(pool.size());
        pool[idx]->upgrade();
        pool.removeAt(idx);
    }
}

QList<Card*> CardManager::getUpgradableCards() const {
    QList<Card*> pool;
    // 检查所有可能存放卡牌的容器
    QList<const QList<Card*>*> piles = { &m_drawPile, &m_hand, &m_discardPile, &m_exhaustPile };
    for (const auto* pile : piles) {
        for (Card* c : *pile) {
            if (c && !c->isUpgraded() && c->getType() != CardType::Status && c->getType() != CardType::Curse) {
                // 防止重复添加（虽然逻辑上不应该在多个堆里）
                if (!pool.contains(c)) pool.append(c);
            }
        }
    }
    return pool;
}

void CardManager::addCardToDiscardPile(Card* newCard) {
    if (!newCard) return;
    m_discardPile.append(newCard);

    // 🔴 呼叫 UI 播放飞牌动画！
    emit cardInsertedToDiscard(newCard);

    emitPileCounts(); // 更新数字
}

// ========================================================
// 🧠 CardManager.cpp
// ========================================================
Card* CardManager::popTopDrawPile() {
    // 1. 如果抽牌堆空了，赶紧洗牌！
    if (m_drawPile.isEmpty()) {
        if (m_discardPile.isEmpty()) {
            qDebug() << "[CardManager] 抽牌堆和弃牌堆都空了！无法强抽顶牌！";
            return nullptr;
        }

        qDebug() << "[CardManager] 抽牌堆空了，正在洗弃牌堆...";
        // 🔴 借用你之前写好的洗牌逻辑（假设叫 shuffleDiscardToDraw() 之类的）
        // 如果你没有单独的函数，可以直接把这三行写在这里：
        m_drawPile.append(m_discardPile);
        m_discardPile.clear();
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_drawPile.begin(), m_drawPile.end(), g);

        // 顺便发射洗牌动画信号
        emit deckShuffled();
    }

    // 2. 如果洗完牌后还不为空，强行把第一张牌（顶牌）拿出来并返回！
    if (!m_drawPile.isEmpty()) {
        Card* topCard = m_drawPile.takeFirst();

        // 发射计数更新信号，让 UI 上的数字变少
        emit pileCountsChanged(m_drawPile.size(), m_discardPile.size(), m_exhaustPile.size());

        return topCard;
    }

    return nullptr;
}

void CardManager::forceMoveToDiscard(Card* card) {
    if (!card) return;

    // 不检查 m_hand，直接放入弃牌堆
    m_discardPile.append(card);

    // 如果它碰巧在手牌里（比如你是手动打出的），顺便移除一下
    m_hand.removeOne(card);

    emit cardDiscarded(card);
    emitPileCounts();
}

void CardManager::forceMoveToExhaust(Card* card) {
    if (!card) return;

    m_exhaustPile.append(card);
    m_hand.removeOne(card);

    emit cardExhausted(card); // 发出信号，确保黑暗之拥等效果生效
    emitPileCounts();
}

void CardManager::refreshHandDynamicText() {
    // 💡 这里咱们直接发射大喇叭信号！
    // 因为 Qt 的信号槽极其强大，只要在 UI 界面（比如 BattleView 或者 HandLayoutManager 里）
    // 监听到这个信号，就让所有的 CardItem 重新去读取一遍伤害文本！

    qDebug() << "[CardManager] 📢 遗物/状态发生改变！通知手牌区全体重绘动态文本喵！";
    emit handTextNeedsUpdate();
}