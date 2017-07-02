#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QRect>
#include <QTimer>
#include <QHelpEvent>
#include <QToolTip>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QFileInfo>
#include "InfoDialog.h"
#include "ActiveTransfer.h"
#include "RecentFile.h"
#include "ui_InfoDialog.h"
#include "control/Utilities.h"
#include "MegaApplication.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);

    //Set window properties
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

#ifdef __APPLE__
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    //Initialize fields
    this->app = app;
    downloadSpeed = 0;
    uploadSpeed = 0;
    currentUpload = 0;
    currentDownload = 0;
    totalUploads = 0;
    totalDownloads = 0;
    activeDownloadState = activeUploadState = MegaTransfer::STATE_NONE;
    remainingDownloadBytes = remainingUploadBytes = 0;
    meanDownloadSpeed = meanUploadSpeed = 0;
    remainingUploads = remainingDownloads = 0;
    ui->lDownloads->setText(QString::fromAscii(""));
    ui->lUploads->setText(QString::fromAscii(""));
    indexing = false;
    waiting = false;
    syncsMenu = NULL;
    activeDownload = NULL;
    activeUpload = NULL;
    transferMenu = NULL;
    gWidget = NULL;

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
    ui->wTransfer1->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransfer1->hideTransfer();
    ui->wTransfer2->setType(MegaTransfer::TYPE_UPLOAD);
    ui->wTransfer2->hideTransfer();

    state = STATE_STARTING;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    scanningTimer.setSingleShot(false);
    scanningTimer.setInterval(60);
    scanningAnimationIndex = 1;
    connect(&scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));

    uploadsFinishedTimer.setSingleShot(true);
    uploadsFinishedTimer.setInterval(5000);
    connect(&uploadsFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllUploadsFinished()));

    downloadsFinishedTimer.setSingleShot(true);
    downloadsFinishedTimer.setInterval(5000);
    connect(&downloadsFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllDownloadsFinished()));

    transfersFinishedTimer.setSingleShot(true);
    transfersFinishedTimer.setInterval(5000);
    connect(&transfersFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllTransfersFinished()));

    recentFilesTimer.setSingleShot(true);
    recentFilesTimer.setInterval(200);
    connect(&recentFilesTimer, SIGNAL(timeout()), this, SLOT(onUpdateRecentFiles()));

    ui->wDownloadDesc->hide();
    ui->wUploadDesc->hide();
    ui->lBlockedItem->setText(QString::fromUtf8(""));

#ifdef __APPLE__
    arrow = new QPushButton(this);
    arrow->setIcon(QIcon(QString::fromAscii("://images/top_arrow.png")));
    arrow->setIconSize(QSize(22,11));
    arrow->setStyleSheet(QString::fromAscii("border: none; padding-bottom: -1px; "));
    arrow->resize(22,11);
    arrow->hide();
#endif

    if (gWidget)
    {
        gWidget->hideDownloads();
    }

    //Create the overlay widget with a semi-transparent background
    //that will be shown over the transfers when they are paused
    overlay = new QPushButton(this);
    overlay->setIcon(QIcon(QString::fromAscii("://images/tray_paused_large_ico.png")));
    overlay->setIconSize(QSize(64, 64));
    overlay->setStyleSheet(QString::fromAscii("background-color: rgba(247, 247, 247, 200); "
                                              "border: none; "));

#ifdef __APPLE__
    minHeightAnimation = new QPropertyAnimation();
    maxHeightAnimation = new QPropertyAnimation();
    animationGroup = new QParallelAnimationGroup();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    animationGroup->addAnimation(minHeightAnimation);
    animationGroup->addAnimation(maxHeightAnimation);
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
#endif

    ui->wTransfer1->hide();
    ui->wTransfer1->hide();
    overlay->resize(ui->wTransfers->minimumSize());
#ifdef __APPLE__
    overlay->move(1, 72);
#else
    overlay->move(2, 60);
    overlay->resize(overlay->width()-4, overlay->height());
#endif
    overlay->hide();
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));
    connect(ui->wTransfer1, SIGNAL(cancel(int, int)), this, SLOT(onTransfer1Cancel(int, int)));
    connect(ui->wTransfer2, SIGNAL(cancel(int, int)), this, SLOT(onTransfer2Cancel(int, int)));

#ifdef __APPLE__
    ui->wRecentlyUpdated->hide();
    ui->wRecent1->hide();
    ui->wRecent2->hide();
    ui->wRecent3->hide();
    setMinimumHeight(377);
    setMaximumHeight(377);
#endif

    if (preferences->logged())
    {
        setUsage();
        updateSyncsButton();
    }
    else
    {
        regenerateLayout();
        if (gWidget)
        {
            gWidget->hideDownloads();
        }
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
    delete gWidget;
    delete activeDownload;
    delete activeUpload;
}

