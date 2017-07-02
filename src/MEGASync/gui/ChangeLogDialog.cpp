#include "ChangeLogDialog.h"
#include "ui_ChangeLogDialog.h"
#include <QtCore>
#include <QDesktopServices>
#include <QString>
#include <QUrl>
#include <QScrollBar>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

ChangeLogDialog::ChangeLogDialog(QString version, QString SDKversion, QString changeLog, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeLogDialog)
{
    ui->setupUi(this);
#ifdef Q_OS_LINUX
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
#endif

    ui->tChangelog->verticalScrollBar()->setStyleSheet(
                QString::fromUtf8("QScrollBar:vertical {"
                           " background: #f6f6f6;"
                           " width: 15px;"
                           " border-left: 1px solid #E4E4E4;"
                          "}"
                          "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical {"
                           " border: none;"
                           " background: none;"
                          "}"
                          "QScrollBar::handle:vertical {"
                           " background: #c0c0c0;"
                           " min-height: 20px;"
                           " border-radius: 4px;"
                           " margin: 3px 3px 3px 3px;"
                          "}"
                 ""));

    ui->lCopyright->setText(ui->lCopyright->text().arg(QDate::currentDate().year()));
    ui->tChangelog->document()->setDocumentMargin(16.0);
    ui->lVersion->setText(version);
    ui->lSDKVersion->setText(QString::fromAscii(" (") + SDKversion + QString::fromAscii(")"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setChangeLogNotes(changeLog);
}

ChangeLogDialog::~ChangeLogDialog()
{
    delete ui;
}

void ChangeLogDialog::setChangeLogNotes(QString notes)
{
    ui->tChangelog->setHtml(QString::fromUtf8("<p style=\"line-height: 119%;\"><span style=\"margin: 16px; font-family: Helvetica; font-size:11px; color: #333333;\">") +
                             tr("New in this version:") +
                             QString::fromUtf8("</span></p>") +
                             QString::fromUtf8("<p style=\" line-height: 146%;\"><span style=\"font-family: Helvetica; font-size:11px; color: #666666;\">") +
                             notes.replace(QString::fromUtf8("\n"), QString::fromUtf8("<br>")) +
                             QString::fromUtf8("</span></p>"));
}

void ChangeLogDialog::on_bTerms_clicked()
{
    QString temsUrl = QString::fromAscii("https://mega.nz/#terms");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(temsUrl));
}

void ChangeLogDialog::on_bPolicy_clicked()
{
    QString policyUrl = QString::fromAscii("https://mega.nz/#privacy");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(policyUrl));
}

void ChangeLogDialog::on_bAck_clicked()
{
    QString ackUrl = QString::fromAscii("https://github.com/meganz/MEGAsync/blob/master/CREDITS.md");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(ackUrl));
}

void ChangeLogDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setChangeLogNotes(Preferences::CHANGELOG);
    }
    QDialog::changeEvent(event);
}
