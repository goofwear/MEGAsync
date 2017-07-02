#include "Utilities.h"
#include "control/Preferences.h"

#include <QApplication>
#include <QImageReader>
#include <QDirIterator>
#include <QDesktopServices>
#include <QTextStream>
#include <QDateTime>
#include <iostream>

#ifndef WIN32
#include "megaapi.h"
#include <utime.h>
#endif

using namespace std;

QHash<QString, QString> Utilities::extensionIcons;
QHash<QString, QString> Utilities::languageNames;

void Utilities::initializeExtensions()
{
    extensionIcons[QString::fromAscii("3ds")] = extensionIcons[QString::fromAscii("3dm")]  = extensionIcons[QString::fromAscii("max")] =
                            extensionIcons[QString::fromAscii("obj")]  = QString::fromAscii("3D.png");

    extensionIcons[QString::fromAscii("aep")] = extensionIcons[QString::fromAscii("aet")]  = QString::fromAscii("aftereffects.png");

    extensionIcons[QString::fromAscii("mp3")] = extensionIcons[QString::fromAscii("wav")]  = extensionIcons[QString::fromAscii("3ga")]  =
                            extensionIcons[QString::fromAscii("aif")]  = extensionIcons[QString::fromAscii("aiff")] =
                            extensionIcons[QString::fromAscii("flac")] = extensionIcons[QString::fromAscii("iff")]  =
                            extensionIcons[QString::fromAscii("m4a")]  = extensionIcons[QString::fromAscii("wma")]  =  QString::fromAscii("audio.png");

    extensionIcons[QString::fromAscii("dxf")] = extensionIcons[QString::fromAscii("dwg")] =  QString::fromAscii("cad.png");


    extensionIcons[QString::fromAscii("zip")] = extensionIcons[QString::fromAscii("rar")] = extensionIcons[QString::fromAscii("tgz")]  =
                            extensionIcons[QString::fromAscii("gz")]  = extensionIcons[QString::fromAscii("bz2")]  =
                            extensionIcons[QString::fromAscii("tbz")] = extensionIcons[QString::fromAscii("tar")]  =
                            extensionIcons[QString::fromAscii("7z")]  = extensionIcons[QString::fromAscii("sitx")] =  QString::fromAscii("compressed.png");

    extensionIcons[QString::fromAscii("sql")] = extensionIcons[QString::fromAscii("accdb")] = extensionIcons[QString::fromAscii("db")]  =
                            extensionIcons[QString::fromAscii("dbf")]  = extensionIcons[QString::fromAscii("mdb")]  =
                            extensionIcons[QString::fromAscii("pdb")] = QString::fromAscii("database.png");

    extensionIcons[QString::fromAscii("dwt")] = QString::fromAscii("dreamweaver.png");

    extensionIcons[QString::fromAscii("folder")] = QString::fromAscii("folder.png");

    extensionIcons[QString::fromAscii("xls")] = extensionIcons[QString::fromAscii("xlsx")] = extensionIcons[QString::fromAscii("xlt")]  =
                            extensionIcons[QString::fromAscii("xltm")]  = QString::fromAscii("excel.png");

    extensionIcons[QString::fromAscii("exe")] = extensionIcons[QString::fromAscii("com")] = extensionIcons[QString::fromAscii("bin")]  =
                            extensionIcons[QString::fromAscii("apk")]  = extensionIcons[QString::fromAscii("app")]  =
                             extensionIcons[QString::fromAscii("msi")]  = extensionIcons[QString::fromAscii("cmd")]  =
                            extensionIcons[QString::fromAscii("gadget")] = QString::fromAscii("executable.png");

    extensionIcons[QString::fromAscii("as")] = extensionIcons[QString::fromAscii("ascs")] =
               extensionIcons[QString::fromAscii("asc")]  = QString::fromAscii("fla_lang.png");
    extensionIcons[QString::fromAscii("fla")] = QString::fromAscii("flash.png");

    extensionIcons[QString::fromAscii("fnt")] = extensionIcons[QString::fromAscii("otf")] = extensionIcons[QString::fromAscii("ttf")]  =
                            extensionIcons[QString::fromAscii("fon")]  = QString::fromAscii("font.png");

    extensionIcons[QString::fromAscii("gpx")] = extensionIcons[QString::fromAscii("kml")] = extensionIcons[QString::fromAscii("kmz")]  = QString::fromAscii("gis.png");


    extensionIcons[QString::fromAscii("gif")] = extensionIcons[QString::fromAscii("tiff")]  = extensionIcons[QString::fromAscii("tif")]  =
                            extensionIcons[QString::fromAscii("bmp")]  = extensionIcons[QString::fromAscii("png")] =
                            extensionIcons[QString::fromAscii("tga")]  = QString::fromAscii("graphic.png");

    extensionIcons[QString::fromAscii("html")] = extensionIcons[QString::fromAscii("htm")]  = extensionIcons[QString::fromAscii("dhtml")]  =
                            extensionIcons[QString::fromAscii("xhtml")]  = QString::fromAscii("html.png");

    extensionIcons[QString::fromAscii("ai")] = extensionIcons[QString::fromAscii("ait")] = QString::fromAscii("illustrator.png");
    extensionIcons[QString::fromAscii("jpg")] = extensionIcons[QString::fromAscii("jpeg")] = QString::fromAscii("image.png");
    extensionIcons[QString::fromAscii("indd")] = QString::fromAscii("indesign.png");

    extensionIcons[QString::fromAscii("jar")] = extensionIcons[QString::fromAscii("java")]  = extensionIcons[QString::fromAscii("class")]  = QString::fromAscii("java.png");

    extensionIcons[QString::fromAscii("mid")] = extensionIcons[QString::fromAscii("midi")] = QString::fromAscii("midi.png");
    extensionIcons[QString::fromAscii("pdf")] = QString::fromAscii("pdf.png");
    extensionIcons[QString::fromAscii("abr")] = extensionIcons[QString::fromAscii("psb")]  = extensionIcons[QString::fromAscii("psd")]  =
                            QString::fromAscii("photoshop.png");

    extensionIcons[QString::fromAscii("pls")] = extensionIcons[QString::fromAscii("m3u")]  = extensionIcons[QString::fromAscii("asx")]  =
                            QString::fromAscii("playlist.png");
    extensionIcons[QString::fromAscii("pcast")] = QString::fromAscii("podcast.png");

    extensionIcons[QString::fromAscii("pps")] = extensionIcons[QString::fromAscii("ppt")]  = extensionIcons[QString::fromAscii("pptx")] = QString::fromAscii("powerpoint.png");

    extensionIcons[QString::fromAscii("prproj")] = extensionIcons[QString::fromAscii("ppj")]  = QString::fromAscii("premiere.png");

    extensionIcons[QString::fromAscii("3fr")] = extensionIcons[QString::fromAscii("arw")]  = extensionIcons[QString::fromAscii("bay")]  =
                            extensionIcons[QString::fromAscii("cr2")]  = extensionIcons[QString::fromAscii("dcr")] =
                            extensionIcons[QString::fromAscii("dng")] = extensionIcons[QString::fromAscii("fff")]  =
                            extensionIcons[QString::fromAscii("mef")] = extensionIcons[QString::fromAscii("mrw")]  =
                            extensionIcons[QString::fromAscii("nef")] = extensionIcons[QString::fromAscii("pef")]  =
                            extensionIcons[QString::fromAscii("rw2")] = extensionIcons[QString::fromAscii("srf")]  =
                            extensionIcons[QString::fromAscii("orf")] = extensionIcons[QString::fromAscii("rwl")]  =
                            QString::fromAscii("raw.png");

    extensionIcons[QString::fromAscii("rm")] = extensionIcons[QString::fromAscii("ra")]  = extensionIcons[QString::fromAscii("ram")]  =
                            QString::fromAscii("real_audio.png");

    extensionIcons[QString::fromAscii("sh")] = extensionIcons[QString::fromAscii("c")]  = extensionIcons[QString::fromAscii("cc")]  =
                            extensionIcons[QString::fromAscii("cpp")]  = extensionIcons[QString::fromAscii("cxx")] =
                            extensionIcons[QString::fromAscii("h")] = extensionIcons[QString::fromAscii("hpp")]  =
                            extensionIcons[QString::fromAscii("dll")] = QString::fromAscii("source_code.png");

    extensionIcons[QString::fromAscii("ods")]  = extensionIcons[QString::fromAscii("ots")]  =
                            extensionIcons[QString::fromAscii("gsheet")]  = extensionIcons[QString::fromAscii("nb")] =
                            extensionIcons[QString::fromAscii("xlr")] = extensionIcons[QString::fromAscii("numbers")]  =
                            QString::fromAscii("spreadsheet.png");

    extensionIcons[QString::fromAscii("swf")] = QString::fromAscii("swf.png");
    extensionIcons[QString::fromAscii("torrent")] = QString::fromAscii("torrent.png");
    extensionIcons[QString::fromAscii("dmg")] = QString::fromAscii("dmg.png");

    extensionIcons[QString::fromAscii("txt")] = extensionIcons[QString::fromAscii("rtf")]  = extensionIcons[QString::fromAscii("ans")]  =
                            extensionIcons[QString::fromAscii("ascii")]  = extensionIcons[QString::fromAscii("log")] =
                            extensionIcons[QString::fromAscii("odt")] = extensionIcons[QString::fromAscii("wpd")]  =
                            QString::fromAscii("text.png");

    extensionIcons[QString::fromAscii("vcf")] = QString::fromAscii("vcard.png");
    extensionIcons[QString::fromAscii("svgz")]  = extensionIcons[QString::fromAscii("svg")]  =
                            extensionIcons[QString::fromAscii("cdr")]  = extensionIcons[QString::fromAscii("eps")] =
                            QString::fromAscii("vector.png");


     extensionIcons[QString::fromAscii("mkv")]  = extensionIcons[QString::fromAscii("webm")]  =
                            extensionIcons[QString::fromAscii("avi")]  = extensionIcons[QString::fromAscii("mp4")] =
                            extensionIcons[QString::fromAscii("m4v")] = extensionIcons[QString::fromAscii("mpg")]  =
                            extensionIcons[QString::fromAscii("mpeg")] = extensionIcons[QString::fromAscii("mov")]  =
                            extensionIcons[QString::fromAscii("3g2")] = extensionIcons[QString::fromAscii("3gp")]  =
                            extensionIcons[QString::fromAscii("asf")] = extensionIcons[QString::fromAscii("wmv")]  =
                            extensionIcons[QString::fromAscii("flv")] = QString::fromAscii("video.png");


     extensionIcons[QString::fromAscii("srt")] = QString::fromAscii("video_subtitles.png");
     extensionIcons[QString::fromAscii("vob")] = QString::fromAscii("video_vob.png");

     extensionIcons[QString::fromAscii("html")]  = extensionIcons[QString::fromAscii("xml")] = extensionIcons[QString::fromAscii("shtml")]  =
                            extensionIcons[QString::fromAscii("dhtml")] = extensionIcons[QString::fromAscii("js")] =
                            extensionIcons[QString::fromAscii("css")]  = QString::fromAscii("web_data.png");

     extensionIcons[QString::fromAscii("php")]  = extensionIcons[QString::fromAscii("php3")]  =
                            extensionIcons[QString::fromAscii("php4")]  = extensionIcons[QString::fromAscii("php5")] =
                            extensionIcons[QString::fromAscii("phtml")] = extensionIcons[QString::fromAscii("inc")]  =
                            extensionIcons[QString::fromAscii("asp")] = extensionIcons[QString::fromAscii("pl")]  =
                            extensionIcons[QString::fromAscii("cgi")] = extensionIcons[QString::fromAscii("py")]  =
                            QString::fromAscii("web_lang.png");


     extensionIcons[QString::fromAscii("doc")]  = extensionIcons[QString::fromAscii("docx")] = extensionIcons[QString::fromAscii("dotx")]  =
                            extensionIcons[QString::fromAscii("wps")] = QString::fromAscii("word.png");
}

