#include "BattleEngine.h"
#include <QDebug>
#include <QTimer> // 🔴【新增】引入定时器
#include <entities/StatusManager.h>
#include "cards/SlimedCard.h"
#include "cards/BurnCard.h"
#include "EnemyFactory.h"
#include "CardManager.h"
#include <StatusManager.h>
#include <qrandom.h>
#include <QRandomGenerator> // 🔴 记得在头部引入现代随机数发生器
#include <QHash> // 🔴 记得在 EnemyItem.cpp 顶部加上这个！
#include <algorithm> // 🔴 确保文件顶部有这个头文件喵！

// ========================================================
// 🔴【分配物理空间】：必须写在所有函数的外面（通常在 include 之后）！
// ========================================================
BattleEngine* BattleEngine::s_instance = nullptr;

// 🔴 1. 注意看参数列表：Enemy* enemy 变成了 QList<Enemy*> enemies！
// 还要记得在初始化列表里把 m_enemies(enemies) 赋好值喵！
BattleEngine::BattleEngine(Player* player, QList<Enemy*> enemies, CardManager* cardManager, RelicManager* relicManager, QObject* parent)
    : QObject(parent), m_player(player), m_cardManager(cardManager), m_relicManager(relicManager), m_enemies(enemies) {

    s_instance = this;

    // ========================================================
    // 👁️【时序钩子：卡牌抽取劫持 (用于异蛇之眼的混乱效果)】
    // ========================================================
    connect(m_cardManager, &CardManager::cardDrawn, this, [this](Card* card) {
        if (!card) return;

        // 雷达扫描：玩家当前是不是处于“混乱”状态？
        if (m_player->getStatusManager()->getStatus(StatusType::Confusion) > 0) {

            // 🎲 命运的轮盘转动：0 到 3 的随机数！
            int randomCost = QRandomGenerator::global()->bounded(4);

            card->setCost(randomCost);

            // 💡 如果你的卡牌有 m_isCostModified 属性，别忘了设为 true，这样 UI 可以把字变色！
            card->setCostModified(true);

            qDebug() << "[Engine] 👁️ 混乱生效！被抽到的卡牌" << card->getName() << "费用突变为了" << randomCost << "！";
        }
    });
    // ========================================================
    // 🔴 2. 【核心纠错】：遍历大军，给每只怪物都挂上监听器！
    // ========================================================
    for (Enemy* enemy : m_enemies) {
        if (!enemy) continue; // 安全第一喵

        // 1. 当这只怪物自己进入下一回合，切换意图时，立刻刷新！
        connect(enemy, &Enemy::intentChanged, this, &BattleEngine::refreshEnemyIntent);

        // 2. 当这只怪物身上的状态（力量、虚弱等）发生任何改变时，瞬间核算刷新！
        connect(enemy->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType, int){
            refreshEnemyIntent();
        });
    }

    // ========================================================
    // 🟢 3. 玩家依然只有一个，直接在循环外面绑好就行！
    // ========================================================
    // 当玩家身上的状态（易伤等）发生改变时，怪物打人会更疼，所以也要瞬间核算刷新！
    connect(m_player->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType, int){
        refreshEnemyIntent();
    });

    // ========================================================
    // 🌌【时序钩子：卡牌消耗事件（On Card Exhausted）】
    // ========================================================
    connect(m_cardManager, &CardManager::cardExhausted, this, [this](Card* exhaustedCard) {
        Q_UNUSED(exhaustedCard);

        // 只要有牌被烧了，立刻去查主角身上有没有【黑暗之拥】！
        int darkEmbraceAmount = m_player->getStatusManager()->getStatus(StatusType::DarkEmbrace);
        if (darkEmbraceAmount > 0) {

            qDebug() << "[Engine] 黑暗之拥触发！虚空传来了回响，强制抽" << darkEmbraceAmount << "张牌！";

            // 触发能力：有几层就抽几张牌！
            m_cardManager->drawCards(darkEmbraceAmount);
        }
    });
}

