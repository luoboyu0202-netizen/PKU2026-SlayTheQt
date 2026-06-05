#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QMap>
#include <QPushButton> // 👈 引入按钮头文件，因为我们要保存按钮指针
#include <QPropertyAnimation> // 👈 引入属性动画头文件
#include <QEvent>             // 👈 引入事件处理头文件
#include "api/GameEnums.h"

// 📍 地圖節點數據結構
struct MapNode {
    int id;
    int layer;
    int position;
    NodeType type; // 🔴 升級為枚舉類型！不再使用 QString
    QList<int> nextNodes;
    int uiX;
    int uiY;
};

class MapManager : public QWidget {
    Q_OBJECT
public:
    explicit MapManager(QWidget *parent = nullptr);

    void generateMapNodes();

    void resetMap(); // 🔴 新增：重置大地图进度

protected:
    void paintEvent(QPaintEvent *event) override;

    // 🔴 新增：事件过滤器，用于捕捉鼠标悬停和离开的瞬间
    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    // 🔴 核心升级一：参数从 enemyId 换成了整个 MapNode 节点！
    // 这样当战斗胜利后，大地图才知道要把哪个节点打上叉叉标记
    void triggerNode(const MapNode& clickedNode);

signals:
    void nodeClicked(const MapNode& node);

public:
    QMap<int, QList<MapNode>> m_mapData;

    // ==========================================
    // 🔴 核心升级二：地图状态管理“记忆系统”
    // ==========================================
    QMap<int, QPushButton*> m_nodeButtons; // 记录所有按钮的指针，方便随时控制它们亮灭
    QList<int> m_visitedNodes;             // 记录玩家已经打赢踩过的节点 ID
    int m_currentLayer = -1;               // 玩家当前所在的层数 (-1 代表还没出发)
    int m_currentNodeId = -1;              // 玩家当前踩着的节点 ID

    // 🔴 新增专属函数：根据当前状态，刷新全图所有按钮的“能否点击”状态
    void refreshNodeStates();
};

#endif // MAPMANAGER_H