void Utilities::countFilesAndFolders(QString path, long *numFiles, long *numFolders, long fileLimit, long folderLimit)
{
    if (!path.size())
    {
        return;
    }

    QApplication::processEvents();

#ifdef WIN32
    if (path.startsWith(QString::fromAscii("\\\\?\\")))
    {
        path = path.mid(4);
    }
#endif

    QFileInfo baseDir(path);
    if (!baseDir.exists() || !baseDir.isDir())
    {
        return;
    }

    if ((((*numFolders) > folderLimit)) || (((*numFiles) > fileLimit)))
    {
        return;
    }

    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (int i = 0; i < entries.size(); i++)
    {
        QFileInfo info = entries[i];
        if (info.isFile())
        {
            (*numFiles)++;
        }
        else if (info.isDir())
        {
            countFilesAndFolders(info.absoluteFilePath(), numFiles, numFolders, fileLimit, folderLimit);
            (*numFolders)++;
        }
    }
}

void Utilities::getFolderSize(QString folderPath, long long *size)
{
    if (!folderPath.size())
    {
        return;
    }

    QDir dir(folderPath);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    for (int i = 0; i < entries.size(); i++)
    {
        QFileInfo info = entries[i];
        if (info.isFile())
        {
            (*size) += info.size();
        }
        else if (info.isDir())
        {
            getFolderSize(info.absoluteFilePath(), size);
        }
    }
}