void InfoDialog::setUsage()
{
    if (!preferences->totalStorage())
    {
        return;
    }

    int percentage = ceil((100 * preferences->usedStorage()) / (double)preferences->totalStorage());
    ui->pUsage->setProgress(preferences->cloudDriveStorage(), preferences->rubbishStorage(),
                            preferences->inShareStorage(), preferences->inboxStorage(),
                            preferences->totalStorage(), preferences->usedStorage());
    QString used = tr("%1 of %2").arg(QString::number(percentage).append(QString::fromAscii("%")))
            .arg(Utilities::getSizeString(preferences->totalStorage()));
    ui->lPercentageUsed->setText(used);
    ui->lTotalUsed->setText(tr("Usage: %1").arg(Utilities::getSizeString(preferences->usedStorage())));
}

void InfoDialog::setTransfer(MegaTransfer *transfer)
{
    if (!transfer)
    {
        return;
    }

    int type = transfer->getType();
    long long completedSize = transfer->getTransferredBytes();
    long long totalSize = transfer->getTotalBytes();
    long long meanSpeed = transfer->getMeanSpeed();

    ActiveTransfer *wTransfer = NULL;
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        activeDownloadState = transfer->getState();
        long long speed = megaApi->getCurrentDownloadSpeed();
        meanDownloadSpeed = meanSpeed;
        remainingDownloadBytes = totalSize - completedSize;
        if (speed || downloadSpeed < 0)
        {
            downloadSpeed = speed;
        }

        wTransfer = !preferences->logged() ? gWidget->getTransfer() : ui->wTransfer1;
        if (!activeDownload || activeDownload->getTag() != transfer->getTag())
        {
            delete activeDownload;
            activeDownload = transfer->copy();
            wTransfer->setFileName(QString::fromUtf8(transfer->getFileName()));
        }
    }
    else
    {
        activeUploadState = transfer->getState();
        long long speed = megaApi->getCurrentUploadSpeed();
        remainingUploadBytes = totalSize - completedSize;
        meanUploadSpeed = meanSpeed;
        if (speed || uploadSpeed < 0)
        {
            uploadSpeed = speed;
        }

        wTransfer = ui->wTransfer2;
        if (!activeUpload || activeUpload->getTag() != transfer->getTag())
        {
            delete activeUpload;
            activeUpload = transfer->copy();
            wTransfer->setFileName(QString::fromUtf8(transfer->getFileName()));
        }
    }
    wTransfer->setProgress(completedSize, totalSize, !transfer->isSyncTransfer());
}

void InfoDialog::addRecentFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey)
{
    RecentFileInfo info1 = ui->wRecent1->getFileInfo();
    RecentFileInfo info2 = ui->wRecent2->getFileInfo();
    ui->wRecent3->setFileInfo(info2);
    ui->wRecent2->setFileInfo(info1);
    ui->wRecent1->setFile(fileName, fileHandle, localPath, nodeKey, QDateTime::currentDateTime().toMSecsSinceEpoch());

#ifdef __APPLE__
    if (!ui->wRecentlyUpdated->isVisible())
    {
        showRecentList();
    }
#endif
    updateRecentFiles();
}

void InfoDialog::clearRecentFiles()
{
    ui->wRecent1->clear();
    ui->wRecent2->clear();
    ui->wRecent3->clear();
    updateRecentFiles();

#ifdef __APPLE__
    ui->wRecentlyUpdated->hide();
    ui->wRecent1->hide();
    ui->wRecent2->hide();
    ui->wRecent3->hide();
    setMinimumHeight(377);
    setMaximumHeight(377);
#endif
}

