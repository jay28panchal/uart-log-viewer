#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QTimeZone>
#include <QAction>

#include "serialtab.h"
#include "finddialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void newTab();
    void refreshPorts();
    void saveLogs();
    void toggleTimestamp(bool enabled);
    void selectTimezone();
    void openFind();
    void findNext();
    void setThemeDark();
    void setThemeLight();

private:
    QStringList availablePorts() const;
    SerialTab* currentTab() const;
    void applyTheme(bool dark);
    void highlightMatch(QTextEdit* textEdit, const QTextCursor& cursor);

    QTabWidget* m_tabs;
    QAction* m_timestampAction;
    QAction* m_themeDark;
    QAction* m_themeLight;
    QTimeZone m_timeZone;
    FindDialog* m_findDialog = nullptr;
};
