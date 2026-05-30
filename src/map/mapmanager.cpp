#include "MapManager.h"
#include <QRandomGenerator> // 引入 Qt 随机数生成器
#include <QDebug>
#include <QPushButton> // 👈 新增这一行，用来创建按钮组件
#include <QPainter> // 👈 新增引入画笔工具
#include <QPen>     // 👈 新增引入画笔样式（颜色、粗细）
#include "../logic/GlobalSaveData.h"
#include "../logic/CardFactory.h"
#include "../logic/RelicFactory.h"
// 如果需要用到全局存档，可以在这里引入 GlobalSaveData.h
#include <QPainterPath> // 👈 新增：用于绘制优雅的弯曲路径

MapManager::MapManager(QWidget *parent) : QWidget(parent) {
    // 强制把大地图的真实尺寸撑开（假设宽度1280，高度2200）
    // 只有内部画板足够高，外面的窗口才能生成上下滚动条！
    this->setMinimumSize(1280, 3000);
    // 界面初始化逻辑
    generateMapNodes();
}

void MapManager::generateMapNodes() {
    m_mapData.clear();
    const int TOTAL_LAYERS = 15;
    int currentId = 1000;

    // ==========================================
    // 第一步：生成纯节点与坐标抖动
    // ==========================================
    for (int layer = 0; layer < TOTAL_LAYERS; ++layer) {
        int nodeCount = (layer == TOTAL_LAYERS - 1) ? 1 : QRandomGenerator::global()->bounded(2, 5);
        QList<MapNode> layerNodes;

        for (int pos = 0; pos < nodeCount; ++pos) {
            MapNode node;
            node.id = currentId++;
            node.layer = layer;
            node.position = pos;

            if (layer == TOTAL_LAYERS - 1) {
                node.type = "Boss";
            } else if (layer == 0) {
                node.type = "Monster";
            } else if (layer == 7) {
                node.type = "Campfire";
            } else {
                int randVal = QRandomGenerator::global()->bounded(100);
                if (randVal < 45) node.type = "Monster";
                else if (randVal < 70) node.type = "Elite";
                else if (randVal < 85) {
                    // 将部分商店替换为宝箱或事件
                    int r = QRandomGenerator::global()->bounded(100);
                    if (r < 50) node.type = "Chest";
                    else node.type = "Event";
                }
                else node.type = "Campfire";
            }

            int canvasWidth = 1280;
            int nodeSpacingX = 160;
            int totalLayerWidth = (nodeCount - 1) * nodeSpacingX;
            int startX = (canvasWidth - totalLayerWidth) / 2 -40;

            // ==========================================
            // 🔴 核心升级：奇偶层交错 (Staggered Grid)
            // 如果是奇数层，整体向右偏移半个节点间距 (80像素)
            // 这样彻底打破了同节点数时的绝对垂直对齐！
            // ==========================================
            if (layer % 2 != 0) {
                startX += nodeSpacingX / 2;
            }

            int baseX = startX + pos * nodeSpacingX;
            int baseY = 2700 - layer * 180;

            node.uiX = baseX + QRandomGenerator::global()->bounded(-20, 20);
            node.uiY = baseY + QRandomGenerator::global()->bounded(-40, 40);

            layerNodes.append(node);
        }
        m_mapData.insert(layer, layerNodes);
    }

    // ==========================================
    // 第二步：交叉连线算法
    // ==========================================
    for (int i = 0; i < TOTAL_LAYERS - 1; ++i) {
        QList<MapNode>& currentLayer = m_mapData[i];
        QList<MapNode>& nextLayer = m_mapData[i+1];

        for (int nextIdx = 0; nextIdx < nextLayer.size(); ++nextIdx) {
            int currIdx = (nextIdx * currentLayer.size()) / nextLayer.size();
            currentLayer[currIdx].nextNodes.append(nextLayer[nextIdx].id);
        }
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
    // 第三步：渲染高清 PNG 贴图按钮
    // ==========================================
    m_nodeButtons.clear();
    m_visitedNodes.clear();
    m_currentLayer = -1;
    m_currentNodeId = -1;

    for (int layer = 0; layer < m_mapData.size(); ++layer) {
        for (const MapNode& node : m_mapData[layer]) {
            QPushButton* btn = new QPushButton(this);
            m_nodeButtons.insert(node.id, btn);

            int iconWidth = 65;
            int iconHeight = 65;
            QString imagePath;

            // 匹配 map_images 文件夹下的对应高清贴图
            if (node.type == "Boss") {
                imagePath = ":/resources/images/map_images/elite.png"; // Boss 暂用精英怪图标放大代替
                iconWidth = 100;
                iconHeight = 100;
            } else if (node.type == "Campfire") {
                imagePath = ":/resources/images/map_images/campfire.png";
            } else if (node.type == "Chest") {
                imagePath = ":/resources/images/map_images/chest.png";
            } else if (node.type == "Elite") {
                imagePath = ":/resources/images/map_images/elite.png";
                iconWidth = 80;
                iconHeight = 80;
            } else if (node.type == "Event") {
                imagePath = ":/resources/images/map_images/event.png";
            } else {
                imagePath = ":/resources/images/map_images/monster.png";
            }

            // 微调中心点，使大图标依然能与连线对齐
            btn->setGeometry(node.uiX - (iconWidth - 65)/2, node.uiY - (iconHeight - 45)/2, iconWidth, iconHeight);

            // 核心样式：设置贴图、透明背景、以及鼠标悬停发光效果
            // 🔴 1. 纯净版样式：删掉原来的 :hover 白框，只保留透明背景和贴图
            QString styleSheet = QString(
                                     "QPushButton {"
                                     "   border-image: url(%1);"
                                     "   background-color: transparent;"
                                     "}"
                                     ).arg(imagePath);
            btn->setStyleSheet(styleSheet);
            btn->setText(""); // 清空占位文字

            // 🔴 2. 给每个按钮装上“记忆”和“监听器”
            // 把每个按钮刚生成时的完美坐标和大小记录在它的私有属性里（防止动画乱漂）
            btn->setProperty("baseGeometry", btn->geometry());
            // 让 MapManager 亲自接管这个按钮的鼠标事件
            btn->installEventFilter(this);

            connect(btn, &QPushButton::clicked, this, [this, node]() {
                this->triggerNode(node);
            });

            btn->show();
        }
    }
    refreshNodeStates();
}

void MapManager::triggerNode(const MapNode& clickedNode) {
    // 以前是 emit battleRequested(clickedNode);
    // 🔴 现在变成无情的中转站，不管是什么节点，直接发射给 GameWindow 裁决！
    emit nodeClicked(clickedNode);
}
// ==========================================
// 绘制大地图底层的交叉连线 (发散触点纯净版)
// ==========================================
void MapManager::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 🌟 1. 恢复了这里的空格！必须和你的实际文件名严格一致
    QPixmap bgMap(":/resources/images/map_images/map_bg .jpg");
    if (!bgMap.isNull()) {
        painter.drawPixmap(0, 0, this->width(), this->height(), bgMap);
    } else {
        qWarning() << "[UI] ⚠️ 警告：无法加载地图背景图片，请检查路径！";
    }

    // 2. 绘制连线 (调低透明度使其更像底纹)
    QPen pen(QColor(60, 60, 60, 180), 3, Qt::DashLine);
    painter.setPen(pen);

    for (int layer = 0; layer < m_mapData.size() - 1; ++layer) {
        const QList<MapNode>& currentLayerNodes = m_mapData[layer];
        const QList<MapNode>& nextLayerNodes = m_mapData[layer + 1];

        for (const MapNode& currNode : currentLayerNodes) {
            // 起点：当前节点的正上方边缘
            int startX = currNode.uiX + 32;
            int startY = currNode.uiY - 5;

            for (int nextId : currNode.nextNodes) {
                for (const MapNode& targetNode : nextLayerNodes) {
                    if (targetNode.id == nextId) {

                        // 终点基础 X 坐标
                        int baseEndX = targetNode.uiX + 32;

                        // 🌟 3. 发散触点逻辑：解决所有线扎在一个点上太死板的问题
                        // 根据X轴距离加一点微小偏移，但最大不超过15像素，杜绝乱飞
                        int xOffset = (startX - baseEndX) * 0.1;
                        if (xOffset > 15) xOffset = 15;
                        if (xOffset < -15) xOffset = -15;

                        int endX = baseEndX + xOffset;
                        int endY = targetNode.uiY + 65 + 5;

                        QPainterPath path;
                        path.moveTo(startX, startY);

                        // 弯曲度严格限定为 Y 轴距离的一半，杜绝溢出盖脸
                        int ctrlYOffset = abs(startY - endY) / 2;

                        QPointF c1(startX, startY - ctrlYOffset);
                        QPointF c2(endX, endY + ctrlYOffset);

                        path.cubicTo(c1, c2, QPointF(endX, endY));

                        painter.drawPath(path);
                        break;
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
    QList<int> availableNextNodeIds;
    if (m_currentLayer != -1 && m_currentNodeId != -1) {
        for (const MapNode& node : m_mapData[m_currentLayer]) {
            if (node.id == m_currentNodeId) {
                availableNextNodeIds = node.nextNodes;
                break;
            }
        }
    }

    for (int layer = 0; layer < m_mapData.size(); ++layer) {
        for (const MapNode& node : m_mapData[layer]) {
            QPushButton* btn = m_nodeButtons[node.id];

            // 命运 A：已经被踩过的节点（打叉标记）
            if (m_visitedNodes.contains(node.id)) {
                btn->setText("❌");
                // 在原有图片基础上，叠加一层半透明黑纱和红色字体
                btn->setStyleSheet(btn->styleSheet() +
                                   "QPushButton { font-size: 36px; color: red; background-color: rgba(0,0,0,120); }");
                btn->setEnabled(false);
                continue;
            }

            // 命运 B：未被踩过的节点，判断是否放行
            if (m_currentLayer == -1) {
                btn->setEnabled(node.layer == 0);
            } else {
                bool isNextLayer = (node.layer == m_currentLayer + 1);
                bool isConnected = availableNextNodeIds.contains(node.id);
                btn->setEnabled(isNextLayer && isConnected);
            }
        }
    }
}

// ==========================================
// 🌟 视觉升级：鼠标悬停平滑缩放动画
// ==========================================
bool MapManager::eventFilter(QObject *watched, QEvent *event) {
    QPushButton* btn = qobject_cast<QPushButton*>(watched);

    // 我们只对有效（未被打叉禁用）的按钮施加魔法
    if (btn && btn->isEnabled()) {

        // 当鼠标踏入图标的领地...
        if (event->type() == QEvent::Enter) {
            // 提取它最初的记忆体型
            QRect baseGeom = btn->property("baseGeometry").toRect();

            // 创建一个 120 毫秒的极速平滑动画
            QPropertyAnimation* anim = new QPropertyAnimation(btn, "geometry", btn);
            anim->setDuration(120);
            // 目标大小：上下左右各往外扩张 8 个像素（整体放大 16 像素）
            anim->setEndValue(baseGeom.adjusted(-8, -8, 8, 8));
            anim->start(QAbstractAnimation::DeleteWhenStopped);

            return true; // 告诉系统：这个事件我处理完了
        }
        // 当鼠标恋恋不舍地离开...
        else if (event->type() == QEvent::Leave) {
            QRect baseGeom = btn->property("baseGeometry").toRect();

            QPropertyAnimation* anim = new QPropertyAnimation(btn, "geometry", btn);
            anim->setDuration(120);
            // 目标大小：精准恢复到它出生时的完美比例
            anim->setEndValue(baseGeom);
            anim->start(QAbstractAnimation::DeleteWhenStopped);

            return true;
        }
    }

    // 其他不关心的事件，原封不动地还给父类处理
    return QWidget::eventFilter(watched, event);
}