#ifndef AICHAT_H
#define AICHAT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrl>

class aichat : public QObject
{
    Q_OBJECT
public:
    explicit aichat(QObject *parent = nullptr);
    void sendMessage(const QString &message, const QString &model = "llama3.2");
    QString getLastResponse() const { return m_lastResponse; }

private slots:
    void handleResponse(QNetworkReply *reply);

signals:
    void responseReady(const QString &response);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_lastResponse;
};

#endif // AICHAT_H
