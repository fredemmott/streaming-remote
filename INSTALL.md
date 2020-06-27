# Installation

## OBS Studio

1. Download the OBS studio plugin and extract it
2. Copy the `.dll` or `.so` to the `obs-plugins` sub-folder of your OBS studio installation
   - If you use 64-bit OBS for Windows, this is likely `C:\Program Files\obs-studio\obs-plugins\64bit`
   - If you use 32-bit OBS for Windows, this is likely `C:\Program Files\obs-studio\obs-plugins\32bit`
   - If you use OBS for MacOS, this is likely `/Applications/OBS.app/Contents/PlugIns`
   - If you installed OBS using homebrew, will need to build from source
3. Start OBS
4. Configure the plugin from "Streaming Remote Settings" in the Tools menu

## StreamDeck plugin

1. Download the plugin and open it

## XSplit Broadcaster

1. Download the XSplit plugin and extract it
2. Copy the `dll` to the `ScriptDlls\Local` subdirectory of your XSplit installation; this is likely
  `C:\Program Files (x86)\SplitmediaLabs\XSplit Broadcaster\x64\Scriptdlls\Local`
3. Select "Extensions" -> "Add extension" -> "Add extension file"
4. Use the "Browse" button to find the index.html file inside the plugin directory
5. Open "Tools" -> "Streaming Remote" to enable or configure; closing the window closes the plugin.

## Web UI

1. Download the Web UI and extract it
2. Open `index.html` in your favorite browser; Google Chrome is recommended