void BattleEngine::refreshEnemyIntent() {
    // ========================================================
    // 🔴【核心多怪大重构】：让雷达扫描整只怪物军队！
    // ========================================================
    for (Enemy* enemy : m_enemies) {
        // 1. 安全检查：如果怪物指针是空的，或者已经变成尸体了，直接跳过它喵！
        if (!enemy || enemy->isDead()) continue;

        // 2. 拿到这只怪物本回合的“基础原始意图”
        Intent intent = enemy->getCurrentIntent();

        // 3. 只要这只怪物的意图带“攻击”，统统扔进管道里，结合它身上的力量、或者玩家身上的易伤重算伤害！
        if (intent.type == IntentType::Attack ||
            intent.type == IntentType::AttackAndDebuff ||
            intent.type == IntentType::AttackAndDefend||
            intent.type ==IntentType::AttackAndInsertStatus) {

            // ⚠️ 核心微调：这里的 source 换成当前循环里的 enemy 喵！
            intent.value = StatusManager::calculateDamage(enemy, m_player, intent.value);
        }
        // 🔴 如果未来加上了“脆弱”对防御的减益，可以在这里让带防御的意图也重算护甲喵！
        else if (intent.type == IntentType::Defend || intent.type == IntentType::DefendAndBuff
                 || intent.type == IntentType::AttackAndDefend) {
            intent.value = StatusManager::calculateBlock(enemy, intent.value);
        }

        // 4. 🔴 极其重要：把计算好的完美意图包裹，连同“它是谁”的指针，一起发射给 UI 舞台！
        // 这样对应的 EnemyItem 就会触发我们在上一回合写好的“过滤器”机制喵！
        emit enemyIntentUpdated(enemy, intent);
    }
}

void BattleEngine::startBattle() {
    qDebug() << "[Engine] Battle Started!";
    m_isFirstTurn = true;

    // ========================================================
    // 🔴 战前动员诊断
    // ========================================================
    qDebug() << "[🔬 Diagnostic - Engine] 开始全场怪物战前初始化，当前怪物数量:" << m_enemies.size();
    for (Enemy* enemy : m_enemies) {
        if (enemy) {
            qDebug() << "[🔬 Diagnostic - Engine] 正在呼叫怪物入场钩子:" << enemy->getName();
            enemy->onBattleStart();
        } else {
            qDebug() << "[🔬 Diagnostic - Engine] 🚨 警告：发现空的怪物指针！";
        }
    }

    // 遗物触发...
    if (m_relicManager) {
        for (Relic* r : m_relicManager->getRelics()) {
            if (r) {
                r->onBattleStart(m_player);
                r->onBattleStart();
            }
        }
    }

    startPlayerTurn();
}

void BattleEngine::startPlayerTurn() {
    qDebug() << "[Engine] --- Player Turn Started ---";

    // ========================================================
    // 💡【核心修复：统一思考与亮牌！】
    // 所有怪物在玩家回合开始的这一刻，统一决定这回合干嘛，并亮出兵器！
    // ========================================================
    for (Enemy* enemy : m_enemies) {
        if (enemy && !enemy->isDead()) {
            enemy->rollNextIntent(); // 大脑开始运转！
        }
    }

    // 统一计算真实伤害，并广播给所有的 UI 舞台！
    refreshEnemyIntent();

    // ========================================================
    // 🛡️【壁垒与首回合判定】：
    // 如果不是第一回合，并且身上没有壁垒状态，才会清空上一回合的护甲！
    // ========================================================
    if (!m_isFirstTurn && m_player->getStatusManager()->getStatus(StatusType::Barricade) <= 0) {
        m_player->loseBlock();
    } else if (m_isFirstTurn) {
        qDebug() << "[Engine] 第一回合开始！完美保留锚等遗物开局赋予的初始护甲！";
    } else {
        qDebug() << "[Engine] 壁垒生效！上一回合残存的护甲被完美保留了下来喵！";
    }

    // ========================================================
    // 🍦【冰淇淋结界】：能量不再于回合结束时清空！
    // ========================================================
    if (m_relicManager && m_relicManager->hasRelic("relic_ice_cream")) {
        // 🔴 触发冰淇淋：不覆盖重置！
        // 保留当前没用完的能量，并且叠加这一回合本该给的基础能量（MaxEnergy）！
        qDebug() << "[Engine] 🍦 冰淇淋发威！保留上回合剩余能量，追加" << m_player->getMaxEnergy() << "点基础能量喵！";

        m_player->addEnergy(m_player->getMaxEnergy());

        // （可选）触发全局遗物发光动画，你可以根据需要补充遗物发光信号
    } else {
        // 🔴 没有冰淇淋：老老实实执行原版逻辑，清空旧能量，重置回满！
        m_player->resetEnergy();
    }

    // ========================================================
    // 🔥【时序钩子：回合开始（Start of Turn）】
    // ========================================================

    // 🔴 救命补丁 1：大声告诉所有遗物，新回合开始了！
    // 没有这段广播，手里剑不会清零，开心小花不会计数！
    if (m_relicManager) {
        for (Relic* r : m_relicManager->getRelics()) {
            if (r) {
                r->onTurnStart(); // 🥷 手里剑会在这里把计数器清 0，🌻 小花会在这里计数！
            }
        }
    }

    // ========================================================
    // 🔥【时序钩子：回合开始（Start of Turn）】
    // ========================================================
    int fireSourceAmount = m_player->getStatusManager()->getStatus(StatusType::FireSource);
    if (fireSourceAmount > 0) {
        // 触发能力！增加额外能量！
        m_player->addEnergy(fireSourceAmount);
        qDebug() << "[Engine] 薪火之源生效！玩家额外获得了" << fireSourceAmount << "点能量！";
    }

    // 🔴 极其关键：读取玩家身上真正的抽牌数！（如果是异蛇，这里就是 7）
    int drawCount = m_player->getBaseDrawCount();
    qDebug() << "[Engine] 本回合准备抽取" << drawCount << "张牌...";

    m_cardManager->drawCards(drawCount);

    emit turnStarted(true);

    // ========================================================
    // 🔴【核心修复】：打破时间循环！
    // 告诉系统：第一回合的开局流程已经走完了，以后都是常规回合了！
    // ========================================================
    m_isFirstTurn = false; // 👈 补上这句！
}