void InfoDialog::updateTransfers()
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();
    totalUploads = megaApi->getTotalUploads();
    totalDownloads = megaApi->getTotalDownloads();

    if (totalUploads < remainingUploads)
    {
        totalUploads = remainingUploads;
    }

    if (totalDownloads < remainingDownloads)
    {
        totalDownloads = remainingDownloads;
    }

    currentDownload = totalDownloads - remainingDownloads + 1;
    currentUpload = totalUploads - remainingUploads + 1;

    if (isVisible())
    {
        if (remainingDownloads)
        {
            int totalRemainingSeconds = meanDownloadSpeed ? remainingDownloadBytes / meanDownloadSpeed : 0;
            int remainingHours = totalRemainingSeconds/3600;
            if ((remainingHours<0) || (remainingHours>99))
            {
                totalRemainingSeconds = 0;
            }

            int remainingMinutes = (totalRemainingSeconds%3600)/60;
            int remainingSeconds =  (totalRemainingSeconds%60);
            QString remainingTime;
            if (totalRemainingSeconds)
            {
                remainingTime = QString::fromAscii("%1:%2:%3").arg(remainingHours, 2, 10, QChar::fromAscii('0'))
                    .arg(remainingMinutes, 2, 10, QChar::fromAscii('0'))
                    .arg(remainingSeconds, 2, 10, QChar::fromAscii('0'));
            }
            else
            {
                remainingTime = QString::fromAscii("--:--:--");
            }

            !preferences->logged() ? gWidget->setRemainingTime(remainingTime)
                      : ui->lRemainingTimeD->setText(remainingTime);
            ui->wDownloadDesc->show();
            QString fullPattern = QString::fromAscii("<span style=\"color: rgb(120, 178, 66); \">%1</span>%2");
            QString operation = tr("Downloading ");
            if (operation.size() && operation[operation.size() - 1] != QChar::fromAscii(' '))
            {
                operation.append(QChar::fromAscii(' '));
            }

            QString pattern(tr("%1 of %2 (%3/s)"));
            QString pausedPattern(tr("%1 of %2 (paused)"));
            QString invalidSpeedPattern(tr("%1 of %2"));
            QString downloadString;


            if (activeDownloadState == MegaTransfer::STATE_PAUSED || preferences->getDownloadsPaused())
            {
                downloadString = pausedPattern.arg(currentDownload).arg(totalDownloads);
            }
            else
            {
                if (downloadSpeed >= 20000)
                {
                    downloadString = pattern.arg(currentDownload)
                            .arg(totalDownloads)
                            .arg(Utilities::getSizeString(downloadSpeed));
                }
                else if (downloadSpeed >= 0)
                {
                    downloadString = invalidSpeedPattern.arg(currentDownload).arg(totalDownloads);
                }
                else
                {
                    downloadString = pausedPattern.arg(currentDownload).arg(totalDownloads);
                }
            }

            if (preferences->logged())
            {
                ui->lDownloads->setText(fullPattern.arg(operation).arg(downloadString));
                if (!ui->wTransfer1->isActive())
                {
                    ui->wDownloadDesc->hide();
                }
                else
                {
                    ui->wDownloadDesc->show();
                }
            }
            else
            {
                gWidget->setDownloadLabel(fullPattern.arg(operation).arg(downloadString));
                if (!gWidget->getTransfer()->isActive())
                {
                    gWidget->hideDownloads();
                }
                else
                {
                    gWidget->showDownloads();
                }
            }
        }

        if (remainingUploads)
        {
            int totalRemainingSeconds = meanUploadSpeed ? remainingUploadBytes / meanUploadSpeed : 0;
            int remainingHours = totalRemainingSeconds/3600;
            if ((remainingHours < 0) || (remainingHours > 99))
            {
                totalRemainingSeconds = 0;
            }

            int remainingMinutes = (totalRemainingSeconds%3600)/60;
            int remainingSeconds =  (totalRemainingSeconds%60);
            QString remainingTime;
            if (totalRemainingSeconds)
            {
                remainingTime = QString::fromAscii("%1:%2:%3").arg(remainingHours, 2, 10, QChar::fromAscii('0'))
                    .arg(remainingMinutes, 2, 10, QChar::fromAscii('0'))
                    .arg(remainingSeconds, 2, 10, QChar::fromAscii('0'));
            }
            else
            {
                remainingTime = QString::fromAscii("--:--:--");
            }

            ui->lRemainingTimeU->setText(remainingTime);
            ui->wUploadDesc->show();
            QString fullPattern = QString::fromAscii("<span style=\"color: rgb(119, 185, 217); \">%1</span>%2");
            QString operation = tr("Uploading ");
            if (operation.size() && operation[operation.size() - 1] != QChar::fromAscii(' '))
            {
                operation.append(QChar::fromAscii(' '));
            }

            QString pattern(tr("%1 of %2 (%3/s)"));
            QString pausedPattern(tr("%1 of %2 (paused)"));
            QString invalidSpeedPattern(tr("%1 of %2"));
            QString uploadString;

            if (activeUploadState == MegaTransfer::STATE_PAUSED || preferences->getUploadsPaused())
            {
                uploadString = pausedPattern.arg(currentUpload).arg(totalUploads);
            }
            else
            {
                if (uploadSpeed >= 20000)
                {
                    uploadString = pattern.arg(currentUpload).arg(totalUploads).arg(Utilities::getSizeString(uploadSpeed));
                }
                else if (uploadSpeed >= 0)
                {
                    uploadString = invalidSpeedPattern.arg(currentUpload).arg(totalUploads);
                }
                else
                {
                    uploadString = pausedPattern.arg(currentUpload).arg(totalUploads);
                }
            }

            ui->lUploads->setText(fullPattern.arg(operation).arg(uploadString));

            if (!ui->wTransfer2->isActive())
            {
                ui->wUploadDesc->hide();
            }
            else
            {
                ui->wUploadDesc->show();
            }
        }

        if (remainingUploads || remainingDownloads)
        {
            if (!preferences->logged() && gWidget->getTransfer()->isActive())
            {
                gWidget->setIdleState(false);
            }
            else if (ui->wTransfer1->isActive() || ui->wTransfer2->isActive())
            {
                ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
            }
        }
    }
}

