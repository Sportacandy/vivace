/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "shortcuts.h"

#include <QKeyCombination>
#include <QKeySequence>

namespace {
constexpr auto kGroup = "shortcuts";
}

const QList<Shortcuts::Def> &Shortcuts::catalog()
{
    // Menu order. Labels are translated; ids and defaults are stable.
    static const QList<Def> c = {
        { "open_file",       tr("Open"),   tr("Open file"),            "Ctrl+O" },
        { "open_url",        tr("Open"),   tr("Open URL"),             "Ctrl+U" },
        { "quit",            tr("Open"),   tr("Quit"),                 "Ctrl+Q" },

        { "frame_step",      tr("Play"),   tr("Frame step"),           "." },
        { "frame_back_step", tr("Play"),   tr("Frame back step"),      "," },
        { "rewind_short",    tr("Play"),   tr("Rewind (short)"),       "Left" },
        { "forward_short",   tr("Play"),   tr("Forward (short)"),      "Right" },
        { "rewind_medium",   tr("Play"),   tr("Rewind (medium)"),      "Shift+Left" },
        { "forward_medium",  tr("Play"),   tr("Forward (medium)"),     "Shift+Right" },
        { "rewind_long",     tr("Play"),   tr("Rewind (long)"),        "Ctrl+Shift+Left" },
        { "forward_long",    tr("Play"),   tr("Forward (long)"),       "Ctrl+Shift+Right" },
        { "speed_normal",    tr("Play"),   tr("Normal speed"),         "Backspace" },
        { "speed_halve",     tr("Play"),   tr("Halve speed"),          "{" },
        { "speed_double",    tr("Play"),   tr("Double speed"),         "}" },
        { "speed_dec",       tr("Play"),   tr("Speed -10%"),           "[" },
        { "speed_inc",       tr("Play"),   tr("Speed +10%"),           "]" },
        { "previous",        tr("Play"),   tr("Previous"),             "<" },
        { "next",            tr("Play"),   tr("Next"),                 ">" },

        { "fullscreen",      tr("Video"),  tr("Fullscreen"),           "F" },
        { "zoom_reset",      tr("Video"),  tr("Reset zoom and pan"),   "Shift+E" },
        { "zoom_out",        tr("Video"),  tr("Zoom out"),             "W" },
        { "zoom_in",         tr("Video"),  tr("Zoom in"),              "E" },
        { "pan_left",        tr("Video"),  tr("Move left"),            "Alt+Left" },
        { "pan_right",       tr("Video"),  tr("Move right"),           "Alt+Right" },
        { "pan_up",          tr("Video"),  tr("Move up"),              "Alt+Up" },
        { "pan_down",        tr("Video"),  tr("Move down"),            "Alt+Down" },
        { "equalizer",       tr("Video"),  tr("Equalizer"),            "Ctrl+E" },
        { "screenshot",      tr("Video"),  tr("Screenshot"),           "S" },

        { "mute",            tr("Audio"),  tr("Mute"),                 "M" },
        { "volume_dec",      tr("Audio"),  tr("Volume -"),             "9" },
        { "volume_inc",      tr("Audio"),  tr("Volume +"),             "0" },

        { "add_bookmark",    tr("Browse"), tr("Add bookmark"),         "Ctrl+A" },

        { "media_info",      tr("View"),   tr("Information and properties"), "Ctrl+I" },
        { "playlist_toggle", tr("View"),   tr("Playlist"),             "L" },

        { "preferences",     tr("Options"), tr("Preferences"),         "Ctrl+P" },

        { "play_pause",      tr("General"), tr("Play / Pause"),        "Space" },
        { "volume_up",       tr("General"), tr("Volume up"),           "Up" },
        { "volume_down",     tr("General"), tr("Volume down"),         "Down" },
    };
    return c;
}

const Shortcuts::Def *Shortcuts::find(const QString &id)
{
    for (const Def &d : catalog())
        if (d.id == id)
            return &d;
    return nullptr;
}

QString Shortcuts::normalize(const QString &seq)
{
    if (seq.isEmpty())
        return QString();
    return QKeySequence(seq).toString(QKeySequence::PortableText);
}

Shortcuts::Shortcuts(QObject *parent) : QObject(parent)
{
    m_store.beginGroup(QString::fromLatin1(kGroup));
    const QStringList keys = m_store.childKeys();
    for (const QString &id : keys) {
        if (find(id))
            m_custom.insert(id, m_store.value(id).toString());
    }
    m_store.endGroup();
}

QString Shortcuts::sequence(const QString &id) const
{
    if (m_custom.contains(id))
        return m_custom.value(id);
    const Def *d = find(id);
    return d ? d->def : QString();
}

QString Shortcuts::defaultSequence(const QString &id) const
{
    const Def *d = find(id);
    return d ? d->def : QString();
}

QString Shortcuts::label(const QString &id) const
{
    const Def *d = find(id);
    return d ? d->label : QString();
}

bool Shortcuts::isCustom(const QString &id) const
{
    return m_custom.contains(id);
}

QVariantMap Shortcuts::sequences() const
{
    QVariantMap m;
    for (const Def &d : catalog())
        m.insert(d.id, sequence(d.id));
    return m;
}

void Shortcuts::setSequence(const QString &id, const QString &seq)
{
    const Def *d = find(id);
    if (!d)
        return;
    const QString norm = normalize(seq);
    // Reverting to the default is stored as "no override".
    if (norm == normalize(d->def)) {
        reset(id);
        return;
    }
    if (m_custom.value(id) == norm)
        return;
    m_custom.insert(id, norm);
    m_store.setValue(QString::fromLatin1(kGroup) + '/' + id, norm);
    emit changed();
}

void Shortcuts::reset(const QString &id)
{
    if (!m_custom.contains(id))
        return;
    m_custom.remove(id);
    m_store.remove(QString::fromLatin1(kGroup) + '/' + id);
    emit changed();
}

void Shortcuts::resetAll()
{
    if (m_custom.isEmpty())
        return;
    m_custom.clear();
    m_store.remove(QString::fromLatin1(kGroup));
    emit changed();
}

QString Shortcuts::conflict(const QString &seq, const QString &excludeId) const
{
    const QString norm = normalize(seq);
    if (norm.isEmpty())
        return QString();
    for (const Def &d : catalog()) {
        if (d.id == excludeId)
            continue;
        if (normalize(sequence(d.id)) == norm)
            return d.id;
    }
    return QString();
}

QVariantList Shortcuts::actionList() const
{
    QVariantList out;
    for (const Def &d : catalog()) {
        QVariantMap row;
        row.insert("id", d.id);
        row.insert("section", d.section);
        row.insert("label", d.label);
        row.insert("sequence", sequence(d.id));
        row.insert("custom", m_custom.contains(d.id));
        out.append(row);
    }
    return out;
}

QString Shortcuts::sequenceForKey(int key, int modifiers) const
{
    // Ignore a lone modifier press (wait for the actual key).
    switch (key) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr:
    case Qt::Key_unknown:
    case 0:
        return QString();
    default:
        break;
    }
    // Keep only the modifier bits (drop keypad etc.).
    const auto mods = static_cast<Qt::KeyboardModifiers>(
            modifiers & (Qt::ControlModifier | Qt::ShiftModifier
                         | Qt::AltModifier | Qt::MetaModifier));
    return QKeySequence(QKeyCombination(mods, static_cast<Qt::Key>(key)))
            .toString(QKeySequence::PortableText);
}
