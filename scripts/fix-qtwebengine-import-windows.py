"""Makes Qt's 6.12.0 QtWebEngine package load with the final Qt 6.12.0 on Windows.

Usage: python fix-qtwebengine-import-windows.py <path to Qt6WebEngineCore.dll>

Qt built its "6.12.0" QtWebEngine package (extensions/qtwebengine/6120,
2026-07-08) before the final Qt 6.12.0 (2026-09-28). In between,
QAccessibleInterface's default constructor became protected. MSVC encodes
access in symbol names, so the package imports the public name
??0QAccessibleInterface@@QEAA@XZ, which Qt6Gui.dll no longer exports, and
the application fails to start ("procedure entry point ... could not be
located"). Qt6Gui.dll exports the same constructor as
??0QAccessibleInterface@@IEAA@XZ. This is the only one of the DLL's Qt
imports that doesn't resolve, and both names have the same length, so this
script renames the import string in place.

The script does nothing when the old name isn't present (a fixed package,
or a DLL that was already patched), so it is safe to keep running. It fails
if the name occurs more than once, because then the file isn't the one
this fix was made for. The patch invalidates the DLL's Authenticode
signature; Windows still loads it.
"""
import sys

OLD = b'??0QAccessibleInterface@@QEAA@XZ\x00'
NEW = b'??0QAccessibleInterface@@IEAA@XZ\x00'


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    path = sys.argv[1]
    with open(path, 'rb') as f:
        data = f.read()
    count = data.count(OLD)
    if count == 0:
        print(f'No patch needed: {path}')
        return
    if count != 1:
        sys.exit(f'Expected 1 occurrence of the import name, found {count}: {path}')
    with open(path, 'wb') as f:
        f.write(data.replace(OLD, NEW))
    print(f'Patched the QAccessibleInterface import: {path}')


if __name__ == '__main__':
    main()
