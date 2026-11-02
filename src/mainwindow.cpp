#include "mainwindow.h"
#include "glyphcanvas.h"
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("GLIFOR — Font Editor");
    resize(1440, 900);
    Glyph a; a.name = "A"; a.codepoint = 65;
    Glyph b; b.name = "B"; b.codepoint = 66;
    Glyph o; o.name = "O"; o.codepoint = 79;
    m_glyphs = {a, b, o};
    createUi(); createActions(); refreshGlyphList(); selectGlyph(0); updateStatus();
}
void MainWindow::createUi() {
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    auto *left = new QWidget(splitter);
    left->setMinimumWidth(170);
    auto *leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    auto *glyphTitle = new QLabel("GLYPHS");
    glyphTitle->setStyleSheet("font-weight: 700; letter-spacing: 2px; color: #aeb9c8;");
    leftLayout->addWidget(glyphTitle);
    m_glyphList = new QListWidget(left);
    m_glyphList->setSpacing(3);
    leftLayout->addWidget(m_glyphList);
    connect(m_glyphList, &QListWidget::currentRowChanged, this, &MainWindow::selectGlyph);

    auto *center = new QWidget(splitter);
    auto *centerLayout = new QVBoxLayout(center);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    m_canvas = new GlyphCanvas(center);
    centerLayout->addWidget(m_canvas, 1);
    connect(m_canvas, &GlyphCanvas::pointAdded, this, [this](const QPointF &point) {
        if (m_currentIndex < 0 || m_currentIndex >= m_glyphs.size()) return;
        Glyph &g = m_glyphs[m_currentIndex];
        g.points.append(point); g.closed = false;
        m_canvas->setGlyph(&g); updateStatus();
    });

    auto *right = new QWidget(splitter);
    right->setMinimumWidth(210);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    auto *title = new QLabel("GLYPH PROPERTIES");
    title->setStyleSheet("font-weight: 700; letter-spacing: 1px; color: #aeb9c8;");
    rightLayout->addWidget(title);
    rightLayout->addWidget(new QLabel("Advance width"));
    m_advanceWidth = new QSpinBox(right);
    m_advanceWidth->setRange(100, 2000); m_advanceWidth->setSingleStep(10);
    rightLayout->addWidget(m_advanceWidth);
    connect(m_advanceWidth, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
        if (m_currentIndex < 0 || m_currentIndex >= m_glyphs.size()) return;
        m_glyphs[m_currentIndex].advanceWidth = value;
        m_canvas->setGlyph(&m_glyphs[m_currentIndex]); updateStatus();
    });
    rightLayout->addSpacing(16);
    auto *close = new QPushButton("Close contour", right);
    auto *clear = new QPushButton("Clear outline", right);
    rightLayout->addWidget(close); rightLayout->addWidget(clear);
    connect(close, &QPushButton::clicked, this, [this]() {
        if (m_currentIndex < 0 || m_currentIndex >= m_glyphs.size()) return;
        Glyph &g = m_glyphs[m_currentIndex];
        if (g.points.size() >= 3) g.closed = true;
        m_canvas->setGlyph(&g); updateStatus();
    });
    connect(clear, &QPushButton::clicked, this, [this]() {
        if (m_currentIndex < 0 || m_currentIndex >= m_glyphs.size()) return;
        Glyph &g = m_glyphs[m_currentIndex];
        g.points.clear(); g.closed = false;
        m_canvas->setGlyph(&g); updateStatus();
    });
    rightLayout->addStretch();
    auto *hint = new QLabel("Mouse: add nodes\nClose: finish polygon\nJSON: save project");
    hint->setStyleSheet("color: #8995a5;");
    rightLayout->addWidget(hint);
    splitter->addWidget(left); splitter->addWidget(center); splitter->addWidget(right);
    splitter->setStretchFactor(0, 0); splitter->setStretchFactor(1, 1); splitter->setStretchFactor(2, 0);
    splitter->setSizes({210, 1000, 230});
    setCentralWidget(splitter);
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);
    setStyleSheet(
        "QMainWindow, QWidget { background: #292f38; color: #e7ebf1; }"
        "QListWidget { background: #1d2229; border: 1px solid #414a57; padding: 5px; }"
        "QListWidget::item { padding: 9px; border-radius: 4px; }"
        "QListWidget::item:selected { background: #435d78; }"
        "QSpinBox, QPushButton { background: #353d48; border: 1px solid #4a5665; padding: 7px; }"
        "QPushButton:hover { background: #465466; }"
        "QToolBar { background: #222830; border: 0; spacing: 6px; padding: 5px; }"
        "QStatusBar { background: #20262e; color: #aeb9c8; }");
}
void MainWindow::createActions() {
    auto *toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    auto add = [toolbar, this](const QString &label, const QKeySequence &key, auto callback) {
        auto *action = new QAction(label, this);
        if (!key.isEmpty()) action->setShortcut(key);
        connect(action, &QAction::triggered, this, callback);
        toolbar->addAction(action);
    };
    add("New", QKeySequence::New, [this]() { newProject(); });
    add("Open", QKeySequence::Open, [this]() { openProject(); });
    add("Save", QKeySequence::Save, [this]() { saveProject(); });
    toolbar->addSeparator();
    add("+ Glyph", QKeySequence(), [this]() { addGlyph(); });
}
void MainWindow::refreshGlyphList() {
    m_glyphList->clear();
    for (const Glyph &g : m_glyphs) {
        m_glyphList->addItem(QString("%1     U+%2").arg(g.displayName())
            .arg(g.codepoint, 4, 16, QLatin1Char('0')).toUpper());
    }
}
void MainWindow::selectGlyph(int row) {
    if (row < 0 || row >= m_glyphs.size()) return;
    m_currentIndex = row;
    m_advanceWidth->blockSignals(true);
    m_advanceWidth->setValue(m_glyphs[row].advanceWidth);
    m_advanceWidth->blockSignals(false);
    m_canvas->setGlyph(&m_glyphs[row]); updateStatus();
}
void MainWindow::newProject() {
    m_glyphs.clear();
    Glyph a; a.name = "A"; a.codepoint = 65; m_glyphs.append(a);
    m_currentIndex = 0; m_projectPath.clear();
    refreshGlyphList(); m_glyphList->setCurrentRow(0);
}
void MainWindow::addGlyph() {
    bool accepted = false;
    const QString text = QInputDialog::getText(this, "Add glyph", "Enter one character:",
        QLineEdit::Normal, QString(), &accepted);
    if (!accepted || text.isEmpty()) return;
    const uint cp = text.toUcs4().first();
    for (const Glyph &g : m_glyphs) {
        if (g.codepoint == cp) {
            QMessageBox::information(this, "GLIFOR", "This glyph is already in the project.");
            return;
        }
    }
    Glyph g; g.codepoint = cp; g.name = QString::fromUcs4(&cp, 1);
    m_glyphs.append(g); refreshGlyphList(); m_glyphList->setCurrentRow(m_glyphs.size() - 1);
}
void MainWindow::openProject() {
    const QString path = QFileDialog::getOpenFileName(this, "Open GLIFOR project", QString(),
        "GLIFOR project (*.glifor.json);;JSON files (*.json)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Open failed", file.errorString()); return;
    }
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::warning(this, "Invalid project", error.errorString()); return;
    }
    QVector<Glyph> loaded;
    for (const QJsonValue &v : doc.object().value("glyphs").toArray())
        if (v.isObject()) loaded.append(Glyph::fromJson(v.toObject()));
    if (loaded.isEmpty()) {
        QMessageBox::warning(this, "Invalid project", "The file contains no glyphs."); return;
    }
    m_glyphs = loaded; m_projectPath = path;
    refreshGlyphList(); m_glyphList->setCurrentRow(0);
}
void MainWindow::saveProject() {
    QString path = m_projectPath;
    if (path.isEmpty()) path = QFileDialog::getSaveFileName(this, "Save GLIFOR project",
        "Untitled.glifor.json", "GLIFOR project (*.glifor.json);;JSON files (*.json)");
    if (path.isEmpty()) return;
    QJsonArray glyphs;
    for (const Glyph &g : m_glyphs) glyphs.append(g.toJson());
    QJsonObject root;
    root.insert("format", "GLIFOR"); root.insert("version", 1);
    root.insert("unitsPerEm", 1000); root.insert("glyphs", glyphs);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Save failed", file.errorString()); return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    m_projectPath = path; updateStatus();
}
void MainWindow::updateStatus() {
    if (m_currentIndex < 0 || m_currentIndex >= m_glyphs.size()) return;
    const Glyph &g = m_glyphs[m_currentIndex];
    m_statusLabel->setText(QString("Glyph: %1   |   Nodes: %2   |   Advance: %3   |   UPM: 1000   |   %4")
        .arg(g.displayName()).arg(g.points.size()).arg(g.advanceWidth)
        .arg(m_projectPath.isEmpty() ? "Unsaved project" : m_projectPath));
}
