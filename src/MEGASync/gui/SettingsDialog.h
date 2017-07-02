#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QtCore>
#include <QNetworkProxy>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QButtonGroup>
#include <ConnectivityChecker.h>

#include "AccountDetailsDialog.h"
#include "BindFolderDialog.h"
#include "SizeLimitDialog.h"
#include "DownloadFromMegaDialog.h"
#include "Preferences.h"
#include "megaapi.h"

namespace Ui {
class SettingsDialog;
}

class MegaProgressDialog : public QProgressDialog
{
public:
    MegaProgressDialog(const QString & labelText, const QString & cancelButtonText, int minimum, int maximum, QWidget * parent = 0, Qt::WindowFlags f = 0);
protected:
    void reject();
    void closeEvent(QCloseEvent * event);
};

class MegaApplication;
class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    enum {ACCOUNT_TAB = 0, SYNCS_TAB = 1, BANDWIDTH_TAB = 2, PROXY_TAB = 3, ADVANCED_TAB = 4};

    explicit SettingsDialog(MegaApplication *app, bool proxyOnly = false, QWidget *parent = 0);
    ~SettingsDialog();
    void setProxyOnly(bool proxyOnly);
    void setOverQuotaMode(bool mode);
    void loadSettings();
    void refreshAccountDetails();
    void setUpdateAvailable(bool updateAvailable);
    void openSettingsTab(int tab);

public slots:
    void stateChanged();
    void syncStateChanged(int state);
    void proxyStateChanged();
    void onLocalCacheSizeAvailable();
    void onRemoteCacheSizeAvailable();
    
private slots:
    void on_bAccount_clicked();

    void on_bSyncs_clicked();

    void on_bBandwidth_clicked();

    void on_bAdvanced_clicked();

    void on_bProxies_clicked();

    void on_bCancel_clicked();

    void on_bOk_clicked();

    void on_bHelp_clicked();

    void on_rProxyManual_clicked();

    void on_rProxyAuto_clicked();

    void on_rNoProxy_clicked();

    void on_bUpgrade_clicked();
    void on_bUpgradeBandwidth_clicked();

    void on_rUploadAutoLimit_clicked();
    void on_rUploadNoLimit_clicked();
    void on_rUploadLimit_clicked();

    void on_rDownloadNoLimit_clicked();
    void on_rDownloadLimit_clicked();

    void on_cProxyRequiresPassword_clicked();
#ifndef WIN32
    void on_bPermissions_clicked();
#endif
    void on_bAdd_clicked();
    void on_bApply_clicked();
    void on_bDelete_clicked();
    void on_bExcludeSize_clicked();

    void on_bUnlink_clicked();
    void on_bExportMasterKey_clicked();

    void on_tSyncs_doubleClicked(const QModelIndex &index);
    void on_bUploadFolder_clicked();
    void on_bDownloadFolder_clicked();

    void on_bAddName_clicked();
    void on_bDeleteName_clicked();
    void on_bClearCache_clicked();
    void on_bClearRemoteCache_clicked();
    void onProxyTestError();
    void onProxyTestSuccess();
    void on_bUpdate_clicked();
    void on_bFullCheck_clicked();
    void on_bStorageDetails_clicked();
    void on_lAccountImage_clicked();

    void onAnimationFinished();

protected:
    void changeEvent(QEvent * event);
    QString getFormatString();

private:
    Ui::SettingsDialog *ui;
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    bool syncsChanged;
    bool excludedNamesChanged;
    QStringList syncNames;
    QStringList languageCodes;
    bool proxyOnly;
    QFutureWatcher<long long> cacheSizeWatcher;
    QFutureWatcher<long long> remoteCacheSizeWatcher;
    MegaProgressDialog *proxyTestProgressDialog;
    AccountDetailsDialog *accountDetailsDialog;
    bool shouldClose;
    int modifyingSettings;
    long long cacheSize;
    long long remoteCacheSize;
    bool hasDefaultUploadOption;
    bool hasDefaultDownloadOption;
    bool hasUpperLimit;
    bool hasLowerLimit;
    long long upperLimit;
    long long lowerLimit;
    int upperLimitUnit;
    int lowerLimitUnit;
    bool sizeLimitsChanged;
    QButtonGroup downloadButtonGroup;
    QButtonGroup uploadButtonGroup;

#ifndef WIN32
    int folderPermissions;
    int filePermissions;
    bool permissionsChanged;
#endif
    int debugCounter;

#ifdef __APPLE__
    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
#endif

    void loadSyncSettings();
    void loadSizeLimits();
    bool saveSettings();
    void onCacheSizeAvailable();
    void onClearCache();
};

#endif // SETTINGSDIALOG_H
