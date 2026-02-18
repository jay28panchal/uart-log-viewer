#include "finddialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

FindDialog::FindDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Find");
    setModal(false);

    m_query = new QLineEdit(this);
    m_matchCase = new QCheckBox("Match case", this);
    m_down = new QRadioButton("Down", this);
    m_up = new QRadioButton("Up", this);
    m_down->setChecked(true);

    auto findBtn = new QPushButton("Find Next", this);
    auto closeBtn = new QPushButton("Close", this);

    connect(findBtn, &QPushButton::clicked, this, &FindDialog::findNextRequested);
    connect(closeBtn, &QPushButton::clicked, this, &FindDialog::close);

    auto row = new QHBoxLayout();
    row->addWidget(new QLabel("Find:", this));
    row->addWidget(m_query);

    auto opts = new QHBoxLayout();
    opts->addWidget(m_matchCase);
    opts->addSpacing(10);
    opts->addWidget(new QLabel("Direction:", this));
    opts->addWidget(m_down);
    opts->addWidget(m_up);
    opts->addStretch();

    auto buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(findBtn);
    buttons->addWidget(closeBtn);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(row);
    layout->addLayout(opts);
    layout->addLayout(buttons);

    m_query->setFocus();
}

QString FindDialog::query() const { return m_query->text(); }
bool FindDialog::matchCase() const { return m_matchCase->isChecked(); }
bool FindDialog::directionDown() const { return m_down->isChecked(); }
