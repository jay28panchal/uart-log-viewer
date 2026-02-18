#include "timezonedialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

TimezoneDialog::TimezoneDialog(const QTimeZone& current, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Select Timezone");

    m_combo = new QComboBox(this);
    const QList<QByteArray> zones = QTimeZone::availableTimeZoneIds();
    for (const auto& z : zones) {
        m_combo->addItem(QString::fromUtf8(z));
    }

    const QString currentId = QString::fromUtf8(current.id());
    int idx = m_combo->findText(currentId);
    if (idx >= 0) m_combo->setCurrentIndex(idx);

    auto okBtn = new QPushButton("Set", this);
    auto cancelBtn = new QPushButton("Cancel", this);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto row = new QHBoxLayout();
    row->addWidget(new QLabel("Timezone:", this));
    row->addWidget(m_combo);

    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(okBtn);
    buttons->addWidget(cancelBtn);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(row);
    layout->addLayout(buttons);
}

QTimeZone TimezoneDialog::selectedTimeZone() const {
    return QTimeZone(m_combo->currentText().toUtf8());
}
