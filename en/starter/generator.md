---
layout: page
title: Create an aWOT 4 project
menu: starter
lang: en
---

# Create an aWOT 4 project

Install **aWOT 4.0.0** with Arduino Library Manager: search for `aWOT`, select
version `4.0.0`, and choose **Install**. Create a new sketch in the Arduino IDE
and save it in a directory with the same name, such as `WebServer/WebServer.ino`.
Copy the complete [Hello world sketch](/en/starter/hello-world.html) for your
Ethernet or Wi-Fi board, set the network details, and select your board and port.
Compile and upload the sketch, then open the board's address in your browser.

The current examples use `App`, keep the installed library separate from the
sketch, and stop the client after processing an ordinary HTTP request. Add routes
with the [basic routing guide](/en/starter/basic-routing.html); use the
[connection guide](/en/guide/connections.html) when keeping a connection open.

The `awot-create` command shipped in `awot-scripts@3.1.2` downloads only
`aWOT.h` and `aWOT.cpp` from the historical `master` source layout and generates
an `Application` sketch. aWOT 4 has multiple source files, so this generator is
not the installation path for v4. The [3.x generator instructions](/en/3x/starter/generator.html)
remain available for historical projects. The same package's separate
`awot-static` converter works with v4 as described in
[Static files](/en/starter/static-files.html).

For a complete frontend and firmware project, follow the
[React, Vite, and TypeScript tutorial](/en/guide/tutorial.html).