bool BattleEngine::playCard(Card* card, Fighter* target) {
    if (!card || m_player->isDead()) return false;

    // ==========================================================
    // 🛡️【喵军师的绝对防御结界】：防连点、防幽灵双击！
    // 只要这牌不在你手里，哪怕天王老子发信号来，大脑也绝对不执行！
    // ==========================================================
    if (!m_cardManager->getHand().contains(card)) {
        qDebug() << "[Engine] 🛑 拦截幽灵信号！卡牌" << card->getName() << "已不在手牌，拒绝二次出牌喵！";
        return false;
    }

    // 1. 检查费用
    if (!m_player->useEnergy(card->getCost())) {
        qDebug() << "[Engine] Not enough energy to play:" << card->getName();
        return false;
    }

    qDebug() << "[Engine] Player played:" << card->getName();

    // ==========================================================
    // 🔴【遗物系统】：通知管家卡牌打出了！
    // ==========================================================

    // ==========================================================
    // 🎬【极致果汁感：主角突刺演出！】
    // 只要打出的是攻击牌，立刻命令主角往前扑击！
    // ==========================================================
    if (card->getType() == CardType::Attack) {
        emit m_player->animationAction();
    }

    // ==========================================================
    // 🔴【卡牌结算】：执行卡牌效果！
    // ==========================================================
    card->play(m_player, target, m_relicManager);

    if (m_relicManager) {
        m_relicManager->onCardPlayed(card);
    }

    // ==========================================================
    // 📢 【新增：全局 UI 刷新通知！】
    // 刚打完一张牌，遗物层数可能变了(钢笔尖到9层了)，力量可能变了。
    // 立刻命令大管家：刷新手牌区的所有动态文本！
    // ==========================================================

    if (m_cardManager) {
        m_cardManager->refreshHandDynamicText(); // 🔴 完美调用！
    }

    // ========================================================
    // 🔴【终极分流闸门】：合并重复代码，干净利落！
    // ========================================================
    if (card->getType() == CardType::Power) {
        // 1. 能力牌：化作光芒融入主角体内，彻底脱离本场牌库循环！
        qDebug() << "[Engine] 能力牌打出，进入能力虚空区！";
        m_cardManager->moveToPowerZone(card);

        // 🔴 发射！让 UI 知道主角要进化了！（直接合并进这个大括号里！）
        emit powerActivated(card, m_player);
    }
    else if (card->isExhaustOnUse()) {
        // 2. 消耗牌：进入墓地
        m_cardManager->moveToExhaust(card);
    }
    else {
        // 3. 普通牌：老老实实去弃牌堆
        m_cardManager->moveToDiscard(card);
    }

    // 🗑️ 注意：这里原本那个多余的 if (card->getType() == CardType::Power) 已经被彻底删除了！

    // ==========================================================
    // 🔴 4. 【多怪胜利判定】：全员阵亡才算赢！
    // ==========================================================
    bool allEnemiesDead = true;
    for (Enemy* e : m_enemies) {
        if (e && !e->isDead()) {
            allEnemiesDead = false; // 只要有一只还在喘气，战斗就没结束喵！
            break;
        }
    }

    if (allEnemiesDead) {
        qDebug() << "[Engine] Victory! All enemies defeated!";
        // ==========================================================
        // 🔴【新增】：战斗胜利！呼叫所有遗物触发战后效果 (例如燃烧之血回血)
        // ==========================================================
        if (m_relicManager) {
            qDebug() << "[Engine] 呼叫所有遗物触发战后效果...";
            for (Relic* r : m_relicManager->getRelics()) {
                if (r) r->onBattleEnd(m_player);
            }
        }

        QTimer::singleShot(1200, this, [this]() {
            emit battleEnded(true);
        });
    }

    return true;
}

