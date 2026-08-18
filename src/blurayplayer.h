/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BLURAYPLAYER_H
#define BLURAYPLAYER_H

#include <QByteArray>
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
    decision in full).

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
    // One clip (= one .m2ts file) within a title's own playlist. A title
    // can chain several clips together (BLURAY_TITLE_INFO::clip_count).
    // libbluray's bd_read() transparently stitches their BYTES into one
    // continuous stream, but each clip carries its OWN independent
    // PCR/PTS/DTS timestamp domain (confirmed empirically: every clip's
    // own in_time is the identical fixed BD-authoring constant 1048560,
    // completely unrelated to any other clip's timestamps) -- the exact
    // problem a BD player's own "PlayItem" stitching solves by presenting
    // ONE continuous timeline across clip boundaries. Found 2026-08-17
    // (real user report against a real disc): naively feeding the raw,
    // discontinuous byte stream to QMediaPlayer let FFmpeg's demuxer hit
    // the discontinuity mid-playback, causing a real stall and a
    // permanently wrong reported duration. A per-clip device-rebuild
    // fix (stop the whole pipeline and reopen at each boundary) was tried
    // and worked, but the rebuild itself cost 100ms-1.5s of dead air at
    // EVERY clip boundary -- unacceptable on real hardware. FIXED
    // PROPERLY (2026-08-17, user's own proposal) by rewriting each
    // packet's PCR/PTS/DTS on the fly (see BlurayTitleDevice::readData())
    // to add this clip's own offsetUnits, making the WHOLE title's byte
    // stream continuously and correctly timestamped from end to end --
    // QMediaPlayer/FFmpeg never sees more than one gapless, well-formed
    // stream, exactly like an ordinary local file, with no rebuild at any
    // clip boundary at all.
    struct ClipRun
    {
        qint64 byteOffset = 0;  // cumulative byte offset within the title's overall bd_read() stream
        qint64 byteSize = 0;    // this clip's own size (BLURAY_CLIP_INFO::pkt_count * 192, the BDAV-TS packet size)
        qint64 startMs = 0;     // cumulative title-relative ("playlist time") start, ms
        qint64 durationMs = 0;  // this clip's own span, ms
        // The amount (90 kHz units, same clock PCR/PTS/DTS use) to ADD to
        // every PCR/PTS/DTS value found in this clip's own packets so they
        // continue seamlessly from the previous clip's rewritten values:
        // BLURAY_CLIP_INFO::start_time (libbluray's own precomputed
        // cumulative "playlist time" for this clip, i.e. exactly where its
        // first frame should land on the continuous title timeline) minus
        // BLURAY_CLIP_INFO::in_time (this clip's own raw, un-rewritten
        // starting timestamp). Commonly NEGATIVE (in_time is a fixed
        // ~11.65s BD-authoring constant for nearly every clip, while
        // start_time grows with cumulative duration) -- rewriting handles
        // this via signed 33-bit modular arithmetic, matching how the
        // real 33-bit PTS/DTS/PCR-base clock itself wraps.
        qint64 offsetUnits = 0;
    };
    struct Title
    {
        unsigned libraryIndex = 0; // bd_select_title()/bd_get_title_info() argument
        unsigned playlistId = 0;   // the .mpls file number, for a friendly label
        qint64 durationMs = 0;
        QList<Chapter> chapters;
        QList<ClipRun> clipRuns;
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

    // Creates a QIODevice serving the WHOLE currently selected title (every
    // clip, back to back, with each clip's own PCR/PTS/DTS rewritten to a
    // continuous timeline -- see ClipRun's own doc comment) as ONE gapless
    // stream. nullptr if no title is selected or the build has no
    // libbluray. The device only reads/seeks through the shared BLURAY
    // handle this BlurayDisc owns — it must outlive any device it created.
    QIODevice *createDevice(QObject *parent = nullptr);

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

/*  QIODevice over the WHOLE title currently selected on a BlurayDisc (every
    clip back to back, see BlurayDisc::ClipRun's own doc comment). Random-
    access: seek() repositions libbluray's own internal read cursor
    (bd_seek), readData() reads sequentially from wherever that cursor is
    (bd_read) — so all repositioning must go through seek(), matching
    QIODevice's own random-access contract.

    Each 192-byte BDAV-TS packet is rewritten IN PLACE as it is read (never
    resized -- byte offsets/size are therefore identical to the raw,
    unrewritten stream, so seeking is exactly as simple as for a single
    clip): its PCR (in the adaptation field, if present) and any PES
    PTS/DTS are shifted by whichever clip that packet falls in own
    offsetUnits, so the WHOLE title presents one continuously, correctly
    timestamped MPEG-TS stream to QMediaPlayer/FFmpeg -- no discontinuity,
    no per-clip device rebuild, no gap.
*/
class BlurayTitleDevice : public QIODevice
{
    Q_OBJECT

public:
    // bd is the BLURAY* (as void*) owned by the BlurayDisc that created
    // this device; it must remain valid for this device's whole lifetime.
    // clips is the selected title's own full, ordered clip list.
    BlurayTitleDevice(void *bd, QList<BlurayDisc::ClipRun> clips, QObject *parent = nullptr);

    bool open(OpenMode mode) override;
    void close() override;
    qint64 size() const override { return m_size; }
    bool isSequential() const override { return false; }
    bool seek(qint64 pos) override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    // Index into m_clips containing absolute byte offset `absPos`, advancing
    // m_currentClip incrementally when possible (reads are normally
    // sequential) rather than always re-searching from scratch.
    int clipIndexForByte(qint64 absPos);

    void *m_bd = nullptr;
    QList<BlurayDisc::ClipRun> m_clips;
    qint64 m_size = 0;
    qint64 m_readCursor = 0;  // absolute byte position of the NEXT byte bd_read() will return
    int m_currentClip = 0;    // last clip index resolved for m_readCursor
    QByteArray m_leftover;    // rewritten bytes produced but not yet returned to the caller
};

#endif // BLURAYPLAYER_H