void InfoDialog::transferFinished(int error)
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();

    if (!remainingDownloads && ui->wTransfer1->isActive())
    {
        if (!downloadsFinishedTimer.isActive())
        {
            if (!error)
            {
                downloadsFinishedTimer.start();
            }
            else
            {
                onAllDownloadsFinished();
            }
        }
    }
    else
    {
        downloadsFinishedTimer.stop();
    }

    if (!remainingUploads && ui->wTransfer2->isActive())
    {
        if (!uploadsFinishedTimer.isActive())
        {
            if (!error)
            {
                uploadsFinishedTimer.start();
            }
            else
            {
                onAllUploadsFinished();
            }
        }
    }
    else
    {
        uploadsFinishedTimer.stop();
    }

    if (!remainingDownloads
            && !remainingUploads
            &&  (ui->sActiveTransfers->currentWidget() != ui->pUpdated
                 || (!preferences->logged() && !gWidget->idleState())))
    {
        if (!transfersFinishedTimer.isActive())
        {
            if (!error)
            {
                transfersFinishedTimer.start();
            }
            else
            {
                onAllTransfersFinished();
            }
        }
    }
    else
    {
        transfersFinishedTimer.stop();
    }
}

void InfoDialog::updateSyncsButton()
{
    int num = preferences->getNumSyncedFolders();
    long long firstSyncHandle = mega::INVALID_HANDLE;
    if (num == 1)
    {
        firstSyncHandle = preferences->getMegaFolderHandle(0);
    }

    MegaNode *rootNode = megaApi->getRootNode();
    if (!rootNode)
    {
        preferences->setCrashed(true);
        ui->bSyncFolder->setText(QString::fromAscii("MEGA"));
        return;
    }
    long long rootHandle = rootNode->getHandle();

    if ((num == 1) && (firstSyncHandle == rootHandle))
    {
        ui->bSyncFolder->setText(QString::fromAscii("MEGA"));
    }
    else
    {
        ui->bSyncFolder->setText(tr("Syncs"));
    }

    delete rootNode;
}

void InfoDialog::setIndexing(bool indexing)
{
    this->indexing = indexing;
}

void InfoDialog::setWaiting(bool waiting)
{
    this->waiting = waiting;
}

void InfoDialog::increaseUsedStorage(long long bytes, bool isInShare)
{
    if (isInShare)
    {
        preferences->setInShareStorage(preferences->inShareStorage() + bytes);
        preferences->setInShareFiles(preferences->inShareFiles()+1);
    }
    else
    {
        preferences->setCloudDriveStorage(preferences->cloudDriveStorage() + bytes);
        preferences->setCloudDriveFiles(preferences->cloudDriveFiles()+1);
    }

    preferences->setUsedStorage(preferences->usedStorage() + bytes);
    setUsage();
}

void InfoDialog::updateState()
{
    updateTransfers();
    if (ui->bPause->isChecked())
    {
        if (!preferences->logged())
        {
            if (gWidget)
            {
                if (!gWidget->idleState())
                {
                    gWidget->setPauseState(true);
                }
                else
                {
                    gWidget->setPauseState(false);
                }
            }
            return;
        }

        downloadSpeed = -1;
        uploadSpeed = -1;
        if (state != STATE_PAUSED)
        {
            state = STATE_PAUSED;
            if (scanningTimer.isActive())
            {
                scanningTimer.stop();
            }

            ui->lSyncUpdated->setText(tr("File transfers paused"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/tray_paused_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);

            ui->label->setIcon(icon);
            ui->label->setIconSize(QSize(64, 64));
        }

        if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
        {
            overlay->setVisible(true);
        }
        else
        {
            overlay->setVisible(false);
        }
    }
    else
    {
        if (!preferences->logged())
        {
            if (gWidget)
            {
                gWidget->setPauseState(false);
                if (!gWidget->getTransfer()->isActive())
                {
                    gWidget->setIdleState(true);
                }
            }
            return;
        }
        overlay->setVisible(false);
        if (downloadSpeed < 0 && uploadSpeed < 0)
        {
            downloadSpeed = 0;
            uploadSpeed = 0;
        }

        if (!waiting)
        {
            ui->lBlockedItem->setText(QString::fromUtf8(""));
        }

        if (waiting)
        {
            const char *blockedPath = megaApi->getBlockedPath();
            if (blockedPath)
            {
                QFileInfo fileBlocked (QString::fromUtf8(blockedPath));
                ui->lBlockedItem->setToolTip(fileBlocked.absoluteFilePath());
                ui->lBlockedItem->setAlignment(Qt::AlignLeft);
                ui->lBlockedItem->setText(tr("Blocked file: %1").arg(QString::fromUtf8("<a style=\" font-size: 12px;\" href=\"local://#%1\">%2</a>")
                                                               .arg(fileBlocked.absoluteFilePath())
                                                               .arg(fileBlocked.fileName())));
                delete blockedPath;
            }
            else if (megaApi->areServersBusy())
            {
                ui->lBlockedItem->setText(tr("Servers are too busy. Please wait..."));
                ui->lBlockedItem->setAlignment(Qt::AlignCenter);
            }
            else
            {
                ui->lBlockedItem->setText(QString::fromUtf8(""));
            }

            if (state != STATE_WAITING)
            {
                state = STATE_WAITING;
                if (scanningTimer.isActive())
                {
                    scanningTimer.stop();
                }

                ui->lSyncUpdated->setText(tr("MEGAsync is waiting"));
                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/tray_scanning_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);

                ui->label->setIcon(icon);
                ui->label->setIconSize(QSize(64, 64));
            }
        }
        else if (indexing)
        {
            if (state != STATE_INDEXING)
            {
                state = STATE_INDEXING;
                if (!scanningTimer.isActive())
                {
                    scanningAnimationIndex = 1;
                    scanningTimer.start();
                }

                ui->lSyncUpdated->setText(tr("MEGAsync is scanning"));

                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/tray_scanning_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->label->setIcon(icon);
                ui->label->setIconSize(QSize(64, 64));
            }
        }
        else
        {
            if (state != STATE_UPDATED)
            {
                state = STATE_UPDATED;
                if (scanningTimer.isActive())
                {
                    scanningTimer.stop();
                }

                ui->lSyncUpdated->setText(tr("MEGAsync is up to date"));
                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/tray_updated_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->label->setIcon(icon);
                ui->label->setIconSize(QSize(64, 64));
            }
        }
    }
}

