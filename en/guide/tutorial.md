---
layout: page
title: Arduino Full Stack Tutorial
menu: guide
lang: en
redirect_from: "/guide/tutorial.html"
---
<h1>Arduino Full Stack Tutorial</h1>
<h2>Deploying a React app on the ESP32</h2>
<p>
  In this tutorial we are going to build a JavaScript single page app to blink a led.
</p>
<p>
  The tutorial is targeted towards the ESP32 chip but a board with the ESP8266 will work as well. In fact any Arduino compatible board will do as long as it has the required program space. For example, you can use the Arduino Mega together with the Ethernet shield if that is what you have. I will describe the required changes for the different boards.
</p>
<p>
  Of course in real life blinking a led definitely does not require a full blown front-end framework. However, the same workflow can be applied to a lot more complex use cases like <a href="https://github.com/lasselukkari/DuinoDCX">this</a>. Also the tutorial is more about the toolchain than it is about Arduino or React programming.
</p>
<p>
  The ready project is hosted at <a href="https://github.com/lasselukkari/led-blink/">Github</a>. If you want to skip the tutorial and just try out the application first you can download the project from there and flash it to your microcontroller. Throughout the instructions I will also include links to the corresponding commits in the version control system. The links will look like this: <a href="https://github.com/lasselukkari/led-blink/commits/master"><i
    class="fab fa-github"></i></a>
</p>
<p>
  The tutorial is divided into 5 parts:
</p>
<ul>
  <li>
    <a href="#environment" class="list-group-item">
    1. Development Environment
    </a>
  </li>
  <li>
    <a href="#setup" class="list-group-item">
    2. Project Setup
    </a>
  </li>
  <li>
    <a href="#back-end" class="list-group-item">
    3. Back-end Development
    </a>
  </li>
  <li>
    <a href="#front-end" class="list-group-item">
    4. Front-end Development
    </a>
  </li>
  <li>
    <a href="#build" class="list-group-item">
    5. Building the Project
    </a>
  </li>
</ul>
<h3 id="environment">
  1. Development Environment
</h3>
<p>
  So let’s get started. There are few prerequisites that you need to follow this tutorial. First off you need a microcontroller development board. Because we are using the Arduino framework the same code is easy to port to multiple different platforms.
</p>
<p>
  Most of the development boards come with an on board user configurable led but you may also connect your own. A relay module is another option. We just need something to toggle on and off.
</p>
<p>
  Unless you already have the Arduino and Node.js development environments ready you will need to install:
</p>
<ul>
  <li><a href="https://nodejs.org/en/">Node.js JavaScript runtime</a></li>
  <li><a href="https://www.arduino.cc/en/main/software">Arduino IDE</a></li>
  <li><a href="https://github.com/espressif/arduino-esp32#installation-instructions">Arduino ESP32 core</a> or <a href="https://github.com/esp8266/Arduino#installing-with-boards-manager">Arduino ESP8266 core</a></li>
</ul>
<p>
  Make sure your node.js is newer than 8.10.0:
  <br/>
</p>
<pre><code class="language-sh">node -v</code></pre>
<p>
  And that your npm is newer than 5.2:
  <br/>
</p>
<pre><code class="language-sh">npm -v</code></pre>
<h3 id="setup">2. Project Setup</h3>
<p>
  While the same workflow can be applied to any single page app framework we are going to use <a href="https://reactjs.org/">React</a>. To get a head start we are going to create the project template with the <a href="https://github.com/facebook/create-react-app">create-react-app</a> tool.
</p>
<p>
  To generate the application boilerplate run:
  <br/>
</p>
<pre><code class="language-sh">npx create-react-app led-blink</code></pre>
<p>
  Patiently wait while the script downloads half of the internet.
</p>
<p>
  Running the command created us a new project directory called <strong>led-blink</strong> <a href="https://github.com/lasselukkari/led-blink/commit/0e7b7786b2c80bc071ef2c458577bc2db74b1935"><i class="fab fa-github"></i></a>. Open the project in your favourite code editor. Our initial project structure now looks like this:
</p>
<div class="card bg-light">
  <pre>led-blink
├── README.md
  ├── node_modules
  ├── package.json
  ├── .gitignore
  ├── public
  │   ├── favicon.ico
  │   ├── index.html
  │   └── manifest.json
  └── src
      ├── App.css
      ├── App.js
      ├── App.test.js
      ├── index.css
      ├── index.js
      ├── logo.svg
      └── serviceWorker.js</pre>
