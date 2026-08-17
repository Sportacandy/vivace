/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "blurayplayer.h"

#include <QDir>
#include <QFileInfo>

#ifdef VIVACE_HAVE_BLURAY
#include <libbluray/bluray.h>
#include <climits>
#endif

bool BlurayDisc::isBlurayFolder(const QString &path)
{
    // A pure filesystem check -- always available, even in a build without
    // libbluray, so callers can still tell "this looks like a BD disc, but
    // this copy of Vivace has no Blu-ray support" apart from "not a disc at
    // all". BDMV/index.bdmv is the standard BD-ROM root marker (same role
    // VIDEO_TS.IFO plays for DVD).
    const QDir dir(path);
    return dir.exists(QStringLiteral("BDMV/index.bdmv"))
        || dir.exists(QStringLiteral("BDMV/INDEX.BDMV"));
}

BlurayDisc::BlurayDisc() = default;

BlurayDisc::~BlurayDisc()
{
    close();
}

#ifdef VIVACE_HAVE_BLURAY

bool BlurayDisc::open(const QString &path)
{
    close();

    // libbluray's public API takes a plain const char*; UTF-8 is what its
    // own Windows file backend expects a non-ASCII path to be encoded as
    // (matching how VLC/mpv pass paths to it) -- NOT verified against real
    // non-ASCII hardware in this environment, flagged as a follow-up.
    BLURAY *bd = bd_open(path.toUtf8().constData(), nullptr);
    if (!bd)
        return false;

    const uint32_t count = bd_get_titles(bd, TITLES_RELEVANT, 0);
    if (count == 0) {
        bd_close(bd);
        return false;
    }

    m_bd = bd;
    m_mainTitleListIndex = -1;
    const int main = bd_get_main_title(bd);

    for (uint32_t i = 0; i < count; ++i) {
        BLURAY_TITLE_INFO *info = bd_get_title_info(bd, i, 0);
        if (!info)
            continue;

        Title title;
        title.libraryIndex = i;
        title.playlistId = info->playlist;
        title.durationMs = static_cast<qint64>(info->duration) / 90; // 90 kHz -> ms

        for (uint32_t c = 0; c < info->chapter_count; ++c) {
            Chapter chapter;
            chapter.startMs = static_cast<qint64>(info->chapters[c].start) / 90;
            chapter.label = QObject::tr("Chapter %1").arg(c + 1);
            title.chapters.append(chapter);
        }

        if (info->clip_count > 0) {
            const BLURAY_CLIP_INFO &clip = info->clips[0];
            for (int a = 0; a < clip.audio_stream_count; ++a) {
                title.audioLanguages.append(QString::fromLatin1(
                        reinterpret_cast<const char *>(clip.audio_streams[a].lang), 4)
                        .trimmed().remove(QChar(u'\0')));
                title.audioCodingTypes.append(clip.audio_streams[a].coding_type);
            }
            for (int p = 0; p < clip.pg_stream_count; ++p) {
                title.subtitleLanguages.append(QString::fromLatin1(
                        reinterpret_cast<const char *>(clip.pg_streams[p].lang), 4)
                        .trimmed().remove(QChar(u'\0')));
            }
        }

        m_titles.append(title);
        if (main >= 0 && static_cast<uint32_t>(main) == i)
            m_mainTitleListIndex = m_titles.size() - 1;

        bd_free_title_info(info);
    }

    if (m_titles.isEmpty()) {
        close();
        return false;
    }
    if (m_mainTitleListIndex < 0)
        m_mainTitleListIndex = 0; // fall back to the first title

    return true;
}

void BlurayDisc::close()
{
    if (m_bd) {
        bd_close(reinterpret_cast<BLURAY *>(m_bd));
        m_bd = nullptr;
    }
    m_titles.clear();
    m_mainTitleListIndex = -1;
    m_selectedTitleListIndex = -1;
}