#ifdef __APPLE__
void InfoDialog::showRecentlyUpdated(bool show)
{
    ui->wRecent->setVisible(show);
    if (!show)
    {
        this->setMinimumHeight(377);
        this->setMaximumHeight(377);
    }
    else
    {
        on_cRecentlyUpdated_stateChanged(0);
    }
}
#endif

void InfoDialog::closeSyncsMenu()
{
#ifdef __APPLE__
    if (syncsMenu && syncsMenu->isVisible())
    {
        syncsMenu->close();
    }

    if (transferMenu && transferMenu->isVisible())
    {
        transferMenu->close();
    }

    ui->wRecent1->closeMenu();
    ui->wRecent2->closeMenu();
    ui->wRecent3->closeMenu();
#endif
}

void InfoDialog::setPaused(bool paused)
{
    ui->bPause->setChecked(paused);
    ui->bPause->setEnabled(true);
}

void InfoDialog::addSync()
{
    addSync(INVALID_HANDLE);
}

void InfoDialog::onTransfer1Cancel(int x, int y)
{
    if (transferMenu)
    {
#ifdef __APPLE__
        transferMenu->close();
        return;
#else
        transferMenu->deleteLater();
#endif
    }

    transferMenu = new QMenu();
#ifndef __APPLE__
    transferMenu->setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif

    if (activeDownloadState == MegaTransfer::STATE_PAUSED)
    {
        transferMenu->addAction(tr("Resume download"), this, SLOT(downloadState()));
    }
    transferMenu->addAction(megaApi->areTransfersPaused(MegaTransfer::TYPE_DOWNLOAD) ? tr("Resume downloads") : tr("Pause downloads"), this, SLOT(globalDownloadState()));
    transferMenu->addAction(tr("Cancel download"), this, SLOT(cancelCurrentDownload()));
    transferMenu->addAction(tr("Cancel all downloads"), this, SLOT(cancelAllDownloads()));

#ifdef __APPLE__
    transferMenu->exec(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }

    transferMenu->deleteLater();
    transferMenu = NULL;
#else
    transferMenu->popup(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
#endif
}

void InfoDialog::onTransfer2Cancel(int x, int y)
{
    if (transferMenu)
    {
#ifdef __APPLE__
        transferMenu->close();
        return;
#else
        transferMenu->deleteLater();
#endif
    }

    transferMenu = new QMenu();
#ifndef __APPLE__
    transferMenu->setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif

    if (activeUploadState == MegaTransfer::STATE_PAUSED)
    {
        transferMenu->addAction(tr("Resume upload"), this, SLOT(uploadState()));
    }
    transferMenu->addAction(megaApi->areTransfersPaused(MegaTransfer::TYPE_UPLOAD) ? tr("Resume uploads") : tr("Pause uploads"), this, SLOT(globalUploadState()));
    transferMenu->addAction(tr("Cancel upload"), this, SLOT(cancelCurrentUpload()));
    transferMenu->addAction(tr("Cancel all uploads"), this, SLOT(cancelAllUploads()));

#ifdef __APPLE__
    transferMenu->exec(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }

    transferMenu->deleteLater();
    transferMenu = NULL;
#else
    transferMenu->popup(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
#endif
}

void InfoDialog::globalDownloadState()
{
    if (!activeDownload)
    {
        return;
    }

    if (megaApi->areTransfersPaused(MegaTransfer::TYPE_DOWNLOAD))
    {
        megaApi->pauseTransfers(false, MegaTransfer::TYPE_DOWNLOAD);
    }
    else
    {
        megaApi->pauseTransfers(true, MegaTransfer::TYPE_DOWNLOAD);
    }
}

void InfoDialog::downloadState()
{
    if (!activeDownload)
    {
        return;
    }

    if (activeDownloadState == MegaTransfer::STATE_PAUSED)
    {
        megaApi->pauseTransfer(activeDownload, false);
    }
    else
    {
        megaApi->pauseTransfer(activeDownload, true);
    }
}

void InfoDialog::globalUploadState()
{
    if (!activeUpload)
    {
        return;
    }

    if (megaApi->areTransfersPaused(MegaTransfer::TYPE_UPLOAD))
    {
        megaApi->pauseTransfers(false, MegaTransfer::TYPE_UPLOAD);
    }
    else
    {
        megaApi->pauseTransfers(true, MegaTransfer::TYPE_UPLOAD);
    }
}

void InfoDialog::uploadState()
{
    if (!activeUpload)
    {
        return;
    }

    if (activeUploadState == MegaTransfer::STATE_PAUSED)
    {
        megaApi->pauseTransfer(activeUpload, false);
    }
    else
    {
        megaApi->pauseTransfer(activeUpload, true);
    }
}

void InfoDialog::cancelAllUploads()
{
    megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
}

void InfoDialog::cancelAllDownloads()
{
    megaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
}

void InfoDialog::cancelCurrentUpload()
{
    megaApi->cancelTransfer(activeUpload);
}

void InfoDialog::cancelCurrentDownload()
{
    megaApi->cancelTransfer(activeDownload);
}

void InfoDialog::onAllUploadsFinished()
{
    remainingUploads = megaApi->getNumPendingUploads();
    if (!remainingUploads)
    {
        ui->wTransfer2->hideTransfer();
        ui->lUploads->setText(QString::fromAscii(""));
        ui->wUploadDesc->hide();
        uploadSpeed = 0;
        currentUpload = 0;
        totalUploads = 0;
        remainingUploadBytes = 0;
        meanUploadSpeed = 0;
        megaApi->resetTotalUploads();
    }
}

void InfoDialog::onAllDownloadsFinished()
{
    remainingDownloads = megaApi->getNumPendingDownloads();
    if (!remainingDownloads)
    {
        if (!preferences->logged())
        {
            gWidget->getTransfer()->hideTransfer();
            gWidget->setDownloadLabel(QString::fromAscii(""));
            gWidget->hideDownloads();
        }
        else
        {
            ui->wTransfer1->hideTransfer();
            ui->lDownloads->setText(QString::fromAscii(""));
            ui->wDownloadDesc->hide();
        }
        downloadSpeed = 0;
        currentDownload = 0;
        totalDownloads = 0;
        remainingDownloadBytes = 0;
        meanDownloadSpeed = 0;
        megaApi->resetTotalDownloads();
    }
}

void InfoDialog::onAllTransfersFinished()
{
    if (!remainingDownloads && !remainingUploads)
    {
        if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
        {
            ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
        }
        else if (!preferences->logged() && !gWidget->idleState())
        {
            gWidget->setIdleState(true);
        }

        if (preferences->logged())
        {
            app->updateUserStats();
        }

        app->showNotificationMessage(tr("All transfers have been completed"));
    }
}

void InfoDialog::onUpdateRecentFiles()
{
    ui->wRecent1->updateWidget();
    ui->wRecent2->updateWidget();
    ui->wRecent3->updateWidget();
}

void InfoDialog::on_bSettings_clicked()
{   
    QPoint p = ui->bSettings->mapToGlobal(QPoint(ui->bSettings->width()-6, ui->bSettings->height()));

#ifdef __APPLE__
    QPointer<InfoDialog> iod = this;
#endif

    app->showTrayMenu(&p);

#ifdef __APPLE__
    if (!iod)
    {
        return;
    }

    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }
#endif
}

void InfoDialog::on_bTransferManager_clicked()
{
    app->transferManagerActionClicked();
}

void InfoDialog::on_bOfficialWeb_clicked()
{
    QString webUrl = QString::fromAscii("https://mega.nz/");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(webUrl));
}

