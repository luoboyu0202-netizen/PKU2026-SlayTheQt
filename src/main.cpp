#include <QApplication>
#include <QScrollArea>      // 👈 引入 Qt 官方的滚动区域组件
#include "map/mapmanager.h" // 你的大地图图纸

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 实例化你刚刚写好的大地图（真实的超大画板）
    MapManager* mapWidget = new MapManager();

    // 2. 创造一个“滚动外壳” (这就是玩家真正看到的窗口)
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWindowTitle("Slay the Qt - 大地图测试");

    // 3. 把你的大地图塞进这个滚动外壳里！
    scrollArea->setWidget(mapWidget);

    // 🔴 核心要求一：让按钮整体在窗口中间对齐
    scrollArea->setAlignment(Qt::AlignCenter);

    // 🔴 核心要求二：整个游戏窗口不可缩放 (锁死外壳尺寸为 1280x720)
    scrollArea->setFixedSize(1280, 720);

    // 显示外壳！
    scrollArea->show();

    return a.exec();
}