#pragma once
#include <QObject>
#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include "CardManager.h"
#include "relics/RelicManager.h" // 请根据你的实际路径调整一下喵
#include <functional>
#include <QDebug>

class RelicManager;

class BattleEngine : public QObject {
    Q_OBJECT

public:

    static BattleEngine* getInstance() { return s_instance; }

    explicit BattleEngine(Player* player, QList<Enemy*> enemies, CardManager* cardManager, RelicManager* relicManager, QObject* parent = nullptr);

    // 🔴 1. 新增接口：设置和获取当前战斗的背景图片地址
    void setBackgroundPath(const QString& path) { m_backgroundPath = path; }
    QString getBackgroundPath() const { return m_backgroundPath; }

    // 🔴 标记当前是否是战斗的第一个回合
    bool m_isFirstTurn = true;

    // ========================================================
    // 🔴【时停魔法 2.0】：支持任意数量、任意提示语的终极接口！
    // ========================================================
    bool isSelectingHandCard() const { return m_isSelectingHandCard; }
    int getSelectionLimit() const { return m_selectionLimit; } // 允许选几张
    // 发起请求：告诉引擎要选几张，屏幕上显式什么字，以及选完后的回调！
    void requestHandSelection(int count, const QString& promptText, std::function<void(QList<Card*>)> callback) {
        m_isSelectingHandCard = true;
        m_selectionLimit = count;
        m_selectionPrompt = promptText;
        m_selectionCallback = callback;
        m_selectedCards.clear(); // 清空上次的残留

        // 🔴 发射结界展开信号！通知 View 层立刻铺上黑幕和文字！
        emit selectionModeStarted(promptText);
    }

    // UI 玩家点了一张牌后，调用这里上交答案！
    // UI 玩家点了一张牌后，调用这里上交答案！
    void submitHandSelection(Card* selectedCard) {
        if (!m_isSelectingHandCard) return;

        // 1. 把玩家选中的这张牌装进暂存列表里喵！
        m_selectedCards.append(selectedCard);

        // 2. 检查是不是已经选够了规定的数量？
        if (m_selectedCards.size() >= m_selectionLimit) {
            m_isSelectingHandCard = false; // 选够了！解除时停！

            if (m_selectionCallback) {
                // 🔴【核心修正 2】：把装满卡牌的列表，整个喂给回调函数！
                m_selectionCallback(m_selectedCards);
                m_selectionCallback = nullptr;     // 清空记忆
            }

            // 如果你有解除 UI 黑幕的信号，也可以在这里发射，比如：
            // emit selectionModeEnded();
        }
    }

    // 流程控制
    void startBattle();
    void startPlayerTurn();
    void endPlayerTurn();
    void processEnemyTurn();

    // 🔴【新接口】：允许外部查询当前勾选了哪些牌，以及是否选够了
    const QList<Card*>& getSelectedCards() const { return m_selectedCards; }

    // 🔴【新接口】：勾选或取消勾选一张牌的开关
    void toggleCardSelection(Card* card);

    // 🔴【新接口】：点击确认键时呼叫这里，终结时停，执行契约！
    void confirmHandSelection();

    // 交互行为
    bool playCard(Card* card, Fighter* target); // 返回 false 表示打出失败（如费用不足）

    // ========================================================
    // 🟢【新增核心】：对外暴露的公共访问器 (Getters)
    // 专门为 BattleView 舞台管家准备的万能钥匙喵！
    // ========================================================
    Player* getPlayer() const { return m_player; }
    // 🔴【接口升级】：不再返回单个怪物，而是返回一整个怪物列表的引用
    const QList<Enemy*>& getEnemies() const { return m_enemies; }
    CardManager* getCardManager() const { return m_cardManager; }
    RelicManager* m_relicManager; // 🔴【存入大脑皮层】

    int modifyIncomingDamage(int baseDamage) {
        if (m_relicManager) {
            // 顺藤摸瓜，调用 RelicManager 内部的逻辑
            return m_relicManager->modifyIncomingDamage(baseDamage);
        }
        return baseDamage;
    }

    int calculateFinalDamage(Fighter* attacker, Fighter* defender, int baseDamage);

    // 🔴 接口 1：触发“打出牌库顶卡牌”的动作
    void triggerPlayTopCard(bool exhaustIt);

    // 🔴 接口 2：UI 动画播完后，回调执行真正的伤害！
    void executeRevealedCard(Card* card, bool exhaustIt);

    // 🟢 辅助工具：帮自动打出的卡牌随便找个受害者
    Fighter* getRandomEnemyTarget();

    // 供卡牌查询的接口
    bool isAutoPlayingCard() const { return m_isAutoPlayingCard; }

    // 专门用来通知全场“某张牌刚刚结算完毕”
    void triggerCardPlayedHooks(Card* playedCard);

    int calculateSnapshotDamage(Fighter* source, Fighter* target, int baseDamage);

signals:
    void turnStarted(bool isPlayerTurn);
    void battleEnded(bool isVictory);
    void enemyIntentUpdated(Enemy* enemy, Intent intent);
    void enemySummoned(Enemy* newEnemy);

    // 🔴 新增结界信号
    void selectionModeStarted(const QString& promptText);
    void selectionModeEnded();
    // 🔴【新信号】：通知 UI 确认按钮当前是否应该亮起（参数为 true 时亮起）
    void selectionValidityChanged(bool canConfirm);
    // 🔴 宣告：一张能力牌已生效，目标是 source
    void powerActivated(Card* card, Fighter* source);

    // 🔴 告诉 UI：有牌被强行拉出来了！快播飞到屏幕中央的动画！
    void topCardRevealed(Card* card, bool exhaustIt);

    void enemyActing(Enemy* enemy);

public slots:
    void refreshEnemyIntent();

private:
    static BattleEngine* s_instance;

    Player* m_player;
    QList<Enemy*> m_enemies;
    CardManager* m_cardManager;
    QString m_backgroundPath;

    // 🔴 状态机记忆变量
    bool m_isSelectingHandCard = false;
    std::function<void(QList<Card*>)> m_selectionCallback;
    int m_selectionLimit = 1;
    QString m_selectionPrompt;
    QList<Card*> m_selectedCards; // 记录当前已经勾选了哪些牌

    // 🔴 核心状态：当前是否处于“自动打牌（非玩家手动出牌）”的托管状态？
    bool m_isAutoPlayingCard = false;

    bool m_isExecutingRevealed = false;

    // ========================================================
    // ⏳ 战斗演出升级：异步结算系统
    // ========================================================
    void processNextEnemyAction(int index); // 核心：递归执行动作的齿轮
    QList<Enemy*> m_pendingSpawns;          // 核心：把临时等候室变成全局的，防止异步传递时丢失！
};