</div>
<p>
  While the <a href="https://github.com/facebook/create-react-app">create-react-app</a> took care of our front-end boilerplate we still have nothing for the back-end that will be running on the microcontroller.
</p>
<p>
  First move into the newly created project directory in the console window by running:
  <br/>
</p>
<pre><code class="language-sh">cd led-blink</code></pre>
<p>
  Next we need to install some more tools to generate the back-end project:
  <br/>
</p>
<pre><code class="language-sh">npm install awot-scripts --save-dev</code></pre>
&nbsp;
<a href="https://github.com/lasselukkari/led-blink/commit/68190bf759bb2e667544fc7b9214ec5d4a122ed8"><i class="fab fa-github"></i></a>
<p>
  To generate the back-end boilerplate run:
  <br/>
</p>
<pre><code class="language-sh">npx awot-create BlinkServer</code></pre>
&nbsp;
<a href="https://github.com/lasselukkari/led-blink/commit/1091a2e9020099faa8c93ca69daf8b749eb250c3"><i class="fab fa-github"></i></a>
<p>
  Or if you are using the Ethernet shield run:
  <br/>
</p>
<pre><code class="language-sh">npx awot-create BlinkServer Ethernet</code></pre>
<p>
  The command created us a ready made Arduino webserver project under the directory <strong>BlinkServer</strong>. Open the project in the Arduino IDE. If you are planning to edit the Arduino code in your own editor instead of the official IDE, you will want to toggle on the <strong>Use external editor</strong> setting in the Arduino IDE preferences. Otherwise files that are open in the editor will not get updated if they are edited elsewhere.
</p>
<p>
  Now the project structure looks like this:
</p>
<div class="card bg-light">
  <pre>led-blink
  ├── README.md
  ├── node_modules
  ├── package.json
  ├── .gitignore
  ├── BlinkServer
  │   ├── BlinkServer.ino
  │   ├── StaticFiles.ino
  │   ├── aWOT.cpp
  │   └── aWOT.h
  ├── public
  └── src</pre>
</div>
<h3 id="back-end">3. Back-end Development</h3>
<p>
  The HTTP API is going to be simple. We need to implement two HTTP handler functions. One for reading the current state of the led and one for updating it. For simplicity's sake we are going to use just the characters ‘0’ and ‘1’ to describe the state. Also set your wifi credentials to the <strong>WIFI_SSID</strong> and <strong>WIFI_PASSWORD</strong> definitions.
  <a href="https://github.com/lasselukkari/led-blink/commit/6ba32f6e119cec62ec84e0a6cb3572010b9fc2cd"><i class="fab fa-github"></i></a>
