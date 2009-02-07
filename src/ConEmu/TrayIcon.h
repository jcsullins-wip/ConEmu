#pragma once

class TrayIcon
{
private:
    NOTIFYICONDATA IconData;

public:
    bool isWindowInTray;
    
    TrayIcon();
    ~TrayIcon();

    void HideWindowToTray();
    void RestoreWindowFromTray();
    void LoadIcon(HWND inWnd, int inIconResource);
    void Delete();
};