qreal Utilities::getDevicePixelRatio()
{
#if QT_VERSION >= 0x050000
    return qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0;
#else
    return 1.0;
#endif
}

QString Utilities::getExtensionPixmap(QString fileName, QString prefix)
{
    if (extensionIcons.isEmpty())
    {
        initializeExtensions();
    }

    QFileInfo f(fileName);
    if (extensionIcons.contains(f.suffix().toLower()))
    {
        return QString(extensionIcons[f.suffix().toLower()]).insert(0, prefix);
    }
    else
    {
        return prefix + QString::fromAscii("generic.png");
    }
}

QString Utilities::languageCodeToString(QString code)
{
    if (languageNames.isEmpty())
    {       
        languageNames[QString::fromAscii("ar")] = QString::fromUtf8("العربية");
        languageNames[QString::fromAscii("bg")] = QString::fromUtf8("български");
        languageNames[QString::fromAscii("cs")] = QString::fromUtf8("Čeština");
        languageNames[QString::fromAscii("de")] = QString::fromUtf8("Deutsch");
        languageNames[QString::fromAscii("ee")] = QString::fromUtf8("Eesti");
        languageNames[QString::fromAscii("en")] = QString::fromUtf8("English");
        languageNames[QString::fromAscii("es")] = QString::fromUtf8("Español");
        languageNames[QString::fromAscii("fa")] = QString::fromUtf8("فارسی");
        languageNames[QString::fromAscii("fi")] = QString::fromUtf8("Suomi");
        languageNames[QString::fromAscii("fr")] = QString::fromUtf8("Français");
        languageNames[QString::fromAscii("he")] = QString::fromUtf8("עברית");
        languageNames[QString::fromAscii("hr")] = QString::fromUtf8("Hrvatski");
        languageNames[QString::fromAscii("hu")] = QString::fromUtf8("Magyar");
        languageNames[QString::fromAscii("id")] = QString::fromUtf8("Bahasa Indonesia");
        languageNames[QString::fromAscii("it")] = QString::fromUtf8("Italiano");
        languageNames[QString::fromAscii("ja")] = QString::fromUtf8("日本語");
        languageNames[QString::fromAscii("ka")] = QString::fromUtf8("ქართული");
        languageNames[QString::fromAscii("ko")] = QString::fromUtf8("한국어");
        languageNames[QString::fromAscii("nl")] = QString::fromUtf8("Nederlands");
        languageNames[QString::fromAscii("pl")] = QString::fromUtf8("Polski");
        languageNames[QString::fromAscii("pt_BR")] = QString::fromUtf8("Português Brasil");
        languageNames[QString::fromAscii("pt")] = QString::fromUtf8("Português");
        languageNames[QString::fromAscii("ro")] = QString::fromUtf8("Română");
        languageNames[QString::fromAscii("ru")] = QString::fromUtf8("Pусский");
        languageNames[QString::fromAscii("sk")] = QString::fromUtf8("Slovenský");
        languageNames[QString::fromAscii("sl")] = QString::fromUtf8("Slovenščina");
        languageNames[QString::fromAscii("sr")] = QString::fromUtf8("српски");
        languageNames[QString::fromAscii("sv")] = QString::fromUtf8("Svenska");
        languageNames[QString::fromAscii("th")] = QString::fromUtf8("ภาษาไทย");
        languageNames[QString::fromAscii("tl")] = QString::fromUtf8("Tagalog");
        languageNames[QString::fromAscii("tr")] = QString::fromUtf8("Türkçe");
        languageNames[QString::fromAscii("uk")] = QString::fromUtf8("Українська");
        languageNames[QString::fromAscii("vi")] = QString::fromUtf8("Tiếng Việt");
        languageNames[QString::fromAscii("zh_CN")] = QString::fromUtf8("简体中文");
        languageNames[QString::fromAscii("zh_TW")] = QString::fromUtf8("中文繁體");


        // Currently unsupported
        // languageNames[QString::fromAscii("mi")] = QString::fromUtf8("Māori");
        // languageNames[QString::fromAscii("ca")] = QString::fromUtf8("Català");
        // languageNames[QString::fromAscii("eu")] = QString::fromUtf8("Euskara");
        // languageNames[QString::fromAscii("af")] = QString::fromUtf8("Afrikaans");
        // languageNames[QString::fromAscii("no")] = QString::fromUtf8("Norsk");
        // languageNames[QString::fromAscii("bs")] = QString::fromUtf8("Bosanski");
        // languageNames[QString::fromAscii("da")] = QString::fromUtf8("Dansk");
        // languageNames[QString::fromAscii("el")] = QString::fromUtf8("ελληνικά");
        // languageNames[QString::fromAscii("lt")] = QString::fromUtf8("Lietuvos");
        // languageNames[QString::fromAscii("lv")] = QString::fromUtf8("Latviešu");
        // languageNames[QString::fromAscii("mk")] = QString::fromUtf8("македонски");
        // languageNames[QString::fromAscii("hi")] = QString::fromUtf8("हिंदी");
        // languageNames[QString::fromAscii("ms")] = QString::fromUtf8("Bahasa Malaysia");
        // languageNames[QString::fromAscii("cy")] = QString::fromUtf8("Cymraeg");
    }
    return languageNames.value(code);
}

