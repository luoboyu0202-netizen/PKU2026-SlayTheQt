#include "MapManager.h"
#include <QRandomGenerator> // 引入 Qt 随机数生成器
#include <QDebug>
#include <QPushButton> // 👈 新增这一行，用来创建按钮组件
#include <QPainter> // 👈 新增引入画笔工具
#include <QPen>     // 👈 新增引入画笔样式（颜色、粗细）
#include "..\entities\cards\testcard.h"
// 如果需要用到全局存档，可以在这里引入 GlobalSaveData.h

MapManager::MapManager(QWidget *parent) : QWidget(parent) {
    // 强制把大地图的真实尺寸撑开（假设宽度1280，高度1500）
    // 只有内部画板足够高，外面的窗口才能生成上下滚动条！
    this->setMinimumSize(1280, 1500);
    // 界面初始化逻辑
    generateMapNodes();
}

void MapManager::generateMapNodes() {
    // TODO: 这里之后会用来绘制你的地图节点（比如商店、精英怪、篝火等）
    m_mapData.clear();
    const int TOTAL_LAYERS = 15; // 假设地图总共有 15 层
    int currentId = 1000;        // 节点 ID 从 1000 开始递增

    // ==========================================
    // 第一步：生成纯节点（无连线）与 第三步：坐标抖动
    // ==========================================
    for (int layer = 0; layer < TOTAL_LAYERS; ++layer) {
        // 每一层随机生成 2 到 4 个节点
        int nodeCount = (layer == TOTAL_LAYERS - 1) ? 1 : QRandomGenerator::global()->bounded(2, 5);
        QList<MapNode> layerNodes;

        for (int pos = 0; pos < nodeCount; ++pos) {
            MapNode node;
            node.id = currentId++;
            node.layer = layer;
            node.position = pos;

            // 根据层数权重分配节点类型
            if (layer == TOTAL_LAYERS - 1) {
                node.type = "Boss";
            } else if (layer == 0) {
                node.type = "Monster"; // 第一层保底普通怪
            } else if (layer == 7) {
                node.type = "Campfire"; // 第七层保底篝火休息
            } else {
                // 其他层随机分配
                int randVal = QRandomGenerator::global()->bounded(100);
                if (randVal < 45) node.type = "Monster";
                else if (randVal < 70) node.type = "Elite";
                else if (randVal < 85) node.type = "Shop";
                else node.type = "Campfire";
            }

            // ==========================================
            // 🔴 全新升级：动态居中与纵向拉伸算法
            // ==========================================
            int canvasWidth = 1280;   // 我们大地图的真实宽度
            int nodeSpacingX = 160;   // 节点之间的横向间距

            // 1. 计算当前这一层所有节点排在一起的“总宽度”
            int totalLayerWidth = (nodeCount - 1) * nodeSpacingX;

            // 2. 根据总宽度，计算出这一层最左边那个节点的起始 X 坐标，绝对完美居中！
            int startX = (canvasWidth - totalLayerWidth) / 2;

            // 3. 算出当前节点的准确 X 坐标
            int baseX = startX + pos * nodeSpacingX;

            // 4. 纵向拉伸：我们画板高度设了 1500，所以把起点往下移，间距拉大，让滚动更爽！
            // 从 Y = 1350 开始，每层往上爬 85 像素
            int baseY = 1350 - layer * 85;
            node.uiX = baseX + QRandomGenerator::global()->bounded(-20, 20); // X 轴抖动
            node.uiY = baseY + QRandomGenerator::global()->bounded(-15, 15); // Y 轴抖动

            layerNodes.append(node);
        }
        m_mapData.insert(layer, layerNodes);
    }

    // ==========================================
    // 第二步：交叉连线算法 (核心连通逻辑)
    // ==========================================
    for (int i = 0; i < TOTAL_LAYERS - 1; ++i) {
        QList<MapNode>& currentLayer = m_mapData[i];
        QList<MapNode>& nextLayer = m_mapData[i+1];

        // 1. 向上保底：确保下一层的每个节点都至少有一条线从当前层连过来
        for (int nextIdx = 0; nextIdx < nextLayer.size(); ++nextIdx) {
            // 找一个相对位置最接近的当前层节点
            int currIdx = (nextIdx * currentLayer.size()) / nextLayer.size();
            currentLayer[currIdx].nextNodes.append(nextLayer[nextIdx].id);
        }

        // 2. 向下保底：确保当前层的每个节点都至少连出一条线到下一层
        for (int currIdx = 0; currIdx < currentLayer.size(); ++currIdx) {
            if (currentLayer[currIdx].nextNodes.isEmpty()) {
                int nextIdx = (currIdx * nextLayer.size()) / currentLayer.size();
                currentLayer[currIdx].nextNodes.append(nextLayer[nextIdx].id);
            }
        }
    }

    qDebug() << "========================================";
    qDebug() << "大地图生成完毕！共生成层数：" << m_mapData.size();
    qDebug() << "========================================";
    // ==========================================
    // 第四步：渲染占位 UI（把隐形数据变成真正的按钮）
    // ==========================================
    // 🔴 在生成新地图前，清空旧的记忆
    m_nodeButtons.clear();
    m_visitedNodes.clear();
    m_currentLayer = -1;
    m_currentNodeId = -1;
    for (int layer = 0; layer < m_mapData.size(); ++layer) {
        for (const MapNode& node : m_mapData[layer]) {
            // 1. 创建一个按钮实体，并依附在当前的大地图窗口 (this) 上
            QPushButton* btn = new QPushButton(this);

            // 🔴 把生成好的按钮指针，用它的 id 当钥匙存进字典里！
            m_nodeButtons.insert(node.id, btn);

            // 2. 在按钮上写上节点类型和层数，方便你测试看图
            btn->setText(QString("层:%1\n%2").arg(layer).arg(node.type));

            // 3. 将按钮摆放到算法算好的坐标位置 (宽 65, 高 45)
            btn->setGeometry(node.uiX, node.uiY, 65, 45);

            // 4. 给不同类型的节点上点颜色，方便区分（基础样式）
            if (node.type == "Boss") {
                btn->setStyleSheet("background-color: #f44336; color: white; font-weight: bold;"); // 红色Boss
            } else if (node.type == "Campfire") {
                btn->setStyleSheet("background-color: #ff9800; color: white;"); // 橙色篝火
            } else if (node.type == "Elite") {
                btn->setStyleSheet("background-color: #9c27b0; color: white;"); // 紫色精英
            } else {
                btn->setStyleSheet("background-color: #607d8b; color: white;"); // 灰蓝色普通节点
            }

            // 🔴 注意这里的 connect 修改！现在我们要把整个 node 传过去！
            connect(btn, &QPushButton::clicked, this, [this, node]() {
                this->triggerBattle(node);
            });

            // 6. 让按钮现身！
            btn->show();
        }
    }
    // 🔴 在整个生成函数的最后一行，调用一次刷新函数，给起点层亮绿灯！
    refreshNodeStates();

}

