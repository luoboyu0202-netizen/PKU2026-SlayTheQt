#include "Card.h"

Card::Card(const QString& id, const QString& name, int cost, bool isEthereal, QObject* parent)
    : QObject(parent), m_id(id), m_name(name), m_cost(cost), m_isEthereal(isEthereal) {
}
