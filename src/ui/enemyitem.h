#pragma once
#include <QGraphicsObject>
#include "../entities/Enemy.h"
#include "StatusIconItem.h"

class BattleEngine;

class EnemyItem : public QGraphicsObject {
    Q_OBJECT
    // ========================================================
    // 🔴【核心魔法】：注册一个可以被动画系统连续修改的浮动属性！
    // ========================================================
    Q_PROPERTY(qreal intentFloatOffset READ intentFloatOffset WRITE setIntentFloatOffset)

public:
    EnemyItem(Enemy* logicEnemy, BattleEngine* engine, int spriteYOffset = 0, QGraphicsItem* parent = nullptr);
    virtual ~EnemyItem() = default;

    // 获取与之绑定的底层数据指针，日后用于卡牌碰撞检测
    Enemy* getLogicEnemy() const { return m_logicEnemy; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    bool m_isTargeted; // 【新增】标记当前怪物是否被箭头指着

    // 🔴 新增的读写接口：
    qreal intentFloatOffset() const { return m_intentFloatOffset; }
    void setIntentFloatOffset(qreal offset) {
        if (m_intentFloatOffset != offset) {
            m_intentFloatOffset = offset;
            update(); // 🟢 极其重要：数值只要一变，立刻触发重绘（paint）！
        }
    }

protected:
    // 🔴 声明鼠标进入和离开的事件
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override; // 新增！

private:
    Enemy* m_logicEnemy;
    BattleEngine* m_engine; // 🔴 2. 核心：在私有变量里给大脑留个位置！

    // UI 内部缓存的显示数值
    int m_hp;
    int m_maxHp;
    int m_block;
    IntentType m_intentType;
    int m_intentValue;

    // 🔴 核心排版工具：
    // 列表负责“记录先后顺序”！
    QList<StatusType> m_activeStatusList;

    // 字典负责“保管肉体指针”，方便我们快速找到它去更新层数或者删掉它！
    QMap<StatusType, StatusIconItem*> m_statusIcons;

    // 自动重排版函数
    void layoutStatusIcons();

    StatusType m_statusType;
    int m_statusValue;

    QString m_tooltipText; // 🔴 用来把拼好的提示文字存起来！
    QPixmap m_enemyPixmap;

    int m_spriteYOffset;

    // 🔴 存储浮动偏移量的变量
    qreal m_intentFloatOffset = 0.0;
    QPixmap m_currentIntentIcon; // 预加载好的当前意图图标
};
