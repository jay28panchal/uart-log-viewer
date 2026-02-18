#include "mainwindow.h"
#include "timezonedialog.h"

#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QSerialPortInfo>
#include <QTextDocument>
#include <QTextCursor>
#include <QRegularExpression>
#include <QPalette>
#include <QFile>
#include <QApplication>
#include <QTabBar>

static QStringList filterPorts(const QList<QSerialPortInfo>& infos) {
    QStringList ports;
#ifdef Q_OS_WIN
    for (const auto& info : infos) ports << info.portName();
#elif defined(Q_OS_MAC)
    for (const auto& info : infos) {
        const QString name = info.portName();
        if (name.startsWith("cu.") || name.startsWith("tty.")) ports << name;
    }
#else
    for (const auto& info : infos) {
        const QString name = info.portName();
        if (name.startsWith("ttyUSB") || name.startsWith("ttyACM")) ports << name;
    }
#endif
    ports.sort();
    return ports;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("UART Log Viewer");
    resize(980, 620);

    m_tabs = new QTabWidget(this);
    setCentralWidget(m_tabs);
    m_tabs->tabBar()->setMovable(false);

    m_timeZone = QTimeZone::systemTimeZone();

    auto fileMenu = menuBar()->addMenu("File");
    auto toolsMenu = menuBar()->addMenu("Tools");

    fileMenu->addAction("New Tab...", this, &MainWindow::newTab);
    fileMenu->addAction("Refresh Ports", this, &MainWindow::refreshPorts);
    fileMenu->addAction("Save Logs...", this, &MainWindow::saveLogs);
    fileMenu->addSeparator();
    fileMenu->addAction("Quit", this, &QWidget::close);

    m_timestampAction = toolsMenu->addAction("Timestamp Each Line");
    m_timestampAction->setCheckable(true);
    connect(m_timestampAction, &QAction::toggled, this, &MainWindow::toggleTimestamp);

    toolsMenu->addAction("Timezone...", this, &MainWindow::selectTimezone);

    auto findAction = toolsMenu->addAction("Find...");
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::openFind);

    toolsMenu->addSeparator();
    m_themeDark = toolsMenu->addAction("Theme: Dark", this, &MainWindow::setThemeDark);
    m_themeLight = toolsMenu->addAction("Theme: Light", this, &MainWindow::setThemeLight);
    m_themeDark->setCheckable(true);
    m_themeLight->setCheckable(true);
    m_themeDark->setChecked(true);

    applyTheme(true);
    ensurePlusTab();
}

void MainWindow::refreshPorts() {
    const QStringList ports = availablePorts();
    if (ports.isEmpty()) {
        QMessageBox::information(this, "UART Log Viewer", "No serial ports detected.");
    }
}

QStringList MainWindow::availablePorts() const {
    return filterPorts(QSerialPortInfo::availablePorts());
}

void MainWindow::newTab() {
    const QStringList ports = availablePorts();
    if (ports.isEmpty()) {
        QMessageBox::information(this, "UART Log Viewer", "No serial ports detected.");
        return;
    }

    QStringList existing;
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto tab = qobject_cast<SerialTab*>(m_tabs->widget(i));
        if (tab) existing << tab->portName();
    }

    QStringList candidates;
    for (const auto& p : ports) if (!existing.contains(p)) candidates << p;
    if (candidates.isEmpty()) {
        QMessageBox::information(this, "UART Log Viewer", "No new ports available.");
        return;
    }

    bool ok = false;
    const QString port = QInputDialog::getItem(this, "Open New UART", "Select port:", candidates, 0, false, &ok);
    if (!ok || port.isEmpty()) return;

    auto tab = new SerialTab(port, this);
    tab->setTimestampEnabled(m_timestampAction->isChecked());
    tab->setTimeZone(m_timeZone);
    int insertAt = m_tabs->count();
    if (m_plusTab) insertAt = m_tabs->indexOf(m_plusTab);
    m_tabs->insertTab(insertAt, tab, port);
    m_tabs->setCurrentWidget(tab);
}

void MainWindow::saveLogs() {
    auto tab = currentTab();
    if (!tab) return;

    const QString defaultName = QString("uart_log_%1.txt").arg(tab->portName().replace("/", "_"));
    const QString path = QFileDialog::getSaveFileName(this, "Save Logs", defaultName, "Text Files (*.txt);;All Files (*)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "UART Log Viewer", "Failed to write file.");
        return;
    }
    file.write(tab->textEdit()->toPlainText().toUtf8());
}

