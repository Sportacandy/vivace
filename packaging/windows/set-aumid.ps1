<#
    Stamps a shortcut's System.AppUserModel.ID property.

    Windows resolves the friendly name/icon shown in the SMTC media flyout (and
    taskbar grouping) by matching a running process's AppUserModelID against a
    Start Menu shortcut whose System.AppUserModel.ID property equals it. main.cpp
    sets the process AUMID with SetCurrentProcessExplicitAppUserModelID; the
    installer creates the shortcut, and this script writes the matching property
    onto it (IFW's CreateShortcut can't set shortcut property-store values).

    Usage:  set-aumid.ps1 <shortcut.lnk> <AppUserModelID>
#>
param(
    [Parameter(Mandatory = $true)][string]$LnkPath,
    [Parameter(Mandatory = $true)][string]$AumId
)
$ErrorActionPreference = "Stop"

Add-Type -Namespace Vivace -Name Aumid -MemberDefinition @'
[StructLayout(LayoutKind.Sequential)]
public struct PROPERTYKEY { public Guid fmtid; public uint pid; }

[StructLayout(LayoutKind.Sequential)]
public struct PROPVARIANT {
    public ushort vt; public ushort r1; public ushort r2; public ushort r3;
    public IntPtr p; public IntPtr p2;
}

[ComImport, Guid("0000010b-0000-0000-C000-000000000046"),
 InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
public interface IPersistFile {
    void GetClassID(out Guid pClassID);
    [PreserveSig] int IsDirty();
    void Load([MarshalAs(UnmanagedType.LPWStr)] string f, uint mode);
    void Save([MarshalAs(UnmanagedType.LPWStr)] string f, [MarshalAs(UnmanagedType.Bool)] bool remember);
    void SaveCompleted([MarshalAs(UnmanagedType.LPWStr)] string f);
    void GetCurFile([MarshalAs(UnmanagedType.LPWStr)] out string f);
}

[ComImport, Guid("886d8eeb-8cf2-4446-8d02-cdba1dbdcf99"),
 InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
public interface IPropertyStore {
    void GetCount(out uint c);
    void GetAt(uint i, out PROPERTYKEY key);
    void GetValue(ref PROPERTYKEY key, out PROPVARIANT pv);
    void SetValue(ref PROPERTYKEY key, ref PROPVARIANT pv);
    void Commit();
}

[DllImport("propsys.dll")]
public static extern int PSGetPropertyKeyFromName([MarshalAs(UnmanagedType.LPWStr)] string name, out PROPERTYKEY key);
[DllImport("ole32.dll")]
public static extern int PropVariantClear(ref PROPVARIANT pv);

const ushort VT_LPWSTR = 31;

public static void SetAumId(string lnk, string aumid) {
    // CLSID_ShellLink
    var t = Type.GetTypeFromCLSID(new Guid("00021401-0000-0000-C000-000000000046"));
    object o = Activator.CreateInstance(t);
    var file = (IPersistFile)o;
    file.Load(lnk, 0x00000002 /* STGM_READWRITE */);
    var store = (IPropertyStore)o;
    PROPERTYKEY key;
    if (PSGetPropertyKeyFromName("System.AppUserModel.ID", out key) != 0)
        throw new Exception("PSGetPropertyKeyFromName failed");
    // Build a VT_LPWSTR PROPVARIANT by hand (InitPropVariantFromString is an
    // inline SDK helper, not a DLL export). StringToCoTaskMemUni allocates with
    // CoTaskMemAlloc, which PropVariantClear frees.
    PROPVARIANT pv = new PROPVARIANT();
    pv.vt = VT_LPWSTR;
    pv.p = Marshal.StringToCoTaskMemUni(aumid);
    try {
        store.SetValue(ref key, ref pv);
        store.Commit();
        file.Save(lnk, true);
    } finally {
        PropVariantClear(ref pv);
        Marshal.ReleaseComObject(o);
    }
}
'@

if (-not (Test-Path $LnkPath)) { throw "Shortcut not found: $LnkPath" }
[Vivace.Aumid]::SetAumId($LnkPath, $AumId)
Write-Host "Set System.AppUserModel.ID='$AumId' on $LnkPath"
