#pragma once
#include "glyph.h"
#include <QMainWindow>
#include <QVector>
class GlyphCanvas;
class QListWidget;
class QSpinBox;
class QLabel;
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    void createUi();
    void createActions();
    void refreshGlyphList();
    void selectGlyph(int row);
    void newProject();
    void addGlyph();
    void openProject();
    void saveProject();
    void updateStatus();
    QVector<Glyph> m_glyphs;
    int m_currentIndex = 0;
    QString m_projectPath;
    QListWidget *m_glyphList = nullptr;
    GlyphCanvas *m_canvas = nullptr;
    QSpinBox *m_advanceWidth = nullptr;
    QLabel *m_statusLabel = nullptr;
};