void BattleEngine::endPlayerTurn() {
    qDebug() << "[Engine] --- Player Turn Ended ---";
    m_cardManager->endTurnProcess(); // 触发丢弃手牌，此时弃牌堆数字在 UI 上会立刻变大！

    // 🔴【核心修正】：延迟 400 毫秒再让怪物行动，并先检查是不是全死了
    QTimer::singleShot(400, this, [this]() {

        // 扫视战场，看看还有没有活着的怪物
        bool hasAliveEnemy = false;
        for (Enemy* e : m_enemies) {
            if (e && !e->isDead()) {
                hasAliveEnemy = true;
                break;
            }
        }

        // 如果还有活着的，就开启怪物的群殴回合！
        if (hasAliveEnemy) {
            processEnemyTurn();
        }
    });

    // ========================================================
    // 🛡️【时序钩子：回合结束（End of Turn）】
    // ========================================================
    // 检查玩家身上是否有“金属化”状态
    int metallicizeAmount = m_player->getStatusManager()->getStatus(StatusType::Metallicize);
    if (metallicizeAmount > 0) {
        // 触发能力！有几层就加多少甲！
        m_player->addBlock(metallicizeAmount);
        qDebug() << "[Engine] 金属化生效！玩家凭空获得了" << metallicizeAmount << "点格挡！";

        // 💡 果汁感预留：你甚至可以在这里 emit 一个信号，让 UI 给主角播放一个白光特效喵！
    }

    if (m_relicManager) {
        for (Relic* r : m_relicManager->getRelics()) {
            if (r) {
                r->onTurnEnd(); // 🛡️ 奥利哈钢此时会进行无甲检测！
            }
        }
    }

}

// ========================================================
// 🎬 引擎发令枪：怪物回合正式开始！
// ========================================================
void BattleEngine::processEnemyTurn() {
    // 1. 清空上一回合可能残留的等候室
    m_pendingSpawns.clear();

    // ========================================================
    // 📏【核心机制：整队！从左到右依次报数！】
    // 根据怪物的 SlotIndex (0=最左, 3=最右) 进行升序重排。
    // 这样接下来的异步齿轮，就会绝对遵循视觉顺序依次执行！
    // ========================================================
    std::sort(m_enemies.begin(), m_enemies.end(), [](Enemy* a, Enemy* b) {
        // 防止空指针崩溃的兜底保护
        if (!a || !b) return false;
        return a->getSlotIndex() < b->getSlotIndex();
    });

    // 2. 启动异步齿轮，从最左边的怪物开始结算！
    processNextEnemyAction(0);
}

