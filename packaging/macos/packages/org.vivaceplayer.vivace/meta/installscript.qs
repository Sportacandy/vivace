/*  Vivace — macOS installer component script (Qt Installer Framework).

    macOS needs no extra shortcut wiring: the deployed vivace.app bundle in the
    target directory is the launcher (Finder / Launchpad / Spotlight pick it up).
    The default createOperations() extracts the payload. Kept for symmetry with
    the Windows/Linux components and as a hook for future steps (e.g. codesign
    verification).
*/

function Component() {}

Component.prototype.createOperations = function () {
    component.createOperations(); // default: extract the payload
};
