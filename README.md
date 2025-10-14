# DCSSVR

A VR port of the open-source dungeon crawler [Dungeon Crawl Stone Soup](https://crawl.develz.org/). Download the latest version [here](https://github.com/Lumorti/DCSSVR/releases). For info on how to play, see the [wiki](http://crawl.chaosforge.org/Crawl_Wiki).

<video src="https://github.com/user-attachments/assets/766cd181-cce0-46a7-be1d-6dbc16a1168c" autoplay loop muted playsinline style="max-width:100%; height:auto;"></video>

This works by running a seperate instance of the ASCII version of the game, then using the output to generate the 3D world. So, when you open your inventory in the VR game, it sends various commands to the ASCII game to open your inventory there, which it then parses and updates the VR version.

Please submit bug reports from the settings menu and I'll get around to fixing them whenever I get a chance. Pull requests, suggestions and other contributions are also welcome. Debug mode can be enabled from the settings if you want to cheat and/or test things.

There's also an experimental APK for Quest (for now not included in the release), but this requires you to run a server (i.e. "server.exe") on a device on the same network, which just acts as an HTTP server for the ASCII game. This hasn't been tested nearly as much as the desktop VR version, and the server can be quite unreliable. The server is needed in this case since Quest doesn't allow running Crawl as a subprocess like on Windows.