// ========================================================
// ⏳ 异步齿轮：一个接一个地结算怪物动作！
// ========================================================
void BattleEngine::processNextEnemyAction(int index) {
    // 🛑 递归出口：如果所有老怪物都行动完毕了！
    if (index >= m_enemies.size()) {
        for (Enemy* spawn : m_pendingSpawns) {
            m_enemies.append(spawn);
            spawn->onBattleStart();
            // ❌ 删掉 spawn->rollNextIntent(); 让新兵也脑子空空地等待！
        }
        m_pendingSpawns.clear();

        // ❌ 删掉 refreshEnemyIntent();！绝对不能在这里亮牌！

        m_player->getStatusManager()->tickEndOfTurnStatuses();
        QTimer::singleShot(1, this, [this]() {
            startPlayerTurn();
        });
        return;
    }

    // ========================================================
    // 🎯 提取当前这只正在行动的怪物
    // ========================================================
    Enemy* enemy = m_enemies[index];

    // 如果这只怪物已经死了或者指针为空，直接跳过它，立刻执行下一个！
    if (!enemy || enemy->isDead()) {
        processNextEnemyAction(index + 1);
        return;
    }

    emit enemyActing(enemy); // 🔴 解除封印！

    // 1. 行动前：清空这只怪物上一回合的格挡
    enemy->loseBlock();

    // 2. 提取当前意图
    Intent currentIntent = enemy->getCurrentIntent();

    // ========================================================
    // 🛠️ 3. 意图结算：根据 currentIntent 执行动作
    // ========================================================
    if (currentIntent.type == IntentType::Attack) {
        for (int i = 0; i < currentIntent.multiHitCount; ++i) {
            if (m_player->isDead()) break;
            int finalDmg = StatusManager::calculateDamage(enemy, m_player, currentIntent.value);
            qDebug() << "[Engine] 怪物多段重拳 第" << i+1 << "击，伤害:" << finalDmg;
            m_player->takeDamage(finalDmg);
        }
    }
    else if (currentIntent.type == IntentType::Defend) {
        int finalBlk = StatusManager::calculateBlock(enemy, currentIntent.value);
        enemy->addBlock(finalBlk);
    }
    else if (currentIntent.type == IntentType::Debuff) {
        qDebug() << "[Engine] Enemy applies Debuff to player. Layers:" << currentIntent.value;
        m_player->getStatusManager()->applyStatus(currentIntent.statusType, currentIntent.value);
    }
    else if (currentIntent.type == IntentType::Buff) {
        qDebug() << "[Engine] Enemy buffs itself. Layers:" << currentIntent.value;
        enemy->getStatusManager()->applyStatus(currentIntent.statusType, currentIntent.value);
    }
    else if (currentIntent.type == IntentType::AttackAndDebuff) {
        int finalDmg = StatusManager::calculateDamage(enemy, m_player, currentIntent.value);
        m_player->takeDamage(finalDmg);
        m_player->getStatusManager()->applyStatus(currentIntent.statusType, currentIntent.statusValue);
    }
    else if (currentIntent.type == IntentType::AttackAndDefend) {
        int finalDmg = StatusManager::calculateDamage(enemy, m_player, currentIntent.value);
        for (int i = 0; i < currentIntent.multiHitCount; ++i) {
            if (m_player->isDead()) break;
            m_player->takeDamage(finalDmg);
        }
        int finalBlk = StatusManager::calculateBlock(enemy, currentIntent.statusValue);
        enemy->addBlock(finalBlk);
    }
    else if (currentIntent.type == IntentType::DefendAndBuff) {
        int finalBlk = StatusManager::calculateBlock(enemy, currentIntent.value);
        enemy->addBlock(finalBlk);
        enemy->getStatusManager()->applyStatus(currentIntent.statusType, currentIntent.statusValue);
    }
    else if (currentIntent.type == IntentType::InsertStatus) {
        qDebug() << "[Engine]" << enemy->getName() << "向玩家牌库塞入了" << currentIntent.value << "张" << currentIntent.cardIdToInsert;
        if (currentIntent.cardIdToInsert == "card_slimed") {
            for (int i = 0; i < currentIntent.value; ++i) {
                Card* slimeCard = new SlimedCard(m_cardManager);
                m_cardManager->addCardToDiscardPile(slimeCard);
            }
            m_cardManager->emitPileCounts();
        }
    }
    // ==========================================================
    // ⚔️ 复合意图：攻击并塞入状态牌！
    // ==========================================================
    else if (currentIntent.type == IntentType::AttackAndInsertStatus) {

        // 1. 结算攻击伤害 (完美支持多段连击)
        int damage = StatusManager::calculateDamage(enemy, m_player, currentIntent.value);
        for (int i = 0; i < currentIntent.multiHitCount; ++i) {
            m_player->takeDamage(damage);
        }
        qDebug() << "[Engine]" << enemy->getName() << "对玩家造成了"
                 << currentIntent.multiHitCount << "次" << damage << "点伤害！";

        // 2. 完美复用你的塞牌逻辑！
        // 🔴 注意：这里读取的是 statusValue，代表要塞几张牌！
        int insertCount = currentIntent.statusValue;
        qDebug() << "[Engine]" << enemy->getName() << "向玩家牌库塞入了"
                 << insertCount << "张" << currentIntent.cardIdToInsert;

        if (currentIntent.cardIdToInsert == "card_slimed") {
            for (int i = 0; i < insertCount; ++i) {
                // 确保你的 SlimedCard 构造函数支持这样传参喵！
                Card* slimeCard = new SlimedCard(m_cardManager);
                m_cardManager->addCardToDiscardPile(slimeCard);
            }
        }
        else if (currentIntent.cardIdToInsert == "card_burn") {
            for (int i = 0; i < insertCount; ++i) {
                // 🟢 实例化我们刚刚写好的灼伤牌！
                Card* burnCard = new BurnCard(m_cardManager);
                m_cardManager->addCardToDiscardPile(burnCard);
            }
        }

        // 统一呼叫 UI 刷新牌堆数字
        m_cardManager->emitPileCounts();
    }
    else if (currentIntent.type == IntentType::Summon) {
        const int MAX_SLOTS = 4;
        QVector<bool> slotOccupied(MAX_SLOTS, false);

        // 🔴 扫描时，不仅要看战场上的老怪物，还要把等候室的新怪物也算上！否则会坑位重叠！
        for (Enemy* e : m_enemies) {
            if (e && !e->isDead() && e->getSlotIndex() >= 0 && e->getSlotIndex() < MAX_SLOTS) {
                slotOccupied[e->getSlotIndex()] = true;
            }
        }
        for (Enemy* e : m_pendingSpawns) {
            if (e && e->getSlotIndex() >= 0 && e->getSlotIndex() < MAX_SLOTS) {
                slotOccupied[e->getSlotIndex()] = true;
            }
        }

        int summonCount = currentIntent.value;
        qDebug() << "[Engine]" << enemy->getName() << "发大招啦！准备召唤" << summonCount << "只" << currentIntent.enemyIdToSummon;
        const int preferredOrder[4] = {1, 2, 3, 0};

        for (int k = 0; k < summonCount; ++k) {
            int targetFreeSlot = -1;
            for (int i = 0; i < MAX_SLOTS; ++i) {
                int checkSlot = preferredOrder[i];
                if (!slotOccupied[checkSlot]) {
                    targetFreeSlot = checkSlot;
                    break;
                }
            }

            if (targetFreeSlot != -1) {
                qDebug() << "[Engine] 遵循视觉平衡，成功在" << targetFreeSlot << "号位挤出了小怪喵！";
                Enemy* newSpawn = EnemyFactory::createEnemy(currentIntent.enemyIdToSummon);
                newSpawn->setSlotIndex(targetFreeSlot);

                // 🔴 存入全局的等候室，等待整个回合结束再入场！
                m_pendingSpawns.append(newSpawn);
                emit enemySummoned(newSpawn);
                slotOccupied[targetFreeSlot] = true;
            } else {
                qDebug() << "[Engine] 战场已满！召唤失败喵！";
                break;
            }
        }
    }
    else if (currentIntent.type == IntentType::Curse) {
        qDebug() << "[Engine]" << enemy->getName() << "正在释放可怕的诅咒！";
        if (currentIntent.statusType != StatusType::None) {
            m_player->getStatusManager()->applyStatus(currentIntent.statusType, currentIntent.value);
        }
        if (!currentIntent.cardIdToInsert.isEmpty()) {
            qDebug() << "[Engine] 诅咒生效：向玩家牌库塞入了" << currentIntent.value << "张" << currentIntent.cardIdToInsert;
        }
    }
    else if (currentIntent.type == IntentType::GroupBuff) {
        qDebug() << "[Engine]" << enemy->getName() << "发出了震耳欲聋的战吼，全体怪物获得强化！";
        for (Enemy* ally : m_enemies) {
            if (ally && !ally->isDead()) {
                ally->getStatusManager()->applyStatus(currentIntent.statusType, currentIntent.value);
            }
        }
        // 如果想让刚召唤还没入场的小弟也吃到 Buff，可以把 m_pendingSpawns 也遍历一遍喵！
    }
    else if (currentIntent.type == IntentType::GroupDefend) {
        qDebug() << "[Engine]" << enemy->getName() << "举起巨盾，保护了所有同伴！";
        for (Enemy* ally : m_enemies) {
            if (ally && !ally->isDead()) {
                int finalBlk = StatusManager::calculateBlock(enemy, currentIntent.value);
                ally->addBlock(finalBlk);
            }
        }
    }

    // ========================================================
    // 🔴 4. 死亡拦截：防鞭尸机制！
    // ========================================================
    if (m_player->isDead()) {
        qDebug() << "[Engine] Player died... playing death animation before Game Over.";

        // ==========================================================
        // ⏳ 魔法修复：给主角留足倒地变黑的时间（1.5 秒后），再宣布失败！
        // ==========================================================
        QTimer::singleShot(1500, this, [this]() {
            emit battleEnded(false);
        });
        return; // 主角阵亡，停止递归！
    }

    // ========================================================
    // 🧟‍♂️【修复意图诈尸】：怪物打完后，彻底清空大脑！
    // ========================================================
    // ❌ 删掉原来的 enemy->rollNextIntent();
    enemy->clearIntent(); // 逻辑层设为空，全局雷达扫到了也不会诈尸！
    emit enemyIntentUpdated(enemy, enemy->getCurrentIntent()); // 通知 UI 隐藏

    enemy->getStatusManager()->tickEndOfTurnStatuses();

    QTimer::singleShot(1200, this, [this, index]() {
        processNextEnemyAction(index + 1);
    });
}