</p>
<div class="diff">
  <div class="d2h-file-list-wrapper">
    <div class="d2h-file-list-header">
      <span class="d2h-file-list-title">Files changed (1)</span>
      <a class="d2h-file-switch d2h-hide">hide</a>
      <a class="d2h-file-switch d2h-show">show</a>
    </div>
    <ol class="d2h-file-list">
      <li class="d2h-file-list-line">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon d2h-changed" height="16" title="modified" version="1.1"
            viewBox="0 0 14 16" width="14">
            <path
              d="M13 1H1C0.45 1 0 1.45 0 2v12c0 0.55 0.45 1 1 1h12c0.55 0 1-0.45 1-1V2c0-0.55-0.45-1-1-1z m0 13H1V2h12v12zM4 8c0-1.66 1.34-3 3-3s3 1.34 3 3-1.34 3-3 3-3-1.34-3-3z"></path>
          </svg>
          <a href="#d2h-777323" class="d2h-file-name">BlinkServer/BlinkServer.ino</a>
          <span class="d2h-file-stats">
          <span class="d2h-lines-added">+16</span>
          <span class="d2h-lines-deleted">-0</span>
          </span>
        </span>
      </li>
    </ol>
  </div>
  <div class="d2h-wrapper">
    <div id="d2h-777323" class="d2h-file-wrapper" data-lang="ino">
      <div class="d2h-file-header">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12">
            <path
              d="M6 5H2v-1h4v1zM2 8h7v-1H2v1z m0 2h7v-1H2v1z m0 2h7v-1H2v1z m10-7.5v9.5c0 0.55-0.45 1-1 1H1c-0.55 0-1-0.45-1-1V2c0-0.55 0.45-1 1-1h7.5l3.5 3.5z m-1 0.5L8 2H1v12h10V5z"></path>
          </svg>
          <span class="d2h-file-name">BlinkServer/BlinkServer.ino</span>
          <span class="d2h-tag d2h-changed d2h-changed-tag">CHANGED</span>
        </span>
      </div>
      <div class="d2h-file-diff">
        <div class="d2h-code-wrapper">
          <table class="d2h-diff-table">
            <tbody class="d2h-diff-tbody">
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -3,11 +3,25 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">3</div>
                  <div class="line-num2">3</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">4</div>
                  <div class="line-num2">4</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">#define WIFI_SSID &quot;network&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">5</div>
                  <div class="line-num2">5</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">#define WIFI_PASSWORD &quot;password&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">6</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">#define LED_BUILTIN 2</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">6</div>
                  <div class="line-num2">7</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">7</div>
                  <div class="line-num2">8</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">WiFiServer server(80);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">8</div>
                  <div class="line-num2">9</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">Application app;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">10</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">bool ledOn;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">11</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">12</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">void readLed(Request &amp;req, Response &amp;res) {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">13</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.print(ledOn);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">14</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">}</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">15</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">16</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">void updateLed(Request &amp;req, Response &amp;res) {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">17</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  ledOn = (req.read() != &#x27;0&#x27;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">18</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  digitalWrite(LED_BUILTIN, ledOn);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">19</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  return readLed(req, res);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">20</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">}</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">9</div>
                  <div class="line-num2">21</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">10</div>
                  <div class="line-num2">22</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">void setup() {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">23</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  pinMode(LED_BUILTIN, OUTPUT);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">11</div>
                  <div class="line-num2">24</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  Serial.begin(115200);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">12</div>
                  <div class="line-num2">25</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">13</div>
                  <div class="line-num2">26</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -18,6 +32,8 @@ void setup() {</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">18</div>
                  <div class="line-num2">32</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">19</div>
                  <div class="line-num2">33</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  Serial.println(WiFi.localIP());</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">20</div>
                  <div class="line-num2">34</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">35</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  app.get(&quot;&#x2F;led&quot;, &amp;readLed);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">36</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  app.put(&quot;&#x2F;led&quot;, &amp;updateLed);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">21</div>
                  <div class="line-num2">37</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  app.route(staticFiles());</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">22</div>
                  <div class="line-num2">38</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  server.begin();</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">23</div>
                  <div class="line-num2">39</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">}</span>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</div>
<p>
  If you are using the ESP8266 chip we need to make one more modification. Renaming one import is all that is needed.
</p>
<div class="diff">
  <div class="d2h-file-list-wrapper">
    <div class="d2h-file-list-header">
      <span class="d2h-file-list-title">Files changed (1)</span>
      <a class="d2h-file-switch d2h-hide">hide</a>
      <a class="d2h-file-switch d2h-show">show</a>
    </div>
    <ol class="d2h-file-list">
      <li class="d2h-file-list-line">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon d2h-changed" height="16" title="modified" version="1.1"
            viewBox="0 0 14 16" width="14">
            <path
              d="M13 1H1C0.45 1 0 1.45 0 2v12c0 0.55 0.45 1 1 1h12c0.55 0 1-0.45 1-1V2c0-0.55-0.45-1-1-1z m0 13H1V2h12v12zM4 8c0-1.66 1.34-3 3-3s3 1.34 3 3-1.34 3-3 3-3-1.34-3-3z"></path>
          </svg>
          <a href="#d2h-777324" class="d2h-file-name">BlinkServer/BlinkServer.ino</a>
          <span class="d2h-file-stats">
          <span class="d2h-lines-added">+1</span>
          <span class="d2h-lines-deleted">-1</span>
          </span>
        </span>
      </li>
    </ol>
  </div>
  <div class="d2h-wrapper">
    <div id="d2h-777324" class="d2h-file-wrapper" data-lang="ino">
      <div class="d2h-file-header">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12">
            <path
              d="M6 5H2v-1h4v1zM2 8h7v-1H2v1z m0 2h7v-1H2v1z m0 2h7v-1H2v1z m10-7.5v9.5c0 0.55-0.45 1-1 1H1c-0.55 0-1-0.45-1-1V2c0-0.55 0.45-1 1-1h7.5l3.5 3.5z m-1 0.5L8 2H1v12h10V5z"></path>
          </svg>
          <span class="d2h-file-name">BlinkServer/BlinkServer.ino</span>
          <span class="d2h-tag d2h-changed d2h-changed-tag">CHANGED</span>
        </span>
      </div>
      <div class="d2h-file-diff">
        <div class="d2h-code-wrapper">
          <table class="d2h-diff-table">
            <tbody class="d2h-diff-tbody">
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -1,4 +1,4 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">1</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">#include &lt;<del>WiFi</del>.h&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">1</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">#include &lt;<ins>ESP8266WiFi</ins>.h&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">2</div>
                  <div class="line-num2">2</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">#include &quot;aWOT.h&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">3</div>
                  <div class="line-num2">3</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">4</div>
                  <div class="line-num2">4</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">#define WIFI_SSID &quot;network&quot;</span>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</div>
<p>
  Hook up your development board, and verify that you have the the correct target board and serial port selected. Next open the serial console from <strong>Tools -> Serial Monitor</strong> in Arduino IDE and then upload the sketch to your microcontroller. When the microcontroller boots up and gets a connection it will print the local IP address. Copy it somewhere as we are going to need it later on.
</p>
<p>
  <img src="../../images/tutorial/serialconsole.jpg" alt="Screen shot ot the Arduino serial console displaying the local IP." />
</p>
<p>
  Mine got the IP address <strong>192.168.1.227</strong>.
</p>
<p>
  Before moving on let's make sure our back-end works as expected.
</p>
<p>
  To to turn the led off:
  <br/>
</p>
<pre><code class="language-sh">curl 'http://192.168.1.227/led' -X PUT --data-binary '0'</code></pre>
<p>
  To to turn the led on:
  <br/>
</p>
<pre><code class="language-sh">curl 'http://192.168.1.227/led' -X PUT --data-binary '1'</code></pre>
<h3 id="front-end">4. Front-end Development</h3>
<p>
  Now that the back-end is done, we are ready to start working with the user interface. Uploading the user interface files to the microcontroller board every time we make changes would be a way too slow development cycle. Luckily we can proxy all API requests to the microcontroller while we do local development. To forward the API requests to the microcontroller we need to configure the proxy setting in our package.json. Replace the <strong>arduino.ip</strong> with the address that was printed out in the <strong>Serial Monitor</strong>.
  <a href="https://github.com/lasselukkari/led-blink/commit/9e7ba2e1d8619ad12e542f9103e7db5c9ae942cb"><i
    class="fab fa-github"></i></a>
</p>
<div class="diff">
  <div class="d2h-file-list-wrapper">
    <div class="d2h-file-list-header">
      <span class="d2h-file-list-title">Files changed (1)</span>
      <a class="d2h-file-switch d2h-hide">hide</a>
      <a class="d2h-file-switch d2h-show">show</a>
    </div>
    <ol class="d2h-file-list">
      <li class="d2h-file-list-line">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon d2h-changed" height="16" title="modified" version="1.1"
            viewBox="0 0 14 16" width="14">
            <path
              d="M13 1H1C0.45 1 0 1.45 0 2v12c0 0.55 0.45 1 1 1h12c0.55 0 1-0.45 1-1V2c0-0.55-0.45-1-1-1z m0 13H1V2h12v12zM4 8c0-1.66 1.34-3 3-3s3 1.34 3 3-1.34 3-3 3-3-1.34-3-3z"></path>
          </svg>
          <a href="#d2h-519664" class="d2h-file-name">package.json</a>
          <span class="d2h-file-stats">
          <span class="d2h-lines-added">+2</span>
          <span class="d2h-lines-deleted">-1</span>
          </span>
        </span>
      </li>
    </ol>
  </div>
  <div class="d2h-wrapper">
    <div id="d2h-519664" class="d2h-file-wrapper" data-lang="json">
      <div class="d2h-file-header">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12">
            <path
              d="M6 5H2v-1h4v1zM2 8h7v-1H2v1z m0 2h7v-1H2v1z m0 2h7v-1H2v1z m10-7.5v9.5c0 0.55-0.45 1-1 1H1c-0.55 0-1-0.45-1-1V2c0-0.55 0.45-1 1-1h7.5l3.5 3.5z m-1 0.5L8 2H1v12h10V5z"></path>
          </svg>
          <span class="d2h-file-name">package.json</span>
          <span class="d2h-tag d2h-changed d2h-changed-tag">CHANGED</span>
        </span>
      </div>
      <div class="d2h-file-diff">
        <div class="d2h-code-wrapper">
          <table class="d2h-diff-table">
            <tbody class="d2h-diff-tbody">
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -25,5 +25,6 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">25</div>
                  <div class="line-num2">25</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  ],</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">26</div>
                  <div class="line-num2">26</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  &quot;devDependencies&quot;: {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">27</div>
                  <div class="line-num2">27</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    &quot;awot-scripts&quot;: &quot;0.0.22&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">28</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">  }</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">28</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  },</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">29</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  &quot;proxy&quot;: &quot;http:&#x2F;&#x2F;arduino.ip&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">29</div>
                  <div class="line-num2">30</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">}</span>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</div>
<p>
  Because every cool javascript project uses a lot of libraries to do the most simple things we are going to add a react toggle button component as a project dependency.
  <br/>
</p>
<pre><code class="language-sh">npm install react-toggle-button --save</code></pre>
<p>
  Spin up the development server unless it’s open already:
  <br/>
</p>
<pre><code class="language-sh">npm start</code></pre>
<p>
  The script will take a while to load. Once it is done you should see something like this in your browser:
</p>
<p>
  <img src="../../images/tutorial/start.jpg" alt="Web app screenshot." class="img-fluid" />
</p>
<p>
  As instructed, open up the App.js and do the following modifications.
    <a href="https://github.com/lasselukkari/led-blink/commit/9b23e6c13056d05a277a96d14531982926a01623"><i
    class="fab fa-github"></i></a>
</p>
<div class="diff">
  <div class="d2h-file-list-wrapper">
    <div class="d2h-file-list-header">
      <span class="d2h-file-list-title">Files changed (1)</span>
      <a class="d2h-file-switch d2h-hide">hide</a>
      <a class="d2h-file-switch d2h-show">show</a>
    </div>
    <ol class="d2h-file-list">
      <li class="d2h-file-list-line">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon d2h-changed" height="16" title="modified" version="1.1"
            viewBox="0 0 14 16" width="14">
            <path
              d="M13 1H1C0.45 1 0 1.45 0 2v12c0 0.55 0.45 1 1 1h12c0.55 0 1-0.45 1-1V2c0-0.55-0.45-1-1-1z m0 13H1V2h12v12zM4 8c0-1.66 1.34-3 3-3s3 1.34 3 3-1.34 3-3 3-3-1.34-3-3z"></path>
          </svg>
          <a href="#d2h-649569" class="d2h-file-name">src/App.js</a>
          <span class="d2h-file-stats">
          <span class="d2h-lines-added">+26</span>
          <span class="d2h-lines-deleted">-11</span>
          </span>
        </span>
      </li>
    </ol>
  </div>
  <div class="d2h-wrapper">
    <div id="d2h-649569" class="d2h-file-wrapper" data-lang="js">
      <div class="d2h-file-header">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12">
            <path
              d="M6 5H2v-1h4v1zM2 8h7v-1H2v1z m0 2h7v-1H2v1z m0 2h7v-1H2v1z m10-7.5v9.5c0 0.55-0.45 1-1 1H1c-0.55 0-1-0.45-1-1V2c0-0.55 0.45-1 1-1h7.5l3.5 3.5z m-1 0.5L8 2H1v12h10V5z"></path>
          </svg>
          <span class="d2h-file-name">src/App.js</span>
          <span class="d2h-tag d2h-changed d2h-changed-tag">CHANGED</span>
        </span>
      </div>
      <div class="d2h-file-diff">
        <div class="d2h-code-wrapper">
          <table class="d2h-diff-table">
            <tbody class="d2h-diff-tbody">
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -1,24 +1,39 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">1</div>
                  <div class="line-num2">1</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">import React, { Component } from &#x27;react&#x27;;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">2</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">import ToggleButton from &#x27;react-toggle-button&#x27;;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">2</div>
                  <div class="line-num2">3</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">import logo from &#x27;.&#x2F;logo.svg&#x27;;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">3</div>
                  <div class="line-num2">4</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">import &#x27;.&#x2F;App.css&#x27;;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">4</div>
                  <div class="line-num2">5</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">5</div>
                  <div class="line-num2">6</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">class App extends Component {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">7</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  constructor(props) {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">8</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    super(props);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">9</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    this.state = { ledOn: false };</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">10</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  }</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">11</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">12</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  setLedState(state){</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">13</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    this.setState({ ledOn: state !== &#x27;0&#x27; })</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">14</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  }</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">15</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">16</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  componentDidMount() {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">17</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    fetch(&#x27;&#x2F;led&#x27;)</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">18</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">      .then(response =&gt; response.text())</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">19</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">      .then(state =&gt; this.setLedState(state));</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">20</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  }</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">21</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">22</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  handleStateChange(ledOn) {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">23</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    fetch(&#x27;&#x2F;led&#x27;, { method: &#x27;PUT&#x27;, body: ledOn ? &#x27;0&#x27; : &#x27;1&#x27; })</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">24</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">      .then(response =&gt; response.text())</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">25</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">      .then(state =&gt; this.setLedState(state));</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">26</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  }</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">27</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">6</div>
                  <div class="line-num2">28</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  render() {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">7</div>
                  <div class="line-num2">29</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    return (</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">8</div>
                  <div class="line-num2">30</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">      &lt;div className=&quot;App&quot;&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">9</div>
                  <div class="line-num2">31</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">        &lt;header className=&quot;App-header&quot;&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">10</div>
                  <div class="line-num2">32</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">          &lt;img src={logo} className=&quot;App-logo&quot; alt=&quot;logo&quot; &#x2F;&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">11</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">          &lt;p&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">12</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">            Edit &lt;code&gt;src&#x2F;App.js&lt;&#x2F;code&gt;and save to reload.</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">13</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">          &lt;&#x2F;p&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">14</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">          &lt;a</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">15</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">            className=&quot;App-link&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">16</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">            href=&quot;https:&#x2F;&#x2F;reactjs.org&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">17</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">            target=&quot;_blank&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">18</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">            rel=&quot;noopener noreferrer&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">19</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">          &gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">20</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">            Learn React</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">21</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">          &lt;&#x2F;a&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">33</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">          &lt;ToggleButton</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">34</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">            value={this.state.ledOn}</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">35</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">            onToggle={value =&gt; this.handleStateChange(value)}</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">36</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">          &#x2F;&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">22</div>
                  <div class="line-num2">37</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">        &lt;&#x2F;header&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">23</div>
                  <div class="line-num2">38</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">      &lt;&#x2F;div&gt;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">24</div>
                  <div class="line-num2">39</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    );</span>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</div>
<p>
  Now toggling the button should also blink the led on the microcontroller.
</p>
<div style="text-align:center;">
  <img src="../../images/tutorial/loop.gif" alt="On/Off toggle button." />
</div>
<h3 id="build">5. Building the Project</h3>
<p>
  We are almost ready to upload the app to the microcontroller, but first we need to build a minified version of it. To do this run:
  <br/>
</p>
<pre><code class="language-sh">npm run build</code></pre>
<p>
  The command created us a new directory called <strong>build</strong>. This directory now contains the minified javascript and css files as well as other static files like images.
</p>
<p>
  The next step is to serve these files with the microcontroller. Unless you have set the <strong>Use external
  editor</strong> setting in the Arduino IDE you will need to close the Arduino project at this point.
</p>
<p>
  To convert the files in the <strong>build</strong> directory to a Arduino compatible format we need first add some configurations to the <strong>package.json</strong>.
  <a href="https://github.com/lasselukkari/led-blink/commit/798e1dd96af3710224ef1a7d95be8f7282f6fa68"><i
    class="fab fa-github"></i></a>
</p>
<div class="diff">
  <div class="d2h-file-list-wrapper">
    <div class="d2h-file-list-header">
      <span class="d2h-file-list-title">Files changed (1)</span>
      <a class="d2h-file-switch d2h-hide">hide</a>
      <a class="d2h-file-switch d2h-show">show</a>
    </div>
    <ol class="d2h-file-list">
      <li class="d2h-file-list-line">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon d2h-changed" height="16" title="modified" version="1.1"
            viewBox="0 0 14 16" width="14">
            <path
              d="M13 1H1C0.45 1 0 1.45 0 2v12c0 0.55 0.45 1 1 1h12c0.55 0 1-0.45 1-1V2c0-0.55-0.45-1-1-1z m0 13H1V2h12v12zM4 8c0-1.66 1.34-3 3-3s3 1.34 3 3-1.34 3-3 3-3-1.34-3-3z"></path>
          </svg>
          <a href="#d2h-519664" class="d2h-file-name">package.json</a>
          <span class="d2h-file-stats">
          <span class="d2h-lines-added">+12</span>
          <span class="d2h-lines-deleted">-2</span>
          </span>
        </span>
      </li>
    </ol>
  </div>
  <div class="d2h-wrapper">
    <div id="d2h-519663" class="d2h-file-wrapper" data-lang="json">
      <div class="d2h-file-header">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12">
            <path
              d="M6 5H2v-1h4v1zM2 8h7v-1H2v1z m0 2h7v-1H2v1z m0 2h7v-1H2v1z m10-7.5v9.5c0 0.55-0.45 1-1 1H1c-0.55 0-1-0.45-1-1V2c0-0.55 0.45-1 1-1h7.5l3.5 3.5z m-1 0.5L8 2H1v12h10V5z"></path>
          </svg>
          <span class="d2h-file-name">package.json</span>
          <span class="d2h-tag d2h-changed d2h-changed-tag">CHANGED</span>
        </span>
      </div>
      <div class="d2h-file-diff">
        <div class="d2h-code-wrapper">
          <table class="d2h-diff-table">
            <tbody class="d2h-diff-tbody">
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -12,7 +12,8 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">12</div>
                  <div class="line-num2">12</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    &quot;start&quot;: &quot;react-scripts start&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">13</div>
                  <div class="line-num2">13</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    &quot;build&quot;: &quot;react-scripts build&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">14</div>
                  <div class="line-num2">14</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    &quot;test&quot;: &quot;react-scripts test&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">15</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">    &quot;eject&quot;: &quot;react-scripts eject&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">15</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    &quot;eject&quot;: &quot;react-scripts eject&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">16</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    &quot;dist&quot;: &quot;awot-static&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">16</div>
                  <div class="line-num2">17</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  },</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">17</div>
                  <div class="line-num2">18</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  &quot;eslintConfig&quot;: {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">18</div>
                  <div class="line-num2">19</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    &quot;extends&quot;: &quot;react-app&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -26,5 +27,14 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">26</div>
                  <div class="line-num2">27</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  &quot;devDependencies&quot;: {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">27</div>
                  <div class="line-num2">28</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">    &quot;awot-scripts&quot;: &quot;0.0.22&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">28</div>
                  <div class="line-num2">29</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">  },</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-del">
                  <div class="line-num1">29</div>
                  <div class="line-num2"></div>
                </td>
                <td class="d2h-del">
                  <div class="d2h-code-line d2h-del">
                    <span class="d2h-code-line-prefix">-</span>
                    <span class="d2h-code-line-ctn">  &quot;proxy&quot;: &quot;http:&#x2F;&#x2F;arduino.ip&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">30</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  &quot;proxy&quot;: &quot;http:&#x2F;&#x2F;arduino.ip&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">31</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  &quot;awot-static&quot;: {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">32</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    &quot;sources&quot;: &quot;.&#x2F;build&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">33</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    &quot;indexFile&quot;: &quot;index.html&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">34</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    &quot;sketchDir&quot;: &quot;.&#x2F;BlinkServer&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">35</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    &quot;exclude&quot;: [</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">36</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">      &quot;*.map&quot;,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">37</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">      &quot;service-worker.js&quot;</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">38</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    ]</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">39</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  }</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-cntx">
                  <div class="line-num1">30</div>
                  <div class="line-num2">40</div>
                </td>
                <td class="d2h-cntx">
                  <div class="d2h-code-line d2h-cntx">
                    <span class="d2h-code-line-prefix"> </span>
                    <span class="d2h-code-line-ctn">}</span>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</div>
<p>
  Next run:
  <br/>
</p>
<pre><code class="language-sh">npm run dist</code></pre>
<p>
  This command took the contents of <strong>build</strong> directory and embedded the data to the <strong>StaticFiles.ino</strong> under the <strong>BlinkServer</strong> directory. To save some space the files were at the same time compressed with gzip. The script also generated the proper http headers for encoding, cache control, response length and content type. In real life you would most likely want to exclude this file from your version control system. <a href="https://github.com/lasselukkari/led-blink/commit/9ebea4b00c909418ca24c429d84fec35708d957b"><i
    class="fab fa-github"></i></a>
</p>
<div class="diff">
  <div class="d2h-file-list-wrapper">
    <div class="d2h-file-list-header">
      <span class="d2h-file-list-title">Files changed (1)</span>
      <a class="d2h-file-switch d2h-hide">hide</a>
      <a class="d2h-file-switch d2h-show">show</a>
    </div>
    <ol class="d2h-file-list">
      <li class="d2h-file-list-line">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon d2h-changed" height="16" title="modified" version="1.1"
            viewBox="0 0 14 16" width="14">
            <path
              d="M13 1H1C0.45 1 0 1.45 0 2v12c0 0.55 0.45 1 1 1h12c0.55 0 1-0.45 1-1V2c0-0.55-0.45-1-1-1z m0 13H1V2h12v12zM4 8c0-1.66 1.34-3 3-3s3 1.34 3 3-1.34 3-3 3-3-1.34-3-3z"></path>
          </svg>
          <a href="#d2h-764011" class="d2h-file-name">BlinkServer/StaticFiles.ino</a>
          <span class="d2h-file-stats">
          <span class="d2h-lines-added">+4487</span>
          <span class="d2h-lines-deleted">-14</span>
          </span>
        </span>
      </li>
    </ol>
  </div>
  <div class="d2h-wrapper">
    <div id="d2h-764011" class="d2h-file-wrapper" data-lang="ino">
      <div class="d2h-file-header">
        <span class="d2h-file-name-wrapper">
          <svg aria-hidden="true" class="d2h-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12">
            <path
              d="M6 5H2v-1h4v1zM2 8h7v-1H2v1z m0 2h7v-1H2v1z m0 2h7v-1H2v1z m10-7.5v9.5c0 0.55-0.45 1-1 1H1c-0.55 0-1-0.45-1-1V2c0-0.55 0.45-1 1-1h7.5l3.5 3.5z m-1 0.5L8 2H1v12h10V5z"></path>
          </svg>
          <span class="d2h-file-name">BlinkServer/StaticFiles.ino</span>
          <span class="d2h-tag d2h-changed d2h-changed-tag">CHANGED</span>
        </span>
      </div>
      <div class="d2h-file-diff">
        <div class="d2h-code-wrapper">
          <table class="d2h-diff-table">
            <tbody class="d2h-diff-tbody">
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">@@ -1,21 +1,4494 @@</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">410</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">void static_index (Request &amp;req, Response &amp;res) {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">411</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  P(static_index_0) = {</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">412</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x8d, 0x55,</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-info"></td>
                <td class="d2h-info">
                  <div class="d2h-code-line d2h-info">...</div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">499</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">    0x00</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">500</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  };</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">501</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">502</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.set(&quot;Content-Encoding&quot;, &quot;gzip&quot;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">503</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.set(&quot;Cache-Control&quot;, &quot;no-cache&quot;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">504</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.set(&quot;Content-Length&quot;, &quot;1045&quot;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">505</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.set(&quot;Last-Modified&quot;, &quot;Wed, 16 Jan 2019 20:18:30 GMT&quot;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">506</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.set(&quot;Vary&quot;, &quot;Accept-Encoding&quot;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">507</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">508</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.success(&quot;text&#x2F;html; charset=utf-8&quot;);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">509</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">  res.writeP(static_index_0, 1045);</span>
                  </div>
                </td>
              </tr>
              <tr>
                <td class="d2h-code-linenumber d2h-ins">
                  <div class="line-num1"></div>
                  <div class="line-num2">510</div>
                </td>
                <td class="d2h-ins">
                  <div class="d2h-code-line d2h-ins">
                    <span class="d2h-code-line-prefix">+</span>
                    <span class="d2h-code-line-ctn">}</span>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</div>
<p>
  You can now upload your sketch. Point your browser to the device IP that is displayed when it boots up. With any luck, in a few hundred milliseconds you should see the same page as before, but now completely hosted on the microcontroller.
</p>
<p>
  <img src="../../images/tutorial/ready.jpg" alt="Screen shot of the ready app in a browser." />
</p>
<p>
  That's it!.
</p>