/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "androidcontentpath.h"

#ifdef Q_OS_ANDROID
#include <QDir>
#include <QFuture>
#include <QObject>
#include <QStringList>
#include <QtCore/qcoreapplication_platform.h>
#include <QtCore/qjnienvironment.h>
#include <QtCore/qjniobject.h>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace AndroidContentPath {

#ifdef Q_OS_ANDROID

namespace {

// Value of the _data column (the file path) for `uri`, optionally filtered
// by `selection`/`selectionArgs`. Empty if the provider doesn't expose it.
QString queryDataColumn(const QJniObject &uri, const QString &selection = {},
                        const QStringList &selectionArgs = {})
{
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    const QJniObject resolver = context.callObjectMethod(
            "getContentResolver", "()Landroid/content/ContentResolver;");
    if (!resolver.isValid() || !uri.isValid())
        return {};

    QJniEnvironment env;
    const jclass stringClass = env.findClass("java/lang/String");
    const QJniObject dataColumn = QJniObject::fromString(QStringLiteral("_data"));
    jobjectArray projection = env->NewObjectArray(1, stringClass, dataColumn.object());

    QJniObject selectionString;
    jobjectArray args = nullptr;
    if (!selection.isEmpty()) {
        selectionString = QJniObject::fromString(selection);
        args = env->NewObjectArray(jsize(selectionArgs.size()), stringClass, nullptr);
        for (qsizetype i = 0; i < selectionArgs.size(); ++i) {
            const QJniObject arg = QJniObject::fromString(selectionArgs.at(i));
            env->SetObjectArrayElement(args, jsize(i), arg.object());
        }
    }

    const QJniObject cursor = resolver.callObjectMethod(
            "query",
            "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;"
            "[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;",
            uri.object(), projection,
            selectionString.isValid() ? selectionString.object<jstring>() : nullptr,
            args, nullptr);
    env.checkAndClearExceptions();
    env->DeleteLocalRef(projection);
    if (args)
        env->DeleteLocalRef(args);
    if (!cursor.isValid())
        return {};

    QString path;
    if (cursor.callMethod<jboolean>("moveToFirst")) {
        const jint column = cursor.callMethod<jint>(
                "getColumnIndex", "(Ljava/lang/String;)I", dataColumn.object<jstring>());
        if (column >= 0)
            path = cursor.callObjectMethod("getString", "(I)Ljava/lang/String;", column)
                           .toString();
    }
    cursor.callMethod<void>("close");
    env.checkAndClearExceptions();
    return path;
}

// MediaStore row `id` in the "external" volume's Files table, which holds
// every media and download file.
QString mediaStoreFilePath(const QString &id)
{
    const QJniObject filesUri = QJniObject::callStaticObjectMethod(
            "android/provider/MediaStore$Files", "getContentUri",
            "(Ljava/lang/String;)Landroid/net/Uri;",
            QJniObject::fromString(QStringLiteral("external")).object<jstring>());
    return queryDataColumn(filesUri, QStringLiteral("_id=?"), { id });
}

QStringList mediaReadPermissions()
{
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 33)
        return { QStringLiteral("android.permission.READ_MEDIA_VIDEO"),
                 QStringLiteral("android.permission.READ_MEDIA_AUDIO") };
    return { QStringLiteral("android.permission.READ_EXTERNAL_STORAGE") };
}

// Requests `remaining` one at a time (requestPermissions() isn't exported),
// then reports whether any of them is granted.
void requestNext(QObject *context, QStringList remaining,
                 std::function<void(bool)> done)
{
    if (remaining.isEmpty()) {
        done(hasMediaReadPermission());
        return;
    }
    const QString permission = remaining.takeFirst();
    QtAndroidPrivate::requestPermission(permission).then(
            context, [context, remaining, done](QtAndroidPrivate::PermissionResult) {
                requestNext(context, remaining, done);
            });
}

} // namespace

QString contentFilePath(const QUrl &url)
{
    if (url.scheme() != QLatin1String("content"))
        return {};

    const QJniObject uri = QJniObject::callStaticObjectMethod(
            "android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;",
            QJniObject::fromString(url.toString(QUrl::FullyEncoded)).object<jstring>());
    if (!uri.isValid())
        return {};

    QString path;
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    const bool isDocument = QJniObject::callStaticMethod<jboolean>(
            "android/provider/DocumentsContract", "isDocumentUri",
            "(Landroid/content/Context;Landroid/net/Uri;)Z",
            context.object(), uri.object());
    if (isDocument) {
        const QString docId = QJniObject::callStaticObjectMethod(
                "android/provider/DocumentsContract", "getDocumentId",
                "(Landroid/net/Uri;)Ljava/lang/String;", uri.object()).toString();
        const QString authority = url.host();
        if (authority == QLatin1String("com.android.externalstorage.documents")) {
            // "<volume>:<relative path>", e.g. "primary:Download/list.m3u8".
            const qsizetype colon = docId.indexOf(u':');
            if (colon > 0) {
                const QString volume = docId.left(colon);
                const QString relative = docId.mid(colon + 1);
                const QString root = volume.compare(QLatin1String("primary"),
                                                    Qt::CaseInsensitive) == 0
                        ? QJniObject::callStaticObjectMethod(
                                  "android/os/Environment", "getExternalStorageDirectory",
                                  "()Ljava/io/File;")
                                  .callObjectMethod("getAbsolutePath", "()Ljava/lang/String;")
                                  .toString()
                        : QStringLiteral("/storage/") + volume;
                path = root + u'/' + relative;
            }
        } else if (authority == QLatin1String("com.android.providers.downloads.documents")) {
            // "raw:<path>", "msf:<MediaStore id>", or a legacy numeric id.
            if (docId.startsWith(QLatin1String("raw:")))
                path = docId.mid(4);
            else if (docId.startsWith(QLatin1String("msf:")))
                path = mediaStoreFilePath(docId.mid(4));
        } else if (authority == QLatin1String("com.android.providers.media.documents")) {
            // "<type>:<MediaStore id>", e.g. "video:1234".
            const qsizetype colon = docId.indexOf(u':');
            if (colon > 0)
                path = mediaStoreFilePath(docId.mid(colon + 1));
        }
    }
    // Other providers (and plain MediaStore URIs) may expose _data directly.
    if (path.isEmpty())
        path = queryDataColumn(uri);

    QJniEnvironment().checkAndClearExceptions();
    path = QDir::cleanPath(path);
    return QDir::isAbsolutePath(path) ? path : QString();
}

bool hasMediaReadPermission()
{
    const QStringList permissions = mediaReadPermissions();
    for (const QString &permission : permissions) {
        if (QtAndroidPrivate::checkPermission(permission).result()
            == QtAndroidPrivate::Authorized)
            return true;
    }
    return false;
}

void requestMediaReadPermission(QObject *context, std::function<void(bool)> done)
{
    if (hasMediaReadPermission()) {
        done(true);
        return;
    }
    requestNext(context, mediaReadPermissions(), std::move(done));
}

#else // !Q_OS_ANDROID

QString contentFilePath(const QUrl &)
{
    return {};
}

bool hasMediaReadPermission()
{
    return true;
}

void requestMediaReadPermission(QObject *, std::function<void(bool)> done)
{
    done(true);
}

#endif

} // namespace AndroidContentPath
