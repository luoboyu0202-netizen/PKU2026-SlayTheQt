#pragma once
#include <QString>
#include <QList>

class Relic;
class QObject;

class RelicFactory {
public:
    static QList<QString> getAllAvailableRelicIds();
    static Relic* createRelic(const QString& relicId, QObject* parent = nullptr);
    static Relic* generateRandomRelic(QObject* parent = nullptr);

private:
    RelicFactory() = default;
    ~RelicFactory() = default;
};
