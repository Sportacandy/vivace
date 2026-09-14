/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Catalog of toolbar-editable items and the default bar layouts. Icon
    fields are Theme icon names (resolved at render time so icon-set
    switching applies). Labels are plain (translation is Phase 6). Used by
    the data-driven MainToolBar / ControlBar and by Preferences > Toolbars
    (PrefToolsPage.qml)'s embedded dual-list editor.
*/
.pragma library

// kind: "button" (icon action), "menu" (popup button), "slider" (stretch),
// "spacer" (stretch filler), "separator".
var catalog = [
    { id: "recentfiles",   label: "Recent files",      icon: "recents",        kind: "menu" },
    { id: "open",          label: "Open file…",        icon: "open",           kind: "button" },
    { id: "opendvd",       label: "Open DVD",          icon: "dvd",            kind: "button" },
    { id: "openbluray",    label: "Open Blu-ray",      icon: "dvd",            kind: "button" },
    { id: "openfolder",    label: "Open directory…",   icon: "openfolder",     kind: "button" },
    { id: "url",           label: "Open URL…",         icon: "url",            kind: "button" },
    { id: "openplaylist",  label: "Open playlist…",    icon: "open_playlist",  kind: "button" },
    { id: "favorites",     label: "Favorites",         icon: "open_favorites", kind: "menu" },
    { id: "tv",            label: "TV",                icon: "open_tv",        kind: "menu" },
    { id: "radio",         label: "Radio",             icon: "open_radio",     kind: "menu" },
    { id: "youtubecache",  label: "YouTube cache…",    icon: "youtube",        kind: "button" },
    { id: "cast",          label: "Cast…",             icon: "cast",           kind: "button" },
    { id: "screenshot",    label: "Screenshot",        icon: "screenshot",     kind: "button" },
    { id: "info",          label: "Information",       icon: "info",           kind: "button" },
    { id: "playlist",      label: "Playlist",          icon: "playlist",       kind: "button" },
    { id: "preferences",   label: "Preferences…",      icon: "prefs",          kind: "button" },
    { id: "playpause",     label: "Play / Pause",      icon: "play",           kind: "button" },
    { id: "stop",          label: "Stop",              icon: "stop",           kind: "button" },
    { id: "previous",      label: "Previous",          icon: "previous",       kind: "button" },
    { id: "next",          label: "Next",              icon: "next",           kind: "button" },
    { id: "prevchapter",   label: "Previous chapter",  icon: "previous",       kind: "button" },
    { id: "nextchapter",   label: "Next chapter",      icon: "next",           kind: "button" },
    { id: "rewindlong",    label: "Rewind (long)",     icon: "rewind10m",      kind: "button" },
    { id: "rewindmed",     label: "Rewind (medium)",   icon: "rewind1m",       kind: "button" },
    { id: "rewindshort",   label: "Rewind (short)",    icon: "rewind10s",      kind: "button" },
    { id: "forwardshort",  label: "Forward (short)",   icon: "forward10s",     kind: "button" },
    { id: "forwardmed",    label: "Forward (medium)",  icon: "forward1m",      kind: "button" },
    { id: "forwardlong",   label: "Forward (long)",    icon: "forward10m",     kind: "button" },
    { id: "framestep",     label: "Frame step",        icon: "forward10s",     kind: "button" },
    { id: "framebackstep", label: "Frame back step",  icon: "rewind10s",      kind: "button" },
    { id: "speed",         label: "Speed",             icon: "speed",          kind: "menu" },
    { id: "speedhalve",    label: "Halve speed",       icon: "speed-x050",     kind: "button" },
    { id: "speednormal",   label: "Normal speed",      icon: "speed-x100",     kind: "button" },
    { id: "speeddouble",   label: "Double speed",      icon: "speed-x200",     kind: "button" },
    { id: "speeddec10",    label: "Speed -10%",        icon: "speed-10",       kind: "button" },
    { id: "speedinc10",    label: "Speed +10%",        icon: "speed+10",       kind: "button" },
    { id: "abmarkera",     label: "Set A marker",      icon: "a_marker",       kind: "button" },
    { id: "abmarkerb",     label: "Set B marker",      icon: "b_marker",       kind: "button" },
    { id: "abclear",       label: "Clear A-B markers", icon: "delete",         kind: "button" },
    { id: "abrepeat",      label: "A-B repeat",        icon: "repeat",         kind: "button" },
    { id: "fullscreen",    label: "Fullscreen",        icon: "fullscreen",     kind: "button" },
    { id: "mute",          label: "Mute",              icon: "volume",         kind: "button" },
    { id: "audiotrack",    label: "Audio track",       icon: "audio_track",    kind: "menu" },
    { id: "subtitletrack", label: "Subtitle track",    icon: "sub",            kind: "menu" },
    { id: "videotrack",    label: "Video track",       icon: "video_track",    kind: "menu" },
    { id: "equalizer",     label: "Equalizer…",        icon: "equalizer",      kind: "button" },
    { id: "aspectratio",   label: "Aspect ratio",      icon: "aspect",         kind: "menu" },
    { id: "rotate",        label: "Rotate",            icon: "rotate",         kind: "menu" },
    { id: "videosize",     label: "Video size",        icon: "video_size",     kind: "menu" },
    { id: "zoompan",       label: "Zoom and pan",      icon: "video_size",     kind: "menu" },
    { id: "flip",          label: "Flip image",        icon: "flip",           kind: "button" },
    { id: "mirror",        label: "Mirror image",      icon: "mirror",         kind: "button" },
    { id: "audiodelaydec", label: "Audio delay -",     icon: "audio_delay_dec",kind: "button" },
    { id: "audiodelayinc", label: "Audio delay +",     icon: "audio_delay_inc",kind: "button" },
    { id: "audiodelayset", label: "Set audio delay…",  icon: "audio_delay",    kind: "button" },
    { id: "loadsubtitles", label: "Load subtitles…",   icon: "sub",            kind: "button" },
    { id: "findsubtitles", label: "Find subtitles…",   icon: "find_subtitles", kind: "button" },
    { id: "unloadsubtitles", label: "Unload subtitles",icon: "unload_subtitles", kind: "button" },
    { id: "subtitledelaydec", label: "Subtitle delay -", icon: "subtitle_delay_dec", kind: "button" },
    { id: "subtitledelayinc", label: "Subtitle delay +", icon: "subtitle_delay_inc", kind: "button" },
    { id: "subtitledelayset", label: "Set subtitle delay…", icon: "subtitle_delay", kind: "button" },
    { id: "dvdmenu",       label: "DVD menu",          icon: "dvd",            kind: "button" },
    { id: "titles",        label: "Title",             icon: "title",          kind: "menu" },
    { id: "chaptersmenu",  label: "Chapters",          icon: "chapter",        kind: "menu" },
    { id: "bookmarksmenu", label: "Bookmarks",         icon: "bookmarks",      kind: "menu" },
    { id: "addbookmark",   label: "Add bookmark",      icon: "add_bookmark",   kind: "button" },
    { id: "help",          label: "Help",              icon: "help",           kind: "menu" },
    { id: "seekslider",    label: "Seek bar",          icon: "",               kind: "slider" },
    { id: "volumeslider",  label: "Volume bar",        icon: "",               kind: "slider" },
    { id: "spacer",        label: "(stretch)",         icon: "",               kind: "spacer" },
    { id: "separator",     label: "(separator)",       icon: "",               kind: "separator" }
];

