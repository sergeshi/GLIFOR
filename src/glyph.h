#pragma once
#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QVector>
struct Glyph {
    QString name;
    uint codepoint = 65;
    int advanceWidth = 650;
    QVector<QPointF> points;
    bool closed = false;
    QJsonObject toJson() const;
    static Glyph fromJson(const QJsonObject &object);
    QString displayName() const;
};
