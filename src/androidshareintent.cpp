/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "androidshareintent.h"

#ifdef Q_OS_ANDROID

#include <QCoreApplication>
#include <QRegularExpression>
#include <QtCore/qcoreapplication_platform.h>
#include <QtCore/qjniobject.h>

namespace {

// Android's Share sheet often hands over more than a bare URL (e.g.
// YouTube's own share text is "Watch \"Title\" on YouTube\nhttps://
// youtu.be/XXXX") -- pull out just the first http(s) URL rather than
// trying to open the whole blob as a media source.
QString extractUrl(const QString &sharedText)
{
    static const QRegularExpression re(QStringLiteral("https?://\\S+"));
    const QRegularExpressionMatch m = re.match(sharedText);
    return m.hasMatch() ? m.captured(0) : QString();
}

// `intent` must be ACTION_SEND with a text/* type carrying EXTRA_TEXT --
// exactly what AndroidManifest.xml's intent-filter declares and what a
// browser's "Share" action sends for a page URL. Returns an empty string
// for anything else (a stray VIEW intent, no intent at all, ...).
QString sharedUrlFromIntent(const QJniObject &intent)
{
    if (!intent.isValid())
        return {};
    const QJniObject action =
            intent.callObjectMethod("getAction", "()Ljava/lang/String;");
    if (!action.isValid()
        || action.toString() != QStringLiteral("android.intent.action.SEND"))
        return {};
    const QJniObject type =
            intent.callObjectMethod("getType", "()Ljava/lang/String;");
    if (!type.isValid() || !type.toString().startsWith(QStringLiteral("text/")))
        return {};
    const QJniObject text = intent.callObjectMethod(
            "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;",
            QJniObject::fromString(QStringLiteral("android.intent.extra.TEXT")).object());
    if (!text.isValid())
        return {};
    return extractUrl(text.toString());
}

} // namespace

QString AndroidShareIntent::coldStartUrl()
{
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
        return {};
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid())
        return {};
    const QJniObject intent =
            activity.callObjectMethod("getIntent", "()Landroid/content/Intent;");
    return sharedUrlFromIntent(intent);
}

bool AndroidShareIntent::handleNewIntent(JNIEnv * /*env*/, jobject intent)
{
    const QString url = sharedUrlFromIntent(QJniObject(intent));
    if (url.isEmpty())
        return false; // not ours -- let any other listener see it
    emit urlShared(url);
    return true;
}

AndroidShareIntent::AndroidShareIntent(QObject *parent)
    : QObject(parent)
{
    QtAndroidPrivate::registerNewIntentListener(this);
}

AndroidShareIntent::~AndroidShareIntent()
{
    QtAndroidPrivate::unregisterNewIntentListener(this);
}

#else // !Q_OS_ANDROID

QString AndroidShareIntent::coldStartUrl()
{
    return {};
}

AndroidShareIntent::AndroidShareIntent(QObject *parent)
    : QObject(parent)
{
}

AndroidShareIntent::~AndroidShareIntent() = default;

#endif