void MainWindow::toggleTimestamp(bool enabled) {
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto tab = qobject_cast<SerialTab*>(m_tabs->widget(i));
        if (tab) tab->setTimestampEnabled(enabled);
    }
}

void MainWindow::selectTimezone() {
    TimezoneDialog dlg(m_timeZone, this);
    if (dlg.exec() != QDialog::Accepted) return;
    m_timeZone = dlg.selectedTimeZone();
    for (int i = 0; i < m_tabs->count(); ++i) {
        auto tab = qobject_cast<SerialTab*>(m_tabs->widget(i));
        if (tab) tab->setTimeZone(m_timeZone);
    }
}

void MainWindow::openFind() {
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this);
        connect(m_findDialog, &FindDialog::findNextRequested, this, &MainWindow::findNext);
    }
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::findNext() {
    auto tab = currentTab();
    if (!tab || !m_findDialog) return;

    QTextDocument::FindFlags flags;
    if (m_findDialog->matchCase()) flags |= QTextDocument::FindCaseSensitively;
    if (!m_findDialog->directionDown()) flags |= QTextDocument::FindBackward;

    QTextEdit* edit = tab->textEdit();
    QTextCursor cursor = edit->textCursor();
    cursor = edit->document()->find(m_findDialog->query(), cursor, flags);
    if (cursor.isNull()) {
        cursor = edit->document()->find(m_findDialog->query(), flags);
    }
    if (cursor.isNull()) {
        QMessageBox::information(this, "UART Log Viewer", "Text not found.");
        return;
    }

    edit->setTextCursor(cursor);
    highlightMatch(edit, cursor);
}

void MainWindow::highlightMatch(QTextEdit* textEdit, const QTextCursor& cursor) {
    QTextEdit::ExtraSelection sel;
    sel.cursor = cursor;
    sel.format.setBackground(QColor("#ffd966"));

    QList<QTextEdit::ExtraSelection> selections;
    selections << sel;
    textEdit->setExtraSelections(selections);
}

SerialTab* MainWindow::currentTab() const {
    return qobject_cast<SerialTab*>(m_tabs->currentWidget());
}

void MainWindow::setThemeDark() {
    m_themeDark->setChecked(true);
    m_themeLight->setChecked(false);
    applyTheme(true);
}

void MainWindow::setThemeLight() {
    m_themeDark->setChecked(false);
    m_themeLight->setChecked(true);
    applyTheme(false);
}

void MainWindow::applyTheme(bool dark) {
    QPalette p;
    if (dark) {
        p.setColor(QPalette::Window, QColor("#1A1410"));
        p.setColor(QPalette::WindowText, QColor("#F5E6D3"));
        p.setColor(QPalette::Base, QColor("#1A1410"));
        p.setColor(QPalette::AlternateBase, QColor("#221C16"));
        p.setColor(QPalette::Text, QColor("#F5E6D3"));
        p.setColor(QPalette::Button, QColor("#221C16"));
        p.setColor(QPalette::ButtonText, QColor("#F5E6D3"));
        p.setColor(QPalette::Highlight, QColor("#E67D22"));
        p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    } else {
        p.setColor(QPalette::Window, QColor("#FAF9F5"));
        p.setColor(QPalette::WindowText, QColor("#1F1E1D"));
        p.setColor(QPalette::Base, QColor("#FAF9F5"));
        p.setColor(QPalette::AlternateBase, QColor("#F4F3EE"));
        p.setColor(QPalette::Text, QColor("#1F1E1D"));
        p.setColor(QPalette::Button, QColor("#F4F3EE"));
        p.setColor(QPalette::ButtonText, QColor("#1F1E1D"));
        p.setColor(QPalette::Highlight, QColor("#C96442"));
        p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    }
    qApp->setPalette(p);
}

void MainWindow::ensurePlusTab() {
    if (m_plusTab) return;
    m_plusTab = new QWidget(this);
    m_tabs->addTab(m_plusTab, "+");
    connect(m_tabs->tabBar(), &QTabBar::tabBarClicked, this, [this](int index) {
        if (isPlusTabIndex(index)) {
            newTab();
        }
    });
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (isPlusTabIndex(index)) {
            int fallback = m_tabs->count() > 1 ? 0 : -1;
            if (fallback >= 0) m_tabs->setCurrentIndex(fallback);
        }
    });
}

bool MainWindow::isPlusTabIndex(int index) const {
    return m_plusTab && index == m_tabs->indexOf(m_plusTab);
}