void MapManager::triggerBattle(const MapNode& clickedNode) {

    // 1. 准备粮草：拼装 Context
    BattleContext context;
    context.currentHp = 75;
    context.maxHp = 80;
    context.gold = 120;
    context.maxEnergy = 3;
    // 🔴 加入你的闪电霹雳卡！
    // 注意：Card 派生类通常由 BattleEngine 管理生命周期，
    // 这里 new 出来的对象，之后会在 BattleLauncher 的逻辑中被移交给 CardManager
    context.currentDeck.append(new testcard());

    // 🔴 关键修改：通过节点类型来决定遭遇战的敌人 ID
    // 这样点击 Boss 节点就会触发 Boss 战，点击普通怪触发小怪战
    if (clickedNode.type == "Boss") context.enemySeedOrId = "Boss_Slime";
    else if (clickedNode.type == "Elite") context.enemySeedOrId = "Elite_Slime";
    else context.enemySeedOrId = "Slime_Squad";

    BattleLauncher* launcher = new BattleLauncher(this);

    // 2. 监听战报：绑定 Signal
    connect(launcher, &BattleLauncher::battleConcluded,
            this, [this, launcher, clickedNode](BattleResult result) {

                if (!result.isVictory) {
                    qDebug() << "主角阵亡，弹出 GameOver 界面!";
                } else {
                    qDebug() << "战斗胜利！";

                    // 3. 标记状态并刷新地图
                    m_currentLayer = clickedNode.layer;
                    m_currentNodeId = clickedNode.id;
                    m_visitedNodes.append(clickedNode.id);

                    refreshNodeStates(); // 重新渲染状态
                }

                launcher->deleteLater();
            });

    launcher->launch(context);
}

