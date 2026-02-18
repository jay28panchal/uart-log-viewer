#pragma once

#include <QWidget>
#include <QSerialPort>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QDateTime>
#include <QTimeZone>

class SerialTab : public QWidget {
    Q_OBJECT
public:
    explicit SerialTab(const QString& portName, QWidget* parent = nullptr);

    QString portName() const { return m_portName; }
    void setTimestampEnabled(bool enabled) { m_timestampEnabled = enabled; }
    void setTimeZone(const QTimeZone& tz) { m_timeZone = tz; }

    void appendText(const QString& text);
    QTextEdit* textEdit() const { return m_textEdit; }

signals:
    void statusChanged(const QString& status);

private slots:
    void toggleConnect();
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void sendLine();
    void clearSend();

private:
    QString formatWithTimestamp(const QString& text);
    QString currentTimestamp() const;
    void setConnectedUi(bool connected);

    QString m_portName;
    QSerialPort m_serial;
    QTextEdit* m_textEdit;
    QComboBox* m_baudCombo;
    QPushButton* m_connectBtn;
    QLineEdit* m_sendEdit;
    QPushButton* m_sendBtn;
    QPushButton* m_clearBtn;
    QLabel* m_statusLabel;

    bool m_timestampEnabled = false;
    QTimeZone m_timeZone;
    QString m_lineBuffer;
};
