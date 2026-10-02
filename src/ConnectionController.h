#pragma once
#include <QObject>
#include <QStringList>

class ConnectionController : public QObject {
    Q_OBJECT
        Q_PROPERTY(int dbTypeIndex READ dbTypeIndex WRITE setDbTypeIndex NOTIFY dbTypeIndexChanged)
        Q_PROPERTY(bool isRemote READ isRemote WRITE setIsRemote NOTIFY isRemoteChanged)
        Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
        Q_PROPERTY(QString port READ port WRITE setPort NOTIFY portChanged)
        Q_PROPERTY(QString login READ login WRITE setLogin NOTIFY loginChanged)
        Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
        Q_PROPERTY(QStringList configs READ configs NOTIFY configsChanged)
        Q_PROPERTY(QString currentConfig READ currentConfig WRITE setCurrentConfig NOTIFY currentConfigChanged)
        Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)
public:
    explicit ConnectionController(QObject* parent = nullptr);

    Q_INVOKABLE QString driverForIndex(int index) const;
    Q_INVOKABLE QString dbTypeForIndex(int index) const;
    Q_INVOKABLE QString displayTypeForIndex(int index) const;
    Q_INVOKABLE bool hasConfig(const QString& name) const;
    Q_INVOKABLE void loadConfig(const QString& name);
    Q_INVOKABLE void saveConfig(const QString& name);
    Q_INVOKABLE void resetForm();
    Q_INVOKABLE void checkConnection();
    Q_INVOKABLE void requestConnect();
    Q_INVOKABLE void appendLogMessage(const QString& message);

    int dbTypeIndex() const { return dbTypeIndex_; }
    bool isRemote() const { return isRemote_; }
    QString address() const { return address_; }
    QString port() const { return port_; }
    QString login() const { return login_; }
    QString password() const { return password_; }
    QStringList configs() const { return configs_; }
    QString currentConfig() const { return currentConfig_; }
    QString logText() const { return logText_; }

public slots:
    void setDbTypeIndex(int value);
    void setIsRemote(bool value);
    void setAddress(const QString& value);
    void setPort(const QString& value);
    void setLogin(const QString& value);
    void setPassword(const QString& value);
    void setCurrentConfig(const QString& value);

signals:
    void dbTypeIndexChanged(); void isRemoteChanged();
    void addressChanged(); void portChanged(); void loginChanged(); void passwordChanged();
    void configsChanged(); void currentConfigChanged(); void logTextChanged();
    void connected(const QString& driver, const QString& dbType, const QString& host,
        int port, const QString& login, const QString& password, const QString& dbPath);
    void connectionFailed(const QString& error);

private:
    QString configFilePath() const;
    QStringList readConfigs() const;
    void reloadConfigs();
    void appendLog(const QString& message);
    bool tryOpenConnection(const QString& prefix, QString& error, QString& outDriver, QString& outDbType,
        QString& outDbPath, QString& outHost, int& outPort,
        QString& outLogin, QString& outPassword);

    int dbTypeIndex_ = 0;
    bool isRemote_ = true;
    QString address_, port_, login_, password_, currentConfig_, logText_;
    QStringList configs_;
    int tempConnectionCounter_ = 0;
};