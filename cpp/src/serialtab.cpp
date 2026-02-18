#include "serialtab.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

static const QStringList kBaudRates = {
    "300", "600", "1200", "2400", "4800", "9600", "19200",
    "38400", "57600", "115200", "230400", "460800", "921600"
};

SerialTab::SerialTab(const QString& portName, QWidget* parent)
    : QWidget(parent), m_portName(portName) {

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);

    m_baudCombo = new QComboBox(this);
    m_baudCombo->addItems(kBaudRates);
    m_baudCombo->setCurrentText("230400");

    m_connectBtn = new QPushButton("Connect", this);
    connect(m_connectBtn, &QPushButton::clicked, this, &SerialTab::toggleConnect);

    m_sendEdit = new QLineEdit(this);
    m_sendBtn = new QPushButton("Enter", this);
    m_clearBtn = new QPushButton("Clear", this);
    m_logBtn = new QPushButton("Log...", this);

    connect(m_sendBtn, &QPushButton::clicked, this, &SerialTab::sendLine);
    connect(m_clearBtn, &QPushButton::clicked, this, &SerialTab::clearSend);
    connect(m_sendEdit, &QLineEdit::returnPressed, this, &SerialTab::sendLine);
    connect(m_logBtn, &QPushButton::clicked, this, &SerialTab::toggleLogging);

    m_statusLabel = new QLabel("Disconnected", this);

    auto topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel(QString("Port: %1").arg(m_portName), this));
    topRow->addStretch();
    topRow->addWidget(new QLabel("Baud:", this));
    topRow->addWidget(m_baudCombo);
    topRow->addWidget(m_connectBtn);

    auto sendRow = new QHBoxLayout();
    sendRow->addWidget(m_sendEdit);
    sendRow->addWidget(m_sendBtn);
    sendRow->addWidget(m_clearBtn);
    sendRow->addWidget(m_logBtn);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(topRow);
    layout->addWidget(m_textEdit);
    layout->addLayout(sendRow);
    layout->addWidget(m_statusLabel);

    connect(&m_serial, &QSerialPort::readyRead, this, &SerialTab::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred, this, &SerialTab::onErrorOccurred);

    m_timeZone = QTimeZone::systemTimeZone();
}

SerialTab::~SerialTab() {
    stopLogging();
}

void SerialTab::setConnectedUi(bool connected) {
    m_connectBtn->setText(connected ? "Disconnect" : "Connect");
    m_statusLabel->setText(connected ? QString("Connected @ %1").arg(m_baudCombo->currentText()) : "Disconnected");
    emit statusChanged(m_statusLabel->text());
}

void SerialTab::toggleConnect() {
    if (m_serial.isOpen()) {
        m_serial.close();
        setConnectedUi(false);
        stopLogging("Disconnected");
        return;
    }

    bool ok = false;
    int baud = m_baudCombo->currentText().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "UART Log Viewer", "Invalid baud rate.");
        return;
    }

    m_serial.setPortName(m_portName);
    m_serial.setBaudRate(baud);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "UART Log Viewer", QString("Failed to open %1: %2").arg(m_portName, m_serial.errorString()));
        return;
    }

    setConnectedUi(true);
}

void SerialTab::onReadyRead() {
    QByteArray data = m_serial.readAll();
    if (data.isEmpty()) return;

    QString text = QString::fromUtf8(data.constData(), data.size());
    text.replace("\x00", "");
    text.replace("\r\n", "\n");
    text.replace("\r", "\n");

    const QString formatted = formatWithTimestamp(text);
    appendText(formatted);
    writeLog(formatted);
}

void SerialTab::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) return;

    if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError) {
        m_serial.close();
        setConnectedUi(false);
        m_statusLabel->setText(QString("Disconnected (port removed)"));
        emit statusChanged(m_statusLabel->text());
        stopLogging("Port removed");
    }
}

void SerialTab::sendLine() {
    if (!m_serial.isOpen()) {
        m_statusLabel->setText("Disconnected (not connected)");
        emit statusChanged(m_statusLabel->text());
        return;
    }
    const QString data = m_sendEdit->text();
    QByteArray out = (data + "\r\n").toUtf8();
    if (m_serial.write(out) < 0) {
        QMessageBox::warning(this, "UART Log Viewer", "Send failed.");
    }
}

void SerialTab::clearSend() {
    m_sendEdit->clear();
}

void SerialTab::toggleLogging() {
    if (m_logging) {
        stopLogging("Logging stopped");
        return;
    }

    QString defaultName = QString("uart_log_%1.txt").arg(m_portName);
    QString path = QFileDialog::getSaveFileName(this, "Start Log", defaultName, "Text Files (*.txt);;All Files (*)");
    if (path.isEmpty()) return;

    m_logFile = new QFile(path, this);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QMessageBox::warning(this, "UART Log Viewer", "Failed to open log file.");
        delete m_logFile;
        m_logFile = nullptr;
        return;
    }

    m_logging = true;
    m_logBtn->setText("Stop Log");
    m_statusLabel->setText(QString("Logging to %1").arg(path));
    emit statusChanged(m_statusLabel->text());
}

void SerialTab::appendText(const QString& text) {
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text);
    m_textEdit->setTextCursor(cursor);
}

void SerialTab::writeLog(const QString& text) {
    if (!m_logging || !m_logFile) return;
    m_logFile->write(text.toUtf8());
    m_logFile->flush();
}

void SerialTab::stopLogging(const QString& reason) {
    if (!m_logging) return;
    m_logging = false;
    if (m_logFile) {
        m_logFile->flush();
        m_logFile->close();
        m_logFile->deleteLater();
        m_logFile = nullptr;
    }
    m_logBtn->setText("Log...");
    if (!reason.isEmpty()) {
        m_statusLabel->setText(reason);
        emit statusChanged(m_statusLabel->text());
    }
}

QString SerialTab::formatWithTimestamp(const QString& text) {
    if (!m_timestampEnabled) return text;

    QString combined = m_lineBuffer + text;
    QStringList lines = combined.split('\n');
    m_lineBuffer = lines.takeLast();

    QString out;
    for (const QString& line : lines) {
        if (line.isEmpty()) {
            out += "\n";
            continue;
        }
        out += QString("%1 %2\n").arg(currentTimestamp(), line);
    }
    return out;
}

QString SerialTab::currentTimestamp() const {
    const QDateTime now = QDateTime::currentDateTime().toTimeZone(m_timeZone);
    const int ms = now.time().msec();
    return QString("[%1]").arg(now.toString("dd-MM-yyyy HH:mm:ss:") + QString("%1").arg(ms, 3, 10, QChar('0')));
}