QString Utilities::getExtensionPixmapSmall(QString fileName)
{
    return getExtensionPixmap(fileName, QString::fromAscii(":/images/small_"));
}

QString Utilities::getExtensionPixmapMedium(QString fileName)
{
    return getExtensionPixmap(fileName, QString::fromAscii(":/images/drag_"));
}

bool Utilities::removeRecursively(QString path)
{
    if (!path.size())
    {
        return false;
    }

    QDir dir(path);
    bool success = false;
    QString qpath = QDir::toNativeSeparators(dir.absolutePath());
    mega::MegaApi::removeRecursively(qpath.toUtf8().constData());
    success = dir.rmdir(dir.absolutePath());
    return success;
}

void Utilities::copyRecursively(QString srcPath, QString dstPath)
{
    if (!srcPath.size() || !dstPath.size())
    {
        return;
    }

    QFileInfo source(srcPath);
    if (!source.exists())
    {
        return;
    }

    if (srcPath == dstPath)
    {
        return;
    }

    QFile dst(dstPath);
    if (dst.exists())
    {
        return;
    }

    if (source.isFile())
    {
        QFile src(srcPath);
        src.copy(dstPath);
#ifndef _WIN32
       QFileInfo info(src);
       time_t t = info.lastModified().toTime_t();
       struct utimbuf times = { t, t };
       utime(dstPath.toUtf8().constData(), &times);
#endif
    }
    else if (source.isDir())
    {
        QDir dstDir(dstPath);
        dstDir.mkpath(QString::fromAscii("."));
        QDirIterator di(srcPath, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            if (!di.fileInfo().isSymLink())
            {
                copyRecursively(di.filePath(), dstPath + QDir::separator() + di.fileName());
            }
        }
    }
}

