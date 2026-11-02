#include "glyph.h"
#include <QJsonArray>
QJsonObject Glyph::toJson() const {
    QJsonArray pointArray;
    for (const QPointF &point : points) {
        QJsonObject item;
        item.insert("x", point.x());
        item.insert("y", point.y());
        pointArray.append(item);
    }
    QJsonObject object;
    object.insert("name", name);
    object.insert("codepoint", static_cast<int>(codepoint));
    object.insert("advanceWidth", advanceWidth);
    object.insert("closed", closed);
    object.insert("points", pointArray);
    return object;
}
Glyph Glyph::fromJson(const QJsonObject &object) {
    Glyph glyph;
    glyph.name = object.value("name").toString();
    glyph.codepoint = static_cast<uint>(object.value("codepoint").toInt(65));
    glyph.advanceWidth = object.value("advanceWidth").toInt(650);
    glyph.closed = object.value("closed").toBool();
    for (const QJsonValue &value : object.value("points").toArray()) {
        const QJsonObject point = value.toObject();
        glyph.points.append(QPointF(point.value("x").toDouble(), point.value("y").toDouble()));
    }
    return glyph;
}
QString Glyph::displayName() const {
    if (!name.isEmpty()) return name;
    return QString::fromUcs4(&codepoint, 1);
}
