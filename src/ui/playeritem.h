#pragma once
#include <QGraphicsObject>
#include <QPixmap> // 🔴【新增】：用于加载图片资源
#include <QMap>
#include <QList>
#include "../entities/Player.h"       // 引入主角逻辑类
#include "StatusIconItem.h"          // 引入精美状态图标

// 前置声明图片类，防止头文件循环包含
class QPixmap;

class PlayerItem : public QGraphicsObject {
    Q_OBJECT
    // 🔴 为了能用 QPropertyAnimation 做出受击震动效果，必须注册 x 属性
    Q_PROPERTY(qreal x READ x WRITE setX)

public:
    explicit PlayerItem(Player* logicPlayer, QGraphicsItem* parent = nullptr);
    virtual ~PlayerItem() = default;

    // QGraphicsItem 的核心两大重写接口
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    // 自动重排版工人
    void layoutStatusIcons();

    Player* m_logicPlayer; // 绑定的逻辑肉体

    // UI 本地缓存的数值
    int m_hp;
    int m_maxHp;
    int m_block;

    // 🔴【核心状态栏排版管家】
    QList<StatusType> m_activeStatusList;
    QMap<StatusType, StatusIconItem*> m_statusIcons;

    // 🔴【新增】：存储主角的图片肉体
    QPixmap m_playerPixmap;
};
