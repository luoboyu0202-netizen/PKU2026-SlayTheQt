#pragma once
#include <QObject>
#include <QString>
#include "StatusManager.h"

class Fighter : public QObject {
    Q_OBJECT

public:
    explicit Fighter(const QString& name, int maxHp, QObject* parent = nullptr);
    virtual ~Fighter() = default;

    // 暴露出获取状态管家的指针，方便外部（卡牌、遗物）去查阅或添加状态
    StatusManager* getStatusManager() const { return m_statusManager; }

    // Getter
    QString getName() const { return m_name; }
    int getHp() const { return m_hp; }
    int getMaxHp() const { return m_maxHp; }
    int getBlock() const { return m_block; }
    bool isDead() const { return m_isDead; }

    // 核心战斗数据操作接口
    virtual void takeDamage(int amount);
    virtual void addBlock(int amount);
    virtual void loseBlock(); // 回合开始时护甲清零
    virtual void heal(int amount);
    virtual void die();

    virtual void setHp(int hp);
    virtual void setMaxHp(int maxHp); // 新增：设置最大生命值接口

signals:
    // 数据驱动 UI：任何数值变化都必须通过这些信号通知表现层
    void hpChanged(int currentHp, int maxHp);
    void blockChanged(int currentBlock);
    void died(Fighter* entity);

protected:
    StatusManager* m_statusManager; // 🔴【新组件】：随身携带的状态背包！
    QString m_name;
    int m_hp;
    int m_maxHp;
    int m_block;
    bool m_isDead;
};
