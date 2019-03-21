---
layout: page
title: aWOT application generator
menu: starter
lang: en
redirect_from: "/starter/generator.html"
---

# Application generator

Use the application generator tool, `awot-create` found in [awot-scipts](https://github.com/lasselukkari/awot-scripts#awot-create), to quickly create an application skeleton.

The `awot-scripts` package installs the `awot-create` command-line tool. Use the following command to do so:

```sh
npm install awot-scripts -g
```

To generate the back-end boilerplate to be used with the WiFi library run:

```sh
npx awot-create YourProject  
```

Or if you are using the Ethernet shield run:

```sh
npx awot-create YourProject Ethernet
```

The command created us a ready made Arduino webserver project under the directory YourProject. The AWOT webserver library is vendored in so there are no additional steps to get started. The code is ready for flashing after you have changed the WiFi credentials.