void BattleEngine::toggleCardSelection(Card* card) {
    if (!card) return;

    if (m_selectedCards.contains(card)) {
        // 如果已经选过了，就拿掉（对应弹回原样）
        m_selectedCards.removeOne(card);
    } else {
        // 如果没选过，且还没达到上限，就加进去
        if (m_selectedCards.size() < m_selectionLimit) {
            m_selectedCards.append(card);
        }
    }

    // 🔴 检查是否选够了数量，广播给 UI 让确认键亮起或熄灭！
    emit selectionValidityChanged(m_selectedCards.size() == m_selectionLimit);
}

void BattleEngine::confirmHandSelection() {
    if (m_selectedCards.size() != m_selectionLimit) return; // 没选满不准确认！

    m_isSelectingHandCard = false; // 解除时停
    emit selectionModeEnded();     // 🔴 广播结界解除信号！让半透明黑幕和确认键退场！

    if (m_selectionCallback) {
        // 🔴 完美切开魔法胶囊，执行我们在【燃烧契约】里写好的烧牌抽牌逻辑！
        m_selectionCallback(m_selectedCards);
        m_selectionCallback = nullptr;
    }
}

// 假设我们在 BattleEngine 里或者封装一个专用的计算器
int BattleEngine::calculateFinalDamage(Fighter* attacker, Fighter* defender, int baseDamage) {
    int finalDamage = baseDamage;

    // 1. 提取双方的状态背包
    StatusManager* atkStatus = attacker->getStatusManager();
    StatusManager* defStatus = defender->getStatusManager();

    // 2. 攻击方增益：力量 (直接加算)
    if (atkStatus->getStatus(StatusType::Strength) > 0) {
        finalDamage += atkStatus->getStatus(StatusType::Strength);
    }

    // 3. 攻击方减益：虚弱 (造成伤害减少 25%)
    if (atkStatus->getStatus(StatusType::Weak) > 0) {
        finalDamage = static_cast<int>(finalDamage * 0.75);
    }

    // 4. 防御方减益：易伤 (受到伤害增加 50%)
    if (defStatus->getStatus(StatusType::Vulnerable) > 0) {
        finalDamage = static_cast<int>(finalDamage * 1.50);
    }

    return finalDamage;
}

