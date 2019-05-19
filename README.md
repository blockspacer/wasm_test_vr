Requires:

-   [Emscripten](https://emscripten.org/)
-   [flatbuffer compiler](https://google.github.io/flatbuffers/)

Setup:

Create a file, e.g. emscripten_install.sh.user for setting up your own environment and building.

```bash
#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

export FLATBUFFER_INC=$HOME/Software/flatbuffers/include
./emscripten_install.sh /var/www/html/vr
```

Make it executable.

```bash
chmod +x emscripten_install.sh.user
```

Run:

```bash
./emscripten_install.sh.user
```
