/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "favoritesmodel.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>

#include "playlistparser.h"

namespace {

// Accepts a URL or a local path (incl. Windows paths with backslashes and
// '#'/spaces, which QUrl::fromUserInput would misparse as a fragment).
QUrl toUrl(const QString &text)
{
    QString trimmed = text.trimmed();
    // Strip surrounding quotes (Windows Explorer "Copy as path" adds them).
    if (trimmed.size() >= 2 && trimmed.startsWith(u'"') && trimmed.endsWith(u'"'))
        trimmed = trimmed.mid(1, trimmed.size() - 2);
    if (trimmed.isEmpty())
        return {};
    const QUrl asUrl(trimmed);
    if (asUrl.scheme().size() > 1) // real scheme, not a drive letter
        return asUrl;
    return QUrl::fromLocalFile(QDir::fromNativeSeparators(trimmed));
}

// SMPlayer's configuration directory (mirrors smplayer's Paths::configPath()),
// where its favorites.m3u8 / tv.m3u8 / radio.m3u8 live.
QString smplayerConfigDir()
{
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    return QDir::homePath() + QStringLiteral("/.smplayer");
#else
    const QString xdg = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (!xdg.isEmpty())
        return xdg + QStringLiteral("/smplayer");
    return QDir::homePath() + QStringLiteral("/.config/smplayer");
#endif
}

// Parse one SMPlayer favorites/TV/radio .m3u8 into nodes, recursing into
// submenu files (subentry=1, whose "file" line points to another .m3u8).
// `visited` breaks reference cycles; `depth` caps nesting. SMPlayer's line
// format is:  #EXTINF:<dur>,<name>,<icon>,<subentry>  then a file/URL line.
QList<FavoritesModel::Node> parseSmplayerM3u(const QString &path,
                                             QSet<QString> &visited, int depth)
{
    QList<FavoritesModel::Node> nodes;
    if (depth > 16)
        return nodes;
    const QFileInfo fi(path);
    const QString key = fi.canonicalFilePath().isEmpty()
                                ? fi.absoluteFilePath()
                                : fi.canonicalFilePath();
    if (visited.contains(key))
        return nodes; // already seen — avoid infinite recursion
    visited.insert(key);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return nodes;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(u'\n');

    // Greedy captures, matching SMPlayer's QRegExp: the last commas separate
    // the fields, so a comma-free name/icon parses correctly.
    static const QRegularExpression info4(
            QStringLiteral("^#EXTINF:(.*),(.*),(.*),(.*)"));
    static const QRegularExpression info3(
            QStringLiteral("^#EXTINF:(.*),(.*),(.*)"));

    QString pendingName;
    bool pendingSub = false;
    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty())
            continue;
        if (line.startsWith(QStringLiteral("#EXTM3U"))
            || line.startsWith(QStringLiteral("#M3U"))) {
            continue;
        }
        const QRegularExpressionMatch m4 = info4.match(line);
        if (m4.hasMatch()) {
            pendingName = m4.captured(2);
            pendingSub = m4.captured(4).toInt() == 1;
            continue;
        }
        const QRegularExpressionMatch m3 = info3.match(line);
        if (m3.hasMatch()) {
            pendingName = m3.captured(2);
            pendingSub = false;
            continue;
        }
        if (line.startsWith(u'#'))
            continue; // some other comment — ignore
        // A file/URL line completes the pending entry.
        FavoritesModel::Node node;
        if (pendingSub) {
            node.submenu = true;
            node.name = pendingName.isEmpty() ? line : pendingName;
            node.children = parseSmplayerM3u(line, visited, depth + 1);
        } else {
            const QUrl url = toUrl(line);
            node.name = pendingName.isEmpty() ? url.fileName() : pendingName;
            node.url = url.toString();
        }
        nodes.append(node);
        pendingName.clear();
        pendingSub = false;
    }
    return nodes;
}

} // namespace

FavoritesModel::FavoritesModel(const QString &filePath, QObject *parent)
    : QObject(parent), m_filePath(filePath)
{
    load();
}

FavoritesModel::Node *FavoritesModel::nodeAt(const QString &path)
{
    Node *node = &m_root;
    if (path.isEmpty())
        return node;
    const QStringList parts = path.split(u'/', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        bool ok = false;
        const int index = part.toInt(&ok);
        if (!ok || index < 0 || index >= node->children.size()
            || !node->children[index].submenu) {
            return nullptr;
        }
        node = &node->children[index];
    }
    return node;
}

QVariantList FavoritesModel::items(const QString &path) const
{
    QVariantList rows;
    Node *node = const_cast<FavoritesModel *>(this)->nodeAt(path);
    if (!node)
        return rows;
    for (const Node &child : node->children) {
        rows << QVariantMap {
            { QStringLiteral("name"), child.name },
            { QStringLiteral("url"), child.url },
            { QStringLiteral("isSubmenu"), child.submenu }
        };
    }
    return rows;
}

QUrl FavoritesModel::urlAt(const QString &path, int index) const
{
    Node *node = const_cast<FavoritesModel *>(this)->nodeAt(path);
    if (!node || index < 0 || index >= node->children.size())
        return {};
    return QUrl(node->children.at(index).url);
}

bool FavoritesModel::isSubmenu(const QString &path, int index) const
{
    Node *node = const_cast<FavoritesModel *>(this)->nodeAt(path);
    if (!node || index < 0 || index >= node->children.size())
        return false;
    return node->children.at(index).submenu;
}