bool Utilities::verifySyncedFolderLimits(QString path)
{
#ifdef WIN32
    if (path.startsWith(QString::fromAscii("\\\\?\\")))
    {
        path = path.mid(4);
    }
#endif

    QString rootPath = QDir::toNativeSeparators(QDir::rootPath());
    if (rootPath == path)
    {
        return false;
    }
    return true;
}
QString Utilities::getTimeString(long long secs, bool secondPrecision)
{
    int seconds = (int) secs % 60;
    int minutes = (int) ((secs / 60) % 60);
    int hours   = (int) (secs / (60 * 60)) % 24;
    int days = (int)(secs / (60 * 60 * 24));

    int items = 0;
    QString time;

    if (days)
    {
        items++;
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">d</span>").arg(days));
    }

    if (items || hours)
    {
        items++;
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">h</span>").arg(hours));
    }

    if (items == 2)
    {
        time = time.trimmed();
        return time;
    }

    if (items || minutes)
    {
        items++;
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">m</span>").arg(minutes));
    }

    if (items == 2)
    {
        time = time.trimmed();
        return time;
    }

    if (secondPrecision)
    {
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">s</span>").arg(seconds));
    }
    time = time.trimmed();
    return time;
}

QString Utilities::getSizeString(unsigned long long bytes)
{
    unsigned long long KB = 1024;
    unsigned long long MB = 1024 * KB;
    unsigned long long GB = 1024 * MB;
    unsigned long long TB = 1024 * GB;

    if (bytes >= TB)
    {
        return QString::number( ((int)((100 * bytes) / TB))/100.0) + QString::fromAscii(" TB");
    }

    if (bytes >= GB)
    {
        return QString::number( ((int)((100 * bytes) / GB))/100.0) + QString::fromAscii(" GB");
    }

    if (bytes >= MB)
    {
        return QString::number( ((int)((100 * bytes) / MB))/100.0) + QString::fromAscii(" MB");
    }

    if (bytes >= KB)
    {
        return QString::number( ((int)((100 * bytes) / KB))/100.0) + QString::fromAscii(" KB");
    }

    return QString::number(bytes) + QString::fromAscii(" bytes");
}

