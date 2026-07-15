#pragma once

#include <QString>

// Facade over QtKeychain (third_party/qtkeychain/, vendored) for storing
// credentials in the OS-native secure store (Windows Credential Manager /
// macOS Keychain / Linux Secret Service+KWallet) instead of plaintext
// QSettings. All entries live under one fixed service name so they show up
// grouped in the OS credential manager UI.
//
// QtKeychain's job API is async-only; these helpers block on a local
// QEventLoop until the job finishes. That is acceptable here because they
// are only called at Settings construction and on a password edit -- never
// in a hot path -- and it lets Settings keep opensubtitlesPassword/
// proxyPassword as ordinary synchronous Q_PROPERTYs, which
// Settings::snapshot()/restore() (Preferences Cancel/Apply) walks generically
// via the metaobject.
class SecureCredentials
{
public:
    // Empty string if the key isn't found, or on error (e.g. no OS backend).
    static QString read(const QString &key);

    // Returns false on error (e.g. no OS secure-storage backend available).
    // Callers should NOT fall back to plaintext storage on failure.
    static bool write(const QString &key, const QString &value);

    // Best-effort; errors (e.g. the key not existing) are ignored.
    static void remove(const QString &key);
};
