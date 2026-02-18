#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>

class FindDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindDialog(QWidget* parent = nullptr);

    QString query() const;
    bool matchCase() const;
    bool directionDown() const;

signals:
    void findNextRequested();

private:
    QLineEdit* m_query;
    QCheckBox* m_matchCase;
    QRadioButton* m_down;
    QRadioButton* m_up;
};
