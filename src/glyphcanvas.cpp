#include "glyphcanvas.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
GlyphCanvas::GlyphCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumSize(420, 420);
    setMouseTracking(true);
}
void GlyphCanvas::setGlyph(const Glyph *glyph) { m_glyph = glyph; update(); }
void GlyphCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#20242b"));
    const int advance = m_glyph ? std::max(300, m_glyph->advanceWidth) : 650;
    m_scale = std::min((width() - 100.0) / (advance + 180.0), (height() - 110.0) / 1120.0);
    m_scale = std::max<qreal>(0.1, m_scale);
    m_originX = (width() - advance * m_scale) / 2.0;
    m_baselineY = height() - 55.0;
    painter.setPen(QPen(QColor("#39414d"), 1, Qt::DashLine));
    for (int unit = 0; unit <= 1000; unit += 100) {
        const qreal y = m_baselineY - unit * m_scale;
        painter.drawLine(0, qRound(y), width(), qRound(y));
    }
    painter.setPen(QPen(QColor("#778392"), 1));
    painter.drawLine(0, qRound(m_baselineY), width(), qRound(m_baselineY));
    painter.drawLine(qRound(m_originX), 15, qRound(m_originX), height() - 20);
    painter.drawLine(qRound(m_originX + advance * m_scale), 15, qRound(m_originX + advance * m_scale), height() - 20);
    painter.setPen(QColor("#aab4c2"));
    painter.drawText(14, 24, "GLYPH EDITOR  /  1000 UPM");
    painter.drawText(14, height() - 18, "Baseline");
    painter.drawText(qRound(m_originX + advance * m_scale) - 45, height() - 18, QString("AW %1").arg(advance));
    if (!m_glyph || m_glyph->points.isEmpty()) {
        painter.setPen(QColor("#aab4c2"));
        painter.drawText(rect(), Qt::AlignCenter, "Click to add outline points\nClose contour when finished");
        return;
    }
    auto toScreen = [this](const QPointF &p) { return QPointF(m_originX + p.x() * m_scale, m_baselineY - p.y() * m_scale); };
    QPainterPath path;
    path.moveTo(toScreen(m_glyph->points.first()));
    for (int i = 1; i < m_glyph->points.size(); ++i) path.lineTo(toScreen(m_glyph->points.at(i)));
    if (m_glyph->closed && m_glyph->points.size() > 2) path.closeSubpath();
    painter.setPen(QPen(QColor("#d9e2f0"), 2));
    painter.setBrush(m_glyph->closed ? QBrush(QColor(91, 145, 190, 55)) : QBrush(Qt::NoBrush));
    painter.drawPath(path);
    painter.setBrush(QColor("#f0b35b"));
    painter.setPen(QPen(QColor("#20242b"), 1));
    for (const QPointF &p : m_glyph->points) painter.drawEllipse(toScreen(p), 4.5, 4.5);
}
void GlyphCanvas::mousePressEvent(QMouseEvent *event) {
    if (!m_glyph || event->button() != Qt::LeftButton) return;
    const QPointF pos = event->position();
    const qreal x = (pos.x() - m_originX) / m_scale;
    const qreal y = (m_baselineY - pos.y()) / m_scale;
    if (x < 0 || x > m_glyph->advanceWidth || y < 0 || y > 1100) return;
    emit pointAdded(QPointF(x, y));
}