void InfoDialog::on_bSyncFolder_clicked()
{
    int num = preferences->getNumSyncedFolders();

    MegaNode *rootNode = megaApi->getRootNode();
    if (!rootNode)
    {
        preferences->setCrashed(true);
        return;
    }

    if ((num == 1) && (preferences->getMegaFolderHandle(0) == rootNode->getHandle()))
    {
        openFolder(preferences->getLocalFolder(0));
    }
    else
    {
        syncsMenu = new QMenu();
        #ifndef __APPLE__
            syncsMenu->setStyleSheet(QString::fromAscii(
                    "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
                    "QMenu::item {background-color: white; color: black;} "
                    "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
        #else
            syncsMenu->setStyleSheet(QString::fromAscii("QMenu {padding-left: -10px; padding-top: 4px; } "
                    "QMenu::separator {height: 8px; margin: 0px; }"));
        #endif
        QAction *addSyncAction = syncsMenu->addAction(tr("Add Sync"), this, SLOT(addSync()));
#ifdef __APPLE__
        addSyncAction->setIcon(QIcon(QString::fromAscii("://images/tray_add_sync_ico.png")));
#else
        addSyncAction->setIcon(QIcon(QString::fromAscii("://images/tray_add_sync_ico2.png")));
#endif
        addSyncAction->setIconVisibleInMenu(true);
        syncsMenu->addSeparator();

        QSignalMapper *menuSignalMapper = new QSignalMapper();
        connect(menuSignalMapper, SIGNAL(mapped(QString)), this, SLOT(openFolder(QString)));
        
        int activeFolders = 0;
        for (int i = 0; i < num; i++)
        {
            if (!preferences->isFolderActive(i))
            {
                continue;
            }

            activeFolders++;
            QAction *action = syncsMenu->addAction(preferences->getSyncName(i), menuSignalMapper, SLOT(map()));
#ifdef __APPLE__
            action->setIcon(QIcon(QString::fromAscii("://images/tray_sync_ico.png")));
#else
            action->setIcon(QIcon(QString::fromAscii("://images/tray_sync_ico2.png")));
#endif
            action->setIconVisibleInMenu(true);
            menuSignalMapper->setMapping(action, preferences->getLocalFolder(i));
        }

        connect(syncsMenu, SIGNAL(aboutToHide()), syncsMenu, SLOT(deleteLater()));
        connect(syncsMenu, SIGNAL(destroyed(QObject*)), menuSignalMapper, SLOT(deleteLater()));

#ifdef __APPLE__
        syncsMenu->exec(this->mapToGlobal(QPoint(20, this->height() - (activeFolders + 1) * 28 - (activeFolders ? 16 : 8))));
        if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
        {
            this->hide();
        }
#else
        syncsMenu->popup(ui->bSyncFolder->mapToGlobal(QPoint(0, -activeFolders*35)));
#endif
        syncsMenu = NULL;
    }
    delete rootNode;
}

void InfoDialog::openFolder(QString path)
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(path));
}