var defaultMainToolbar = [
    "open", "url", "favorites", "separator",
    "screenshot", "separator",
    "info", "playlist", "separator",
    "preferences", "separator",
    "previous", "next", "separator",
    "audiotrack", "subtitletrack", "spacer"
];

var defaultControlBar = [
    "playpause", "stop", "separator",
    "prevchapter", "rewindlong", "rewindmed", "rewindshort",
    "seekslider",
    "forwardshort", "forwardmed", "forwardlong", "nextchapter",
    "separator", "fullscreen", "mute", "volumeslider"
];

// Mini GUI control widget (SMPlayer MiniGui: minimal, no status bar,
// no main toolbar).
var miniControlBar = [
    "playpause", "stop", "separator",
    "seekslider", "separator",
    "fullscreen", "mute", "volumeslider"
];

// Mpc GUI control widget (SMPlayer MpcGui): the seek slider sits on its own
// full-width row (handled by ControlBar), so it is not listed here.
var mpcControlBar = [
    "playpause", "stop", "separator",
    "rewindmed", "rewindshort", "forwardshort", "forwardmed", "separator",
    "spacer", "mute", "volumeslider"
];

// The default control-bar layout for a GUI mode ("Basic"/"Mini"/"Mpc").
function defaultControlBarFor(gui) {
    if (gui === "Mini") return miniControlBar;
    if (gui === "Mpc") return mpcControlBar;
    return defaultControlBar;
}

function find(id) {
    for (var i = 0; i < catalog.length; ++i)
        if (catalog[i].id === id)
            return catalog[i];
    return null;
}

function labelFor(id) { var e = find(id); return e ? e.label : id; }
function iconFor(id)  { var e = find(id); return e ? e.icon : ""; }
function kindFor(id)  { var e = find(id); return e ? e.kind : "button"; }
