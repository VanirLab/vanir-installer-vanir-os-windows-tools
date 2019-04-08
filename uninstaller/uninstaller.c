
#include <Windows.h>

/* 
 * This utility performs post-uninstallation cleanup of Qubes Windows Tools.
 * It's needed because MSI can't revert some changes done by the PV drivers co-installers
 * (like inserting device stack filters for example).
 * We should not depend on QWT libraries because they are already uninstalled at this point.
 */

PWSTR serviceNames[] = { L"qvideo", L"xen", L"xenagent", L"xenlite", L"xenbus", L"xenfilt", L"xeniface", L"xennet", L"xenvif", L"xendisk", L"xenvbd" };

int wmain(int argc, WCHAR *argv[])
{
    LONG status;
    HKEY key;

    // delete QWT keys
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software", 0, KEY_ALL_ACCESS, &key);
    if (status != ERROR_SUCCESS)
        return status;

    RegDeleteTree(key, L"Invisible Things Lab"); // ignore failures in case it's already deleted

    status = RegCloseKey(key);
    if (status != ERROR_SUCCESS)
        return status;

    // delete device stack filter entries created by Xen pvdrivers
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e96a-e325-11ce-bfc1-08002be10318}",
        0, KEY_ALL_ACCESS, &key);

    if (status != ERROR_SUCCESS)
        return status;

    // FIXME: don't delete the whole value, there might be some other filters besides Xen (XENFILT).
    RegDeleteValue(key, L"UpperFilters");

    status = RegCloseKey(key);
    if (status != ERROR_SUCCESS)
        return status;

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e97d-e325-11ce-bfc1-08002be10318}",
        0, KEY_ALL_ACCESS, &key);

    if (status != ERROR_SUCCESS)
        return status;

    RegDeleteValue(key, L"UpperFilters");

    status = RegCloseKey(key);
    if (status != ERROR_SUCCESS)
        return status;

    // delete driver services, SC manager can't do it
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services",
        0, KEY_ALL_ACCESS, &key);

    if (status != ERROR_SUCCESS)
        return status;

    for (int i = 0; i < ARRAYSIZE(serviceNames); i++)
        RegDeleteTree(key, serviceNames[i]);

    status = RegCloseKey(key);
    if (status != ERROR_SUCCESS)
        return status;

    return 0;
}
