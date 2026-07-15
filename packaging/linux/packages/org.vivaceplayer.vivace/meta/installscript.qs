/*  Vivace — Linux installer component script (Qt Installer Framework).

    Registers a menu entry via a freedesktop .desktop file pointing at the
    installed binary and icon (CreateDesktopEntry writes to
    ~/.local/share/applications for a user-scope install).
*/

function Component() {}

Component.prototype.createOperations = function () {
    component.createOperations(); // default: extract the payload

    if (systemInfo.productType !== "windows") {
        component.addOperation(
            "CreateDesktopEntry",
            "vivace.desktop",
            "Type=Application\n"
            + "Name=Vivace\n"
            + "GenericName=Media Player\n"
            + "Comment=A fast, pure-Qt media player\n"
            + "Exec=\"@TargetDir@/bin/vivace\" %U\n"
            + "Icon=@TargetDir@/vivace.png\n"
            + "Terminal=false\n"
            + "Categories=AudioVideo;Player;Video;\n"
            + "MimeType=video/mp4;video/x-matroska;video/webm;video/mpeg;"
            + "video/quicktime;video/x-msvideo;audio/mpeg;audio/flac;"
            + "audio/x-wav;audio/ogg;\n");
    }
};
