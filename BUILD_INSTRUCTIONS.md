# Build Instructions for Windows

The error `idf.py: The term 'idf.py' is not recognized` means the ESP-IDF environment variables are not set in your current terminal.

## Option 1: Use the ESP-IDF Shortcut (Easiest)

1.  Open your Windows Start Menu.
2.  Search for **"ESP-IDF 5.x PowerShell"** (or "CMD").
3.  Click it to open. This terminal automatically has `idf.py` ready.
4.  Navigate to your project folder:
    ```powershell
    cd "C:\Users\ismai\EEM5043-Smart-Home-System IoT Project using ESP32-C3"
    ```
5.  Run your build commands:
    ```powershell
    idf.py set-target esp32c3
    idf.py build
    idf.py flash monitor
    ```

## Option 2: Enable in Current PowerShell

If you want to use your current terminal (like inside VS Code), you must run the **export script** provided by Espressif.

1.  Find where you installed ESP-IDF. Default is usually `C:\Espressif\frameworks\esp-idf-v5.x\` or `C:\Users\YOUR_USER\esp\esp-idf`.
2.  Run the `export.ps1` script.

**Example command (adjust path if needed):**
```powershell
C:\Espressif\frameworks\esp-idf-v5.1.2\export.ps1
```
OR
```powershell
$HOME/esp/esp-idf/export.ps1
```

Once you see a message saying "ESP-IDF Environment Initialized", you can run `idf.py`.
