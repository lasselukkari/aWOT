---
title: "aWOT 3.x archive — aWOT application generator"
description: "Archived instructions for generating an aWOT 3 application."
route: "/en/3x/starter/generator.html"
section: "archive"
---

> **aWOT 3.x archive.** This page describes the previous API.
> Read the [aWOT 4 reference](/en/4x/api.html) or the
> [migration guide](/en/guide/migrating-to-v4.html) for current projects.

This historical tooling workflow targets aWOT 3.x. For aWOT 4, use the [Hello world instructions](/en/starter/hello-world.html).

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