QString Utilities::extractJSONString(QString json, QString name)
{
    QString pattern = name + QString::fromUtf8("\":\"");
    int pos = json.indexOf(pattern);
    if (pos < 0)
    {
        return QString();
    }

    int end = json.indexOf(QString::fromUtf8("\""), pos + pattern.size());
    if (end < 0)
    {
        return QString();
    }

    return json.mid(pos + pattern.size(), end - pos - pattern.size());
}

long long Utilities::extractJSONNumber(QString json, QString name)
{
    QString pattern = name + QString::fromUtf8("\":");
    int pos = json.indexOf(pattern);
    if (pos < 0)
    {
        return 0;
    }

    int end = pos + pattern.size();
    int count = 0;
    while (json[end].isDigit())
    {
        end++;
        count++;
    }

    return json.mid(pos + pattern.size(), count).toLongLong();
}

QString Utilities::getDefaultBasePath()
{
#ifdef WIN32
    #if QT_VERSION < 0x050000
        QString defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }
    #else
        QStringList defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }
    #endif
#else
    #if QT_VERSION < 0x050000
        QString defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }
    #else
        QStringList defaultPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }
    #endif
#endif

    QDir dataDir = QDir(Preferences::instance()->getDataPath());
    QString rootPath = QDir::toNativeSeparators(dataDir.rootPath());
    if (rootPath.size() && rootPath.at(rootPath.size() - 1) == QDir::separator())
    {
        rootPath.resize(rootPath.size() - 1);
        return rootPath;
    }
    return QString();
}
