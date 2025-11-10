# DCSSVR

A VR port of the open-source dungeon crawler [Dungeon Crawl Stone Soup](https://crawl.develz.org/). Download the latest version [here](https://github.com/Lumorti/DCSSVR/releases). For info on how to play, see the [wiki](http://crawl.chaosforge.org/Crawl_Wiki).

<video src="https://github.com/user-attachments/assets/766cd181-cce0-46a7-be1d-6dbc16a1168c" autoplay loop muted playsinline style="max-width:100%; height:auto;"></video>

This works by running a seperate instance of the ASCII version of the game, then using the output to generate the 3D world. So, when you open your inventory in the VR game, it sends various commands to the ASCII game to open your inventory there, which it then parses and updates the VR version.

Please submit bug reports from the settings menu and I'll get around to fixing them whenever I get a chance. Pull requests, suggestions and other contributions are also welcome. Debug mode can be enabled from the settings if you want to cheat and/or test things.

There's also an experimental APK for Quest (for now not included in the release), but this requires you to run a server (i.e. "server.exe") on a device on the same network, which just acts as an HTTP server for the ASCII game. This hasn't been tested nearly as much as the desktop VR version, and the server can be quite unreliable. The server is needed in this case since Quest doesn't allow running Crawl as a subprocess like on Windows.

## Changelog

DCSSVR v0.3 (10/11/2025)
 - Fixed bug where killed enemies wouldn't show as dead during level up
 - Fixed dying not resetting inventory state
 - Added enemy indicator
 - Added collision to closed doors
 - Improved bug report text input
 - Disabled Formicid for now since dig doesn't work
 - Increased debug logging so I can better figure out what went wrong in future
 - Fixed some in-game log messages

DCSSVR v0.2 (21/10/2025)
 - Fixed softlock if you cancel the initial weapon selection
 - Added keyboard typing to the bug report menu
 - Sending a bug report no longer closes the menu so one can send multiple in a row
 - No longer highlight floors of diagonals
 - Allow entering trap tiles

DCSSVR v0.1 (14/10/2025)
 - Initial release
 - Things that are disabled because they're quite different: Coglins, digging, Nemelex

Future ideas
 - Added more extensive tutorial to info panel
 - Add puppet in inventory to show equipped items

Reported bugs that are so far unreproducable
 - Sleeping indicator on a monster not updating (2025-10-30/182321-W7ZAZALJ)
 - Item sprites remain on the ground past pickup (2025-10-18/230456-D3IWAC2F)
 - Unable to interact with ladder (2025-10-18/225408-YIIUBAOD)
 - Can't interact with consumables (2025-10-18/230023-3XXAXCIM)