void BattleEngine::triggerPlayTopCard(bool exhaustIt) {
    if (!m_cardManager) return;

    // 1. 呼叫管家：把顶牌交出来！
    Card* topCard = m_cardManager->popTopDrawPile();

    if (!topCard) {
        qDebug() << "[Engine] 牌库和弃牌堆都空了！地狱狂徒宣告失败喵！";
        return;
    }

    qDebug() << "[Engine] 强行抽出了顶部卡牌：" << topCard->getName();

    // 2. 🔴 关键点：发射信号，把控制权交给 UI！
    emit topCardRevealed(topCard, exhaustIt);
}

void BattleEngine::executeRevealedCard(Card* card, bool exhaustIt) {
    if (!card || m_player->isDead()) return;

    static int callCount = 0;
    callCount++;
    qDebug() << "[LOCK] 当前 executeRevealedCard 调用次数:" << callCount << " 卡牌:" << card->getName();

    if (m_isExecutingRevealed) {
        qDebug() << "🛑 [CRITICAL] 幽灵二次调用已拦截！";
        return;
    }

    // 1. 如果它是单体指向攻击牌（RequiresTarget == true），
    // 但系统自动打出时没有玩家的红线瞄准，我们必须帮它随机抓个倒霉蛋！
    Fighter* target = nullptr;
    if (card->requiresTarget()) {
        target = getRandomEnemyTarget();
        if (!target) {
            qDebug() << "[Engine] 没有活着的怪物可以作为目标，自动出牌取消！";
            return; // 怪物死光了，不打出了
        }
    }

    // ========================================================
    // 🔴【核心操作】：开启自动驾驶指示灯！
    // ========================================================
    m_isAutoPlayingCard = true;

    // 强行打出！此时卡牌内部的 play() 就能知道自己是被托管打出的了！
    card->play(m_player, target, m_relicManager);
    triggerCardPlayedHooks(card);

    // 🔴【核心操作】：打完后立刻关闭，恢复正常！
    m_isAutoPlayingCard = false;

    // 2. 🔴 无视费用，直接强制打出！
    qDebug() << "[Engine] 自动打出卡牌结算完毕喵！";

    // 3. 决定它的最终归宿（使用强行移动逻辑）
    if (card->getType() == CardType::Power) {
        // 能力牌的情况（通常需要进 PowerZone，这块逻辑如果也依赖 hand 检查，也需要改）
        m_cardManager->moveToPowerZone(card);
        emit powerActivated(card, m_player);
    }
    else if (exhaustIt || card->isExhaustOnUse()) {
        m_cardManager->forceMoveToExhaust(card); // 使用强制移动
    }
    else {
        m_cardManager->forceMoveToDiscard(card); // 使用强制移动
    }

    // 4. 🔴 别忘了做死人检查！(万一这张牌把全场怪秒了呢？)
    bool allEnemiesDead = true;
    for (Enemy* e : m_enemies) {
        if (e && !e->isDead()) {
            allEnemiesDead = false;
            break;
        }
    }
    if (allEnemiesDead) {
        qDebug() << "[Engine] Victory triggered by Auto-Play!";
        // ==========================================================
        // 🔴【新增】：自动打出触发胜利！也要呼叫遗物！
        // ==========================================================
        if (m_relicManager) {
            qDebug() << "[Engine] 呼叫所有遗物触发战后效果...";
            for (Relic* r : m_relicManager->getRelics()) {
                if (r) r->onBattleEnd(m_player);
            }
        }
        emit battleEnded(true);
    }
}