bool BlurayDisc::selectTitle(int titleListIndex)
{
    if (!m_bd || titleListIndex < 0 || titleListIndex >= m_titles.size())
        return false;
    BLURAY *bd = reinterpret_cast<BLURAY *>(m_bd);
    if (!bd_select_title(bd, m_titles.at(titleListIndex).libraryIndex))
        return false;
    m_selectedTitleListIndex = titleListIndex;
    return true;
}

QIODevice *BlurayDisc::createDevice(QObject *parent)
{
    if (!m_bd || m_selectedTitleListIndex < 0)
        return nullptr;
    BLURAY *bd = reinterpret_cast<BLURAY *>(m_bd);
    const qint64 sizeBytes = static_cast<qint64>(bd_get_title_size(bd));
    return new BlurayTitleDevice(m_bd, sizeBytes, parent);
}

qint64 BlurayDisc::seekTimeMs(qint64 ms)
{
    if (!m_bd)
        return -1;
    BLURAY *bd = reinterpret_cast<BLURAY *>(m_bd);
    const int64_t result = bd_seek_time(bd, static_cast<uint64_t>(ms) * 90);
    if (result < 0)
        return -1;
    return tellTimeMs();
}

qint64 BlurayDisc::seekChapter(int chapterIndex)
{
    if (!m_bd || chapterIndex < 0)
        return -1;
    BLURAY *bd = reinterpret_cast<BLURAY *>(m_bd);
    const int64_t result = bd_seek_chapter(bd, static_cast<unsigned>(chapterIndex));
    if (result < 0)
        return -1;
    return tellTimeMs();
}

qint64 BlurayDisc::tellTimeMs() const
{
    if (!m_bd)
        return 0;
    BLURAY *bd = reinterpret_cast<BLURAY *>(m_bd);
    return static_cast<qint64>(bd_tell_time(bd)) / 90;
}

qint64 BlurayDisc::currentTitleSizeBytes() const
{
    if (!m_bd || m_selectedTitleListIndex < 0)
        return 0;
    return static_cast<qint64>(bd_get_title_size(reinterpret_cast<BLURAY *>(m_bd)));
}

#else // !VIVACE_HAVE_BLURAY

bool BlurayDisc::open(const QString &) { return false; }
void BlurayDisc::close() { m_titles.clear(); m_mainTitleListIndex = -1; m_selectedTitleListIndex = -1; }
bool BlurayDisc::selectTitle(int) { return false; }
QIODevice *BlurayDisc::createDevice(QObject *) { return nullptr; }
qint64 BlurayDisc::seekTimeMs(qint64) { return -1; }
qint64 BlurayDisc::seekChapter(int) { return -1; }
qint64 BlurayDisc::tellTimeMs() const { return 0; }
qint64 BlurayDisc::currentTitleSizeBytes() const { return 0; }

#endif // VIVACE_HAVE_BLURAY

BlurayTitleDevice::BlurayTitleDevice(void *bd, qint64 sizeBytes, QObject *parent)
    : QIODevice(parent), m_bd(bd), m_size(sizeBytes)
{
}

bool BlurayTitleDevice::open(OpenMode mode)
{
    return QIODevice::open(mode | QIODevice::Unbuffered);
}

void BlurayTitleDevice::close()
{
    QIODevice::close();
}

bool BlurayTitleDevice::seek(qint64 pos)
{
#ifdef VIVACE_HAVE_BLURAY
    if (!m_bd)
        return false;
    const int64_t result = bd_seek(reinterpret_cast<BLURAY *>(m_bd), static_cast<uint64_t>(pos));
    if (result < 0)
        return false;
    return QIODevice::seek(pos);
#else
    Q_UNUSED(pos);
    return false;
#endif
}

qint64 BlurayTitleDevice::readData(char *data, qint64 maxSize)
{
#ifdef VIVACE_HAVE_BLURAY
    if (!m_bd)
        return -1;
    const int n = bd_read(reinterpret_cast<BLURAY *>(m_bd),
                           reinterpret_cast<unsigned char *>(data),
                           static_cast<int>(qMin<qint64>(maxSize, INT_MAX)));
    return n; // -1 error, 0 EOF, >0 bytes read -- matches QIODevice's own contract
#else
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
#endif
}
