---
layout: page
title: Serving static files in aWOT
menu: starter
lang: en
redirect_from: "/starter/static-files.html"
---

# Serving static files in aWOT

To serve static files such as images, CSS files, and JavaScript files, use the `awot-static` script found in [awot-scipts](https://github.com/lasselukkari/awot-scripts#awot-static).

Use the following command to install awot-scripts:

```sh
$ npm install awot-scripts --save-dev 
```

Example configuration used in the front-end project:

```arduino
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

This will generate a static gzipped payload file `StaticFiles.ino` to the `sketchDir` directory. To mount the http handlers call `app.use(staticFiles());`. 

See the tutorial for more details.