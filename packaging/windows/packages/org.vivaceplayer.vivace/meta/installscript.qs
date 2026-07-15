/*  Vivace — Windows installer component script (Qt Installer Framework).

    Creates Start Menu and Desktop shortcuts to vivace.exe and an uninstaller
    entry (the latter is automatic). The shortcut icon comes from the exe's
    embedded icon (iconId=0) once the brand icon is embedded.

    IFW's CreateShortcut cannot set the shortcut's System.AppUserModel.ID
    property, which is what makes the Windows SMTC / media flyout show "Vivace"
    instead of "unknown app". main.cpp sets the process AUMID
    (VivacePlayer.Vivace); after creating the Start Menu shortcut we stamp the
    matching AUMID onto it with the bundled set-aumid.ps1 helper.
*/

function Component() {
    // Insert the custom wizard pages once the UIs have loaded.
    component.loaded.connect(this, Component.prototype.installerLoaded);
}

// Add the two custom pages: (1) an install-scope page ("all users / just me")
// before the target-directory page — default per-user (no admin); "all users"
// switches the target to Program Files and requests elevation. (2) an
// uninstall-consent page before the "Ready to Install" summary, shown only when
// reinstalling over an existing installation.
Component.prototype.installerLoaded = function () {
    if (installer.isInstaller() !== true)
        return; // no scope choice when updating/uninstalling

    // Install-scope page (all users / just me), before the target-directory page.
    if (installer.addWizardPage(component, "InstallScopeWidget",
                                QInstaller.TargetDirectory)) {
        var page = gui.pageWidgetByObjectName("DynamicInstallScopeWidget");
        if (page !== null) {
            var justMe = gui.findChild(page, "justMeRadioButton");
            var allUsers = gui.findChild(page, "allUsersRadioButton");
            if (justMe !== null && allUsers !== null) {
                justMe.checked = true;
                installer.setValue("TargetDir",
                                   installer.value("HomeDir") + "/Vivace");
                // toggled fires for either button; read the state directly.
                allUsers.toggled.connect(this,
                                         Component.prototype.installScopeChanged);
            }
        }
    }

    // Uninstall-consent page, just before the "Ready to Install" summary. Its
    // entered handler populates the message and auto-skips it when there is no
    // existing installation. The actual uninstall runs on the Ready page (see
    // control.qs), which is reachable only by pressing Next here — so this page
    // is the explicit consent for that uninstall.
    if (installer.addWizardPage(component, "UninstallCommitWidget",
                                QInstaller.ReadyForInstallation)) {
        var cpage = gui.pageWidgetByObjectName("DynamicUninstallCommitWidget");
        if (cpage !== null)
            cpage.entered.connect(this, Component.prototype.uninstallCommitEntered);
    }
};

Component.prototype.uninstallCommitEntered = function () {
    var dir = installer.value("TargetDir");
    if (!installer.fileExists(dir + "/maintenancetool.exe")) {
        // Nothing installed here — skip the consent page.
        gui.clickButton(buttons.NextButton);
        return;
    }
    var page = gui.pageWidgetByObjectName("DynamicUninstallCommitWidget");
    if (page !== null && page.messageLabel !== undefined) {
        page.messageLabel.setText(
            "Vivace is already installed in:\n" + dir + "\n\n"
            + "It must be uninstalled before this version can be installed. "
            + "Click Next to uninstall the existing Vivace and continue, or "
            + "Cancel to quit without changing anything.");
    }
};

Component.prototype.installScopeChanged = function (allUsers) {
    if (allUsers) {
        installer.setValue("TargetDir",
                           installer.value("ApplicationsDir") + "/Vivace");
        // Prompt for administrator rights now (per-machine install). No-op if
        // already elevated; the GUI stays put and only operations run elevated.
        installer.gainAdminRights();
    } else {
        installer.setValue("TargetDir", installer.value("HomeDir") + "/Vivace");
    }
};

Component.prototype.createOperations = function () {
    component.createOperations(); // default: extract the payload

    if (systemInfo.productType === "windows") {
        // The exe lives in bin/ (Qt standard deploy layout).
        // Start Menu shortcut named "Vivace Media Player" so unfamiliar users
        // recognise it under All apps > V. (The desktop shortcut and the app's
        // own window title stay the short "Vivace".)
        component.addOperation(
            "CreateShortcut",
            "@TargetDir@/bin/vivace.exe",
            "@StartMenuDir@/Vivace Media Player.lnk",
            "workingDirectory=@TargetDir@/bin",
            "iconPath=@TargetDir@/bin/vivace.exe", "iconId=0",
            "description=Vivace Media Player");

        // Stamp the Start Menu shortcut's AppUserModelID (must match main.cpp's
        // AUMID) so Windows resolves the SMTC media flyout / taskbar to this
        // app. The App Resolver keys off Start Menu shortcuts, so only that one
        // is stamped. No UNDOEXECUTE needed: CreateShortcut's undo deletes the lnk.
        component.addOperation(
            "Execute",
            "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
            "-File", "@TargetDir@/set-aumid.ps1",
            "@StartMenuDir@/Vivace Media Player.lnk", "VivacePlayer.Vivace");

        component.addOperation(
            "CreateShortcut",
            "@TargetDir@/bin/vivace.exe",
            "@DesktopDir@/Vivace.lnk",
            "workingDirectory=@TargetDir@/bin",
            "iconPath=@TargetDir@/bin/vivace.exe", "iconId=0",
            "description=Vivace Media Player");
    }
};
