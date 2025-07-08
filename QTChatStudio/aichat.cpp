#include "aichat.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>

aichat::aichat(QObject *parent)
    : QObject{parent}
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &aichat::handleResponse);
}

void aichat::sendMessage(const QString &message, const QString &model)
{
    QNetworkRequest request(QUrl("http://localhost:11434/api/generate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject requestData;
    requestData["model"] = model;
    requestData["prompt"] = message;
    requestData["stream"] = false;

    QJsonDocument doc(requestData);
    m_networkManager->post(request, doc.toJson());
}

void aichat::handleResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();
        
        m_lastResponse = obj.value("response").toString();
        emit responseReady(m_lastResponse);
    } else {
        QString errorMsg = QString("Network error: %1").arg(reply->errorString());
        qDebug() << "[AI Chat]:" << errorMsg;
        emit errorOccurred(errorMsg);
    }
    
    reply->deleteLater();
}
