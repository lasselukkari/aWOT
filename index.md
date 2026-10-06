---
layout: home
title: aWOT - Arduino web server library
menu: home
lang: en
redirect_from: "/en/index.html"
---
<section id="home-content">
  {% include header/header-{{ page.lang }}.html %}
  <div id="overlay"></div>
  <div id="homepage-leftpane" class="pane">
    <img id="logo-img" src="images/logo.png" />
  </div>
  <div id="homepage-rightpane" class="pane">
    <section id="description">
      <div class="awot"><a href="/">aWOT</a></div>
      <span class="description">A portable HTTP server library for <a href='https://arduino.cc'>Arduino</a>. Install aWOT 4.0.0 through Arduino Library Manager and <a href="/en/starter/hello-world.html">start with a complete sketch</a>.
      </span>
    </section>
  </div>
</section>
<section id="intro">

  <div id="boxes" class="clearfix">
    <div id="simplicity">
      <h3>Simple</h3> Define routes with <code>App</code>, read requests as Arduino Streams, and write responses with familiar Print methods.
    </div>
    <div id="compatibile">
      <h3>Compatible</h3> Use the same HTTP handlers with Arduino Client transports, including Ethernet and Wi-Fi, or with a Stream such as Serial.
    </div>
    <div id="small">
      <h3>Configurable</h3> Choose request and output buffers for your board. Start with Uno Ethernet or ESP32 Wi-Fi, then explore <a href="/en/guide/configuration.html">memory and callback settings</a>.
    </div>
    <div id="like-express">
      <h3><a href="https://expressjs.com">Like Express</a></h3> Compose routes and middleware with explicit continuation. Add browser interfaces, WebSockets, or server-sent events with the <a href="/en/guide/tutorial.html">v4 guides</a>.
    </div>
  </div>
</section>