void FavoritesModel::addItem(const QString &path, const QString &name,
                             const QString &url)
{
    Node *node = nodeAt(path);
    if (!node)
        return;
    // A blank row is allowed (SMPlayer's "New item" adds an editable row
    // that the user fills in inline afterwards).
    const QUrl parsed = toUrl(url);
    Node item;
    item.name = name.trimmed().isEmpty() && !parsed.isEmpty()
                        ? parsed.fileName() : name.trimmed();
    item.url = parsed.toString();
    node->children.append(item);
    save();
    ++m_revision;
    emit changed();
}

void FavoritesModel::addSubmenu(const QString &path, const QString &name)
{
    Node *node = nodeAt(path);
    if (!node)
        return;
    Node submenu;
    submenu.submenu = true;
    submenu.name = name.trimmed().isEmpty() ? tr("New submenu") : name.trimmed();
    node->children.append(submenu);
    save();
    ++m_revision;
    emit changed();
}

void FavoritesModel::updateItem(const QString &path, int index,
                                const QString &name, const QString &url)
{
    Node *node = nodeAt(path);
    if (!node || index < 0 || index >= node->children.size())
        return;
    Node &child = node->children[index];
    if (child.submenu) {
        child.name = name.trimmed();
    } else {
        const QUrl parsed = toUrl(url);
        child.name = name.trimmed().isEmpty() && !parsed.isEmpty()
                             ? parsed.fileName() : name.trimmed();
        child.url = parsed.toString();
    }
    save();
    ++m_revision;
    emit changed();
}

void FavoritesModel::removeAt(const QString &path, int index)
{
    Node *node = nodeAt(path);
    if (!node || index < 0 || index >= node->children.size())
        return;
    node->children.removeAt(index);
    save();
    ++m_revision;
    emit changed();
}

void FavoritesModel::clearAt(const QString &path)
{
    Node *node = nodeAt(path);
    if (!node || node->children.isEmpty())
        return;
    node->children.clear();
    save();
    ++m_revision;
    emit changed();
}

void FavoritesModel::move(const QString &path, int from, int to)
{
    Node *node = nodeAt(path);
    if (!node || from < 0 || from >= node->children.size() || to < 0
        || to >= node->children.size() || from == to) {
        return;
    }
    node->children.move(from, to);
    save();
    ++m_revision;
    emit changed();
}

void FavoritesModel::addUrl(const QString &name, const QString &url)
{
    addItem(QString(), name, url);
}

// ------------------------------------------------------------- persistence

static QJsonArray toJson(const QList<FavoritesModel::Node> &nodes);

static QJsonObject nodeToJson(const FavoritesModel::Node &node)
{
    QJsonObject obj;
    obj[QStringLiteral("name")] = node.name;
    if (node.submenu) {
        obj[QStringLiteral("submenu")] = true;
        obj[QStringLiteral("children")] = toJson(node.children);
    } else {
        obj[QStringLiteral("url")] = node.url;
    }
    return obj;
}

static QJsonArray toJson(const QList<FavoritesModel::Node> &nodes)
{
    QJsonArray array;
    for (const FavoritesModel::Node &node : nodes)
        array.append(nodeToJson(node));
    return array;
}

static QList<FavoritesModel::Node> fromJson(const QJsonArray &array)
{
    QList<FavoritesModel::Node> nodes;
    for (const QJsonValue &value : array) {
        const QJsonObject obj = value.toObject();
        FavoritesModel::Node node;
        node.name = obj.value(QStringLiteral("name")).toString();
        node.submenu = obj.value(QStringLiteral("submenu")).toBool();
        if (node.submenu)
            node.children = fromJson(obj.value(QStringLiteral("children")).toArray());
        else
            node.url = obj.value(QStringLiteral("url")).toString();
        nodes.append(node);
    }
    return nodes;
}

void FavoritesModel::load()
{
    QFile file(m_filePath);
    if (file.open(QIODevice::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        m_root.children = fromJson(doc.array());
        return;
    }

    // No Vivace file yet — one-time migration from an existing list.
    // 1) A legacy flat .m3u8 sibling written by an early Vivace build.
    const QString legacy = m_filePath.chopped(
                                   QFileInfo(m_filePath).suffix().size())
                           + QStringLiteral("m3u8");
    const QList<PlaylistEntry> entries =
            PlaylistParser::load(QUrl::fromLocalFile(legacy));
    if (!entries.isEmpty()) {
        for (const PlaylistEntry &entry : entries) {
            Node node;
            node.name = entry.title.isEmpty() ? entry.url.fileName()
                                              : entry.title;
            node.url = entry.url.toString();
            m_root.children.append(node);
        }
        save();
        return;
    }

    // 2) SMPlayer's list of the same name (favorites/tv/radio .m3u8), so users
    // migrating from SMPlayer keep their favorites, TV and radio channels
    // (including submenus). Runs only while Vivace has no file of its own.
    const QString smFile = smplayerConfigDir() + u'/'
            + QFileInfo(m_filePath).completeBaseName() + QStringLiteral(".m3u8");
    if (QFileInfo::exists(smFile)) {
        QSet<QString> visited;
        m_root.children = parseSmplayerM3u(smFile, visited, 0);
        if (!m_root.children.isEmpty())
            save();
    }
}

void FavoritesModel::save() const
{
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    file.write(QJsonDocument(toJson(m_root.children)).toJson());
}