void InfoDialog::updateRecentFiles()
{
    if (!recentFilesTimer.isActive())
    {
        recentFilesTimer.start();
    }
}

void InfoDialog::disableGetLink(bool disable)
{
    ui->wRecent1->disableGetLink(disable);
    ui->wRecent2->disableGetLink(disable);
    ui->wRecent3->disableGetLink(disable);
}

void InfoDialog::addSync(MegaHandle h)
{
    static BindFolderDialog *dialog = NULL;
    if (dialog)
    {
        if (h != mega::INVALID_HANDLE)
        {
            dialog->setMegaFolder(h);
        }

        dialog->activateWindow();
        dialog->raise();
        dialog->setFocus();
        return;
    }

    dialog = new BindFolderDialog(app);
    if (h != mega::INVALID_HANDLE)
    {
        dialog->setMegaFolder(h);
    }

    int result = dialog->exec();
    if (result != QDialog::Accepted)
    {
        delete dialog;
        dialog = NULL;
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    MegaHandle handle = dialog->getMegaFolder();
    MegaNode *node = megaApi->getNodeByHandle(handle);
    QString syncName = dialog->getSyncName();
    delete dialog;
    dialog = NULL;
    if (!localFolderPath.length() || !node)
    {
        delete node;
        return;
    }

   const char *nPath = megaApi->getNodePath(node);
   if (!nPath)
   {
       delete node;
       return;
   }

   preferences->addSyncedFolder(localFolderPath, QString::fromUtf8(nPath), handle, syncName);
   delete [] nPath;
   megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
   delete node;
   updateSyncsButton();
}

#ifdef __APPLE__
void InfoDialog::moveArrow(QPoint p)
{
    arrow->move(p.x()-(arrow->width()/2+1), 2);
    arrow->show();
}
#endif

void InfoDialog::on_bPause_clicked()
{
    app->pauseTransfers(ui->bPause->isChecked());
}

void InfoDialog::onOverlayClicked()
{
    ui->bPause->setChecked(false);
    on_bPause_clicked();
}

void InfoDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if (preferences->logged())
        {
            if (preferences->totalStorage())
            {
                setUsage();
            }
            updateSyncsButton();
            state = STATE_STARTING;
            updateState();
        }
    }
    QDialog::changeEvent(event);
}

