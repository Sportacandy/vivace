/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BLURAYPLAYER_H
#define BLURAYPLAYER_H

#include <QIODevice>
#include <QList>
#include <QString>

/*  Basic/primitive Blu-ray Disc playback: BlurayDisc opens a BD-ROM folder
    (BDMV/index.bdmv), enumerates its titles and their chapters, and hands
    out a QIODevice (BlurayTitleDevice) that streams the selected title's
    already-demuxed M2TS bytes into QMediaPlayer::setSourceDevice() — the
    same integration point DVD titles use via DvdTitleDevice.

    Unlike DVD (Vivace's own from-scratch VIDEO_TS.IFO/VOB parser, see
    dvdifoparser.h), this is a thin wrapper around libbluray's public C API
    (bd_open/bd_get_titles/bd_get_title_info/bd_read/bd_seek/...) — BD-ROM's
    on-disc structure (MPLS playlists, CLPI clip info, MovieObject, optional
    AACS/BD+ crypto) is far more involved than DVD's compact IFO format, and
    no vendored reference source is available locally to reimplement it
    against from scratch (CMakeLists.txt's own comment explains this
    decision in full). libbluray already handles multi-clip stitching and
    (when a matching key is available) AACS decryption internally, so this
    wrapper needs none of DvdTitleDevice's own sector/cell-stitching logic.

    Scope, deliberately narrow (mirrors "DVD simple play", the very first
    DVD milestone, not the later menu-lite/BD-J work): no on-disc HDMV/BD-J
    menu navigation at all — just open the disc, list its titles/chapters,
    and play one. A commercial (AACS-encrypted) disc will fail to open
    unless libbluray finds a usable key (KEYDB.cfg / libaacs) — same
    "unencrypted discs only" limitation DVD started with.

    Every method below compiles unconditionally and is a safe no-op
    (returns false/empty/-1) when Vivace was built without libbluray
    (VIVACE_HAVE_BLURAY not defined — see CMakeLists.txt), so
    PlayerController never needs its own #ifdefs.
*/
class BlurayDisc
{
public:
    struct Chapter
    {
        qint64 startMs = 0;
        QString label;
    };
    struct Title
    {
        unsigned libraryIndex = 0; // bd_select_title()/bd_get_title_info() argument
        unsigned playlistId = 0;   // the .mpls file number, for a friendly label
        qint64 durationMs = 0;
        QList<Chapter> chapters;
        // Declared language per audio/PG(subtitle) stream, from the FIRST
        // clip's own BLURAY_STREAM_INFO::lang (ISO 639-2 code, e.g. "jpn",
        // "eng") -- matched POSITIONALLY against whatever FFmpeg actually
        // demuxes (m_player->audioTracks()/subtitleTracks()), same
        // approximation dvdAudioTrackLabels() already makes for DVD, since
        // Qt Multimedia's own generic track model carries no language
        // metadata for these BD-sourced MPEG-TS streams (confirmed empirically
        // against a real disc: FFmpeg's own QMediaMetaData::Language was
        // empty for every track). Only the first clip is read (right for the
        // common single-clip title; an approximation for a multi-clip one,
        // same caveat DVD's own equivalent already documents).
        QList<QString> audioLanguages;
        QList<QString> subtitleLanguages;
        // Raw libbluray bd_stream_type_e coding_type per audio stream (e.g.
        // 0x82 = DTS, 0x86 = DTS-HD Master Audio), matched positionally
        // against audioLanguages -- Qt's own QMediaMetaData::AudioCodec enum
        // has no entry for most BD-native audio codecs (DTS family, TrueHD),
        // so it reports "Unspecified" for them; the Information dialog uses
        // this instead to show a real codec name for BD's "Initial Audio
        // Stream" section (see blurayAudioCodecName() in playercontroller.cpp).
        QList<quint8> audioCodingTypes;
    };

    BlurayDisc();
    ~BlurayDisc();

    // True if path looks like a BD-ROM folder (has BDMV/index.bdmv).
    static bool isBlurayFolder(const QString &path);

    // Opens the disc and enumerates its titles. Returns false (and leaves
    // the object closed) on failure — including an encrypted disc libbluray
    // can't decrypt, or a build without libbluray at all.
    bool open(const QString &path);
    void close();
    bool isOpen() const { return m_bd != nullptr; }

    const QList<Title> &titles() const { return m_titles; }
    // Index into titles() of the disc's likely main feature (bd_get_main_title());
    // -1 if titles() is empty.
    int mainTitleListIndex() const { return m_mainTitleListIndex; }

    // Selects a title (index into titles(), not the raw libbluray title
    // number) for reading. Must be called before createDevice().
    bool selectTitle(int titleListIndex);
    int selectedTitleListIndex() const { return m_selectedTitleListIndex; }

    // Creates a QIODevice serving the currently selected title's demuxed
    // M2TS stream; nullptr if no title is selected or the build has no
    // libbluray. The device only reads/seeks through the shared BLURAY
    // handle this BlurayDisc owns — it must outlive any device it created.
    QIODevice *createDevice(QObject *parent = nullptr);

    // Time-based seek/chapter helpers (libbluray's bd_seek_time/
    // bd_seek_chapter/bd_tell_time) — operate on the same shared handle a
    // previously created device reads from, so a subsequent read continues
    // from wherever these left the internal cursor. Returns the resulting
    // position in ms, or a negative value on failure.
    qint64 seekTimeMs(qint64 ms);
    qint64 seekChapter(int chapterIndex);
    qint64 tellTimeMs() const;

    // Real logical byte size of the currently selected title
    // (bd_get_title_size()) -- QFileInfo::size() on the disc's own hint URL
    // is meaningless for the Information dialog (it's a synthetic
    // device-backed source pointing at the disc FOLDER, not a real playable
    // file, so QFileInfo reports the folder's own directory-entry size,
    // typically 0). 0 if no title is selected or the build has no libbluray.
    qint64 currentTitleSizeBytes() const;

private:
    void *m_bd = nullptr; // BLURAY*, opaque here so this header never needs libbluray.h
    QList<Title> m_titles;
    int m_mainTitleListIndex = -1;
    int m_selectedTitleListIndex = -1;
};

/*  QIODevice over the title currently selected on a BlurayDisc. Random-
    access: seek() repositions libbluray's own internal read cursor
    (bd_seek), readData() reads sequentially from wherever that cursor is
    (bd_read) — so all repositioning must go through seek(), matching
    QIODevice's own random-access contract.
*/
class BlurayTitleDevice : public QIODevice
{
    Q_OBJECT

public:
    // bd is the BLURAY* (as void*) owned by the BlurayDisc that created
    // this device; it must remain valid for this device's whole lifetime.
    // sizeBytes is bd_get_title_size() at creation time.
    BlurayTitleDevice(void *bd, qint64 sizeBytes, QObject *parent = nullptr);

    bool open(OpenMode mode) override;
    void close() override;
    qint64 size() const override { return m_size; }
    bool isSequential() const override { return false; }
    bool seek(qint64 pos) override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    void *m_bd = nullptr;
    qint64 m_size = 0;
};

#endif // BLURAYPLAYER_H
