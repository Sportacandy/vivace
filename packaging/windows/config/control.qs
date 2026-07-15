/*  Vivace — Qt Installer Framework control script.

    Lets the installer reinstall over an EXISTING installation by uninstalling the
    previous one first (running its own maintenancetool purge), then letting IFW
    install fresh into the freed directory.

    WHEN the uninstall runs (important — an earlier version got this wrong):
      * GUI: only once the user has EXPLICITLY CONSENTED. An "Uninstall existing
        version" consent page (installscript.qs / uninstallcommit.ui) is shown
        just before the "Ready to Install" summary whenever the target contains an
        installation; the user must press Next there (Cancel quits without
        touching anything). The uninstall then runs when the Ready page appears
        (ReadyForInstallationPageCallback) — which is reachable only by consenting
        on that page, and is safely BEFORE IFW writes any files. It is NOT run
        when the folder page is merely shown, so cancelling anywhere before
        consent leaves the existing installation completely intact. (The first
        version purged as soon as the folder page was displayed, so a Cancel there
        destroyed the existing install.)
      * CLI/unattended: from the constructor, because there is no page flow and
        the CLI raises the hard "TargetDirectoryInUse" error unless the directory
        is freed before validation. (The GUI does NOT raise that error — it allows
        installing into a directory that already contains an installation.)

    WHY the maintenancetool and not a directory delete: an earlier version deleted
    the directory directly (rmdir /s /q). That removed maintenancetool.exe but LEFT
    the uninstall entries in the registry, so Vivace showed up as several phantom
    installs that could no longer be uninstalled. Running purge removes the files
    AND the registry entries — the correct, reversible uninstall.

    The maintenancetool self-relaunches from a temporary copy to delete its own
    directory, so the launched process returns BEFORE the delete completes. We run
    it through a PowerShell wrapper that starts purge and then polls until
    maintenancetool.exe is gone (120 s timeout); installer.execute() blocks on the
    wrapper, so nothing proceeds until the purge has finished.

    Failure mode is benign: if purge fails or times out, maintenancetool.exe is
    still present and the existing installation is left intact — nothing is
    corrupted (the GUI then just installs over it; the CLI shows its normal error).

    Elevation: a per-user install (home) purges unelevated. An all-users install
    (Program Files) needs admin — the install-scope page already called
    gainAdminRights() for that choice; we call it again (idempotent) and run the
    purge through IFW's elevated operation executor, since installer.execute()
    stays unelevated even after gainAdminRights().

    NOTE: an unattended install to a *custom* directory (installer --root <dir>)
    can't be auto-overwritten here — --root is applied after this script's
    constructor. Uninstall first (maintenancetool purge) for such installs.
*/

function isProgramFilesTarget(dir) {
    var pf = installer.value("ApplicationsDir");
    if (!pf)
        return false;
    return dir.toLowerCase().indexOf(pf.toLowerCase()) === 0;
}

function removePreviousInstall(dir) {
    if (!dir)
        return;
    var mt = dir + "/maintenancetool.exe";
    if (!installer.fileExists(mt))
        return; // no previous installation in this directory

    var mtWin = mt.replace(/\//g, "\\");
    // Start purge (headless via the command args, in a hidden window), then wait
    // for the tool's temp copy to finish deleting the install directory (poll
    // maintenancetool.exe). The wrapper polls for up to 120 s, so its own window
    // must stay hidden the whole time — run powershell with -WindowStyle Hidden
    // (and hide the launched maintenancetool) so no console flashes at the user.
    var ps = "$ErrorActionPreference='SilentlyContinue';"
           + "Start-Process -FilePath '" + mtWin + "' -WindowStyle Hidden "
           + "-ArgumentList @('purge','--default-answer','--confirm-command');"
           + "$deadline=(Get-Date).AddSeconds(120);"
           + "while((Test-Path -LiteralPath '" + mtWin + "') "
           + "-and ((Get-Date) -lt $deadline)){Start-Sleep -Milliseconds 500}";
    var psArgs = ["-NoProfile", "-WindowStyle", "Hidden",
                  "-ExecutionPolicy", "Bypass", "-Command", ps];

    if (isProgramFilesTarget(dir)) {
        installer.gainAdminRights(); // idempotent; needed for the CLI path
        installer.performOperation("Execute", ["powershell"].concat(psArgs));
    } else {
        installer.execute("powershell", psArgs);
    }
}

function Controller() {
    // Command-line/unattended installs have no page flow, and the CLI raises the
    // hard TargetDirectoryInUse error unless the directory is freed before
    // validation — so free the (config-default) target here.
    if (installer.isInstaller() && installer.isCommandLineInstance())
        removePreviousInstall(installer.value("TargetDir"));
}

Controller.prototype.ReadyForInstallationPageCallback = function() {
    // GUI: reached only after the user consented on the "Uninstall existing
    // version" page (installscript.qs auto-skips that page when nothing is
    // installed, so a no-op here in that case) and before any files are written —
    // the safe moment to uninstall the previous installation. Idempotent: a no-op
    // if the target has no maintenancetool.exe.
    if (installer.isInstaller())
        removePreviousInstall(installer.value("TargetDir"));
};