void InfoDialog::regenerateLayout()
{
    static bool loggedInMode = true;
    bool logged = preferences->logged();

    if (loggedInMode == logged)
    {
        return;
    }
    loggedInMode = logged;

    QLayout *dialogLayout = layout();
    if (!loggedInMode)
    {
        if (!gWidget)
        {
            gWidget = new GuestWidget();
            connect(gWidget, SIGNAL(actionButtonClicked(int)), this, SLOT(onUserAction(int)));
            connect(gWidget, SIGNAL(cancelCurrentDownload()), this, SLOT(cancelCurrentDownload()));
            connect(gWidget, SIGNAL(cancelAllDownloads()), this, SLOT(cancelAllDownloads()));
            connect(gWidget, SIGNAL(pauseClicked()), this, SLOT(onOverlayClicked()));
        }

#ifndef __APPLE__
        ui->wRecent->hide();
        ui->wRecentlyUpdated->hide();
        ui->wRecent1->hide();
        ui->wRecent2->hide();
        ui->wRecent3->hide();
        setMinimumHeight(365);
        setMaximumHeight(365);
#endif

        ui->bPause->setVisible(false);
        ui->bTransferManager->setVisible(false);
        ui->bSyncFolder->setVisible(false);
        dialogLayout->removeWidget(ui->sActiveTransfers);
        ui->sActiveTransfers->setVisible(false);
        dialogLayout->removeWidget(ui->wUsage);
        ui->wUsage->setVisible(false);
        dialogLayout->addWidget(gWidget);
        gWidget->setVisible(true);

        ((QVBoxLayout *)dialogLayout)->insertWidget(dialogLayout->count(), ui->wRecent);
        ((QVBoxLayout *)dialogLayout)->insertWidget(dialogLayout->count(), ui->wBottom);

        overlay->setVisible(false);
    }
    else
    {
#ifndef __APPLE__
        ui->wRecent->show();
        ui->wRecentlyUpdated->show();
        ui->wRecent1->show();
        ui->wRecent2->show();
        ui->wRecent3->show();
        setMaximumHeight(539);
        setMinimumHeight(539);
#endif

        ui->bPause->setVisible(true);
        ui->bTransferManager->setVisible(true);
        ui->bSyncFolder->setVisible(true);
        dialogLayout->removeWidget(gWidget);
        gWidget->setVisible(false);
        dialogLayout->addWidget(ui->sActiveTransfers);
        ui->sActiveTransfers->setVisible(true);

        ((QVBoxLayout *)dialogLayout)->insertWidget(dialogLayout->count(), ui->wRecent);
        dialogLayout->addWidget(ui->wUsage);
        ui->wUsage->setVisible(true);
        ((QVBoxLayout *)dialogLayout)->insertWidget(dialogLayout->count(), ui->wBottom);
    }

    if (activeDownload)
    {
        ActiveTransfer *wTransfer = !logged ? gWidget->getTransfer() : ui->wTransfer1;
        wTransfer->setFileName(QString::fromUtf8(activeDownload->getFileName()));
        wTransfer->setProgress(activeDownload->getTotalBytes() - remainingDownloadBytes,
                               activeDownload->getTotalBytes(),
                               !activeDownload->isSyncTransfer());
    }

    updateTransfers();
    app->onGlobalSyncStateChanged(NULL);
}

void InfoDialog::onUserAction(int action)
{
    app->userAction(action);
}
void InfoDialog::scanningAnimationStep()
{
    scanningAnimationIndex = scanningAnimationIndex%18;
    scanningAnimationIndex++;
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/scanning_anime")+
                 QString::number(scanningAnimationIndex) + QString::fromUtf8(".png") , QSize(), QIcon::Normal, QIcon::Off);

    ui->label->setIcon(icon);
    ui->label->setIconSize(QSize(64, 64));
}

#ifdef __APPLE__
void InfoDialog::paintEvent( QPaintEvent * e)
{
    QDialog::paintEvent(e);
    QPainter p( this );
    p.setCompositionMode( QPainter::CompositionMode_Clear);
    p.fillRect( ui->wArrow->rect(), Qt::transparent );
}

void InfoDialog::hideEvent(QHideEvent *event)
{
    arrow->hide();
    QDialog::hideEvent(event);
}

void InfoDialog::on_cRecentlyUpdated_stateChanged(int arg1)
{
    ui->wRecent1->hide();
    ui->wRecent2->hide();
    ui->wRecent3->hide();
    ui->cRecentlyUpdated->setEnabled(false);

    if (ui->cRecentlyUpdated->isChecked())
    {
        minHeightAnimation->setTargetObject(this);
        maxHeightAnimation->setTargetObject(this);
        minHeightAnimation->setPropertyName("minimumHeight");
        maxHeightAnimation->setPropertyName("maximumHeight");
        minHeightAnimation->setStartValue(minimumHeight());
        maxHeightAnimation->setStartValue(maximumHeight());
        minHeightAnimation->setEndValue(408);
        maxHeightAnimation->setEndValue(408);
        minHeightAnimation->setDuration(150);
        maxHeightAnimation->setDuration(150);
        animationGroup->start();
    }
    else
    {
        /*minHeightAnimation->setTargetObject(this);
        maxHeightAnimation->setTargetObject(this);
        minHeightAnimation->setPropertyName("minimumHeight");
        maxHeightAnimation->setPropertyName("maximumHeight");
        minHeightAnimation->setStartValue(minimumHeight());
        maxHeightAnimation->setStartValue(maximumHeight());
        minHeightAnimation->setEndValue(552);
        maxHeightAnimation->setEndValue(552);
        minHeightAnimation->setDuration(150);
        maxHeightAnimation->setDuration(150);
        animationGroup->start();*/

        //this->hide();
        this->setMaximumHeight(552);
        this->setMinimumHeight(552);
        onAnimationFinished();
        //this->show();
    }
}

void InfoDialog::onAnimationFinished()
{
    if (this->minimumHeight() == 552)
    {
        ui->wRecent1->show();
        ui->wRecent2->show();
        ui->wRecent3->show();
    }

    ui->lRecentlyUpdated->show();
    ui->cRecentlyUpdated->show();
    ui->wRecentlyUpdated->show();
    ui->cRecentlyUpdated->setEnabled(true);
}


void InfoDialog::showRecentList()
{
    on_cRecentlyUpdated_stateChanged(0);
}
#endif

#ifndef Q_OS_LINUX
void InfoDialog::on_bOfficialWebIcon_clicked()
{
    on_bOfficialWeb_clicked();
}
#endif
