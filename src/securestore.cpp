#include "securestore.h"

#include "keychain.h"

#include <QDebug>
#include <QEventLoop>

using namespace QKeychain;

namespace {

constexpr auto kService = "Vivace";

// Jobs are stack-allocated by the callers below; QtKeychain's Job::autoDelete
// defaults to true and calls deleteLater() on itself from emitFinished(),
// which would be undefined behavior on a non-heap object, so always disable
// it before start().
void runBlocking(Job &job)
{
    job.setAutoDelete(false);
    QEventLoop loop;
    QObject::connect(&job, &Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

} // namespace

QString SecureCredentials::read(const QString &key)
{
    ReadPasswordJob job{QLatin1String(kService)};
    job.setKey(key);
    runBlocking(job);
    if (job.error() != NoError)
        return QString();
    return job.textData();
}

bool SecureCredentials::write(const QString &key, const QString &value)
{
    WritePasswordJob job{QLatin1String(kService)};
    job.setKey(key);
    job.setTextData(value);
    runBlocking(job);
    if (job.error() != NoError) {
        qWarning("SecureCredentials: failed to store '%s' in the OS credential "
                 "store: %s",
                 qPrintable(key), qPrintable(job.errorString()));
        return false;
    }
    return true;
}

void SecureCredentials::remove(const QString &key)
{
    DeletePasswordJob job{QLatin1String(kService)};
    job.setKey(key);
    runBlocking(job);
}
