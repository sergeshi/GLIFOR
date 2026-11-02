#pragma once
#include "glyph.h"
#include <QWidget>
class GlyphCanvas : public QWidget {
    Q_OBJECT
public:
    explicit GlyphCanvas(QWidget *parent = nullptr);
    void setGlyph(const Glyph *glyph);
signals:
    void pointAdded(const QPointF &point);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    const Glyph *m_glyph = nullptr;
    qreal m_scale = 1.0;
    qreal m_originX = 0.0;
    qreal m_baselineY = 0.0;
};
