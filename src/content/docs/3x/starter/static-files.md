---
title: "aWOT 3.x archive — Serving static files in aWOT"
description: "Archived instructions for serving static files with aWOT 3."
route: "/en/3x/starter/static-files.html"
section: "archive"
---

> **aWOT 3.x archive.** This page describes the previous API.
> Read the [aWOT 4 reference](/en/4x/api.html) or the
> [migration guide](/en/guide/migrating-to-v4.html) for current projects.

This historical tooling workflow targets aWOT 3.x. For aWOT 4, use the [current instructions](/en/starter/static-files.html).

# Serving static files in aWOT

To serve static files such as images, CSS files, and JavaScript files, use the `awot-static` script found in [awot-scipts](https://github.com/lasselukkari/awot-scripts#awot-static).

Use the following command to install awot-scripts:

```sh
$ npm install awot-scripts --save-dev 
```

Example configuration used in the front-end project:

```cpp
{
  "scripts": {
    "awot-static": "awot-static"
  },
  "awot-static": {
   "sources": "./build",
    "indexFile": "index.html",
    "sketchDir": "./ArduinoProject",
    "exclude": [
      "*.map"
    ]
  }
}
```

Execute the script by running `npm run awot-static`.

This will generate a static gzipped payload file `StaticFiles.h` to the `sketchDir` directory. Include the file with `#include "StaticFiles.h"` and to mount the http handlers call `app.use(staticFiles());`.

See the [tutorial](https://awot.net/guide/tutorial.html) for more details.
