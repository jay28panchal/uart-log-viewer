#pragma once

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QTimeZone>

class TimezoneDialog : public QDialog {
    Q_OBJECT
public:
    explicit TimezoneDialog(const QTimeZone& current, QWidget* parent = nullptr);

    QTimeZone selectedTimeZone() const;

private:
    QComboBox* m_combo;
};