Fighter* BattleEngine::getRandomEnemyTarget() {
    QList<Enemy*> aliveEnemies;
    for (Enemy* e : m_enemies) {
        if (e && !e->isDead()) {
            aliveEnemies.append(e);
        }
    }

    if (aliveEnemies.isEmpty()) return nullptr;

    // 🔴 极其完美的现代随机挑选算法
    int randomIndex = QRandomGenerator::global()->bounded(aliveEnemies.size());
    return aliveEnemies[randomIndex];
}

void BattleEngine::triggerCardPlayedHooks(Card* playedCard) {
    if (!playedCard || !m_relicManager) return;

    // 广播给遗物
    for (Relic* r : m_relicManager->getRelics()) {
        if (r) r->onCardPlayed(playedCard);
    }

    // 以后这里还可以加上：广播给能力/状态（StatusManager）
    // 比如有的怪物有“你每打出一张牌它就加力量”的Buff，也全都在这里触发！
}

int BattleEngine::calculateSnapshotDamage(Fighter* source, Fighter* target, int baseDamage) {
    if (!source || !target) return baseDamage; // 🔴 嫌疑人 A 在此！

    int finalDamage = baseDamage;
    StatusManager* srcStatus = source->getStatusManager();

    // 1. 结算力量 (基础伤害 + 力量)
    if (srcStatus) {
        finalDamage += srcStatus->getStatus(StatusType::Strength);
        if (finalDamage < 0) finalDamage = 0; // 保底
    }

    // 2. 攻击方减益：虚弱 (-25% 向下取整)
    if (srcStatus && srcStatus->getStatus(StatusType::Weak) > 0) {
        finalDamage = static_cast<int>(finalDamage * 0.75);
    }

    // ========================================================
    // 🔴 3. 遗物介入！(哪怕是在手里预览没有目标，钢笔尖也能翻倍了！)
    // ========================================================
    if (m_relicManager) {
        for (Relic* r : m_relicManager->getRelics()) {
            finalDamage = r->modifyAttackDamage(finalDamage);
        }
    }

    // 4. 防御方减益：易伤 (+50% 向下取整)
    // 🟢 修正：走到这里，如果 target 为空就不算易伤；如果拖拽到了怪头上，且有易伤，才乘 1.5！
    if (target && target->getStatusManager()) {
        if (target->getStatusManager()->getStatus(StatusType::Vulnerable) > 0) {
            finalDamage = static_cast<int>(finalDamage * 1.50);
        }
    }

    return finalDamage > 0 ? finalDamage : 0;
}