// ==========================================
// 绘制大地图底层的交叉连线
// ==========================================
void MapManager::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event); // 先让父类完成基础绘制

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，让线条更平滑

    // 设置画笔的样式：浅灰色，粗细为 3，线条样式为虚线 (DashLine)
    QPen pen(QColor(180, 180, 180), 3, Qt::DashLine);
    painter.setPen(pen);

    // 遍历地图数据，把每一层和下一层连起来
    // 注意：最后一层（Boss层）没有下一层了，所以循环到 m_mapData.size() - 1 即可
    for (int layer = 0; layer < m_mapData.size() - 1; ++layer) {
        const QList<MapNode>& currentLayerNodes = m_mapData[layer];
        const QList<MapNode>& nextLayerNodes = m_mapData[layer + 1];

        // 遍历当前层的所有节点
        for (const MapNode& currNode : currentLayerNodes) {
            // 计算当前按钮的中心点坐标（按钮宽 65，高 45，所以各加上一半）
            int startX = currNode.uiX + 32;
            int startY = currNode.uiY + 22;

            // 遍历该节点通向下一层的所有目标节点 ID
            for (int nextId : currNode.nextNodes) {

                // 去下一层寻找对应的目标节点
                for (const MapNode& targetNode : nextLayerNodes) {
                    if (targetNode.id == nextId) {
                        // 计算目标按钮的中心点坐标
                        int endX = targetNode.uiX + 32;
                        int endY = targetNode.uiY + 22;

                        // 画出连接两点的线！
                        painter.drawLine(startX, startY, endX, endY);
                        break; // 找到了就跳出内层循环，继续画下一条线
                    }
                }
            }
        }
    }
}
// ==========================================
// 判定裁判：刷新全图按钮状态与标记
// ==========================================
void MapManager::refreshNodeStates() {
    // 1. 先找出当前踩着的节点，看看它能通往哪些下一层节点？
    QList<int> availableNextNodeIds;
    if (m_currentLayer != -1 && m_currentNodeId != -1) {
        for (const MapNode& node : m_mapData[m_currentLayer]) {
            if (node.id == m_currentNodeId) {
                availableNextNodeIds = node.nextNodes; // 拿到了通行白名单！
                break;
            }
        }
    }

    // 2. 遍历全图所有按钮，挨个宣判命运
    for (int layer = 0; layer < m_mapData.size(); ++layer) {
        for (const MapNode& node : m_mapData[layer]) {
            QPushButton* btn = m_nodeButtons[node.id];

            // 命运 A：已经被踩过的节点（打叉标记！）
            if (m_visitedNodes.contains(node.id)) {
                btn->setText("❌");
                // 变灰变暗，加上红框，并且绝对不准再点！
                btn->setStyleSheet("background-color: #222222; color: #555555; border: 2px solid darkred;");
                btn->setEnabled(false);
                continue;
            }

            // 命运 B：未被踩过的节点，判断是否放行
            if (m_currentLayer == -1) {
                // 游戏刚开局：只有第 0 层放行
                btn->setEnabled(node.layer == 0);
            } else {
                // 游戏进行中：必须是下一层，且 ID 必须在通行白名单里才放行！
                bool isNextLayer = (node.layer == m_currentLayer + 1);
                bool isConnected = availableNextNodeIds.contains(node.id);
                btn->setEnabled(isNextLayer && isConnected);
            }
        }
    }
}