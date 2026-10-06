<ul id="menu">
  <li id="app-api"><a href="#app">App</a><ul id="app-menu">
    <li><a href="#app-App">Construction and ownership</a></li>
    <li><a href="#app-get">App::get</a></li>
    <li><a href="#app-head">App::head</a></li>
    <li><a href="#app-post">App::post</a></li>
    <li><a href="#app-put">App::put</a></li>
    <li><a href="#app-del">App::del</a></li>
    <li><a href="#app-patch">App::patch</a></li>
    <li><a href="#app-options">App::options</a></li>
    <li><a href="#app-use">App::use</a></li>
    <li><a href="#app-memberCallbacks">Member callbacks</a></li>
    <li><a href="#app-notFound">App::notFound</a></li>
    <li><a href="#app-finally">App::finally</a></li>
    <li><a href="#app-onError">App::onError and ErrorHandler</a></li>
    <li><a href="#app-setTimeout">App::setTimeout</a></li>
    <li><a href="#app-process-options">App::ProcessOptions</a></li>
    <li><a href="#app-process-result">App::ProcessResult</a></li>
    <li><a href="#app-process">App::process</a></li>
    <li><a href="#app-strcmpi">App::strcmpi and App::strcmpiP</a></li>
    <li><a href="#app-header">Migrating App header registration</a></li>
  </ul></li>
  <li id="router-api"><a href="#router">Router</a><ul id="router-menu">
    <li><a href="#router-Router">Construction and ownership</a></li>
    <li><a href="#router-Middleware">Router::Middleware</a></li>
    <li><a href="#router-get">Router::get</a></li>
    <li><a href="#router-head">Router::head</a></li>
    <li><a href="#router-post">Router::post</a></li>
    <li><a href="#router-put">Router::put</a></li>
    <li><a href="#router-del">Router::del</a></li>
    <li><a href="#router-patch">Router::patch</a></li>
    <li><a href="#router-options">Router::options</a></li>
    <li><a href="#router-use">Router::use and matching</a></li>
    <li><a href="#router-memberCallbacks">Member callbacks</a></li>
  </ul></li>
  <li id="req-api"><a href="#req">Request</a><ul id="req-menu">
    <li><a href="#req-header">Request::Header</a></li>
    <li><a href="#req-context">Request::context</a></li>
    <li><a href="#req-client">Request::client</a></li>
    <li><a href="#req-methodType">Request::MethodType</a></li>
    <li><a href="#req-method">Request::method</a></li>
    <li><a href="#req-minorVersion">Request::minorVersion</a></li>
    <li><a href="#req-next">Request::next</a></li>
    <li><a href="#req-get">Request::get</a></li>
    <li><a href="#req-path">Request::path</a></li>
    <li><a href="#req-query">Request::query</a></li>
    <li><a href="#req-route">Request::route</a></li>
    <li><a href="#req-form">Request::form</a></li>
    <li><a href="#req-left">Request::left</a></li>
    <li><a href="#req-available">Request::available</a></li>
    <li><a href="#req-read">Request::read</a></li>
    <li><a href="#req-peek">Request::peek</a></li>
    <li><a href="#req-push">Request::push</a></li>
    <li><a href="#req-bytesRead">Request::bytesRead</a></li>
    <li><a href="#req-availableForWrite">Request::availableForWrite</a></li>
    <li><a href="#req-write">Request::write</a></li>
    <li><a href="#req-flush">Request::flush</a></li>
    <li><a href="#req-inherited">Inherited Arduino interfaces</a></li>
    <li><a href="#req-setTimeout">Request::setTimeout</a></li>
    <li><a href="#req-find">Request::find</a></li>
    <li><a href="#req-findUntil">Request::findUntil</a></li>
    <li><a href="#req-parseInt">Request::parseInt</a></li>
    <li><a href="#req-parseFloat">Request::parseFloat</a></li>
    <li><a href="#req-readBytes">Request::readBytes</a></li>
    <li><a href="#req-readBytesUntil">Request::readBytesUntil</a></li>
    <li><a href="#req-readString">Request::readString</a></li>
    <li><a href="#req-readStringUntil">Request::readStringUntil</a></li>
    <li><a href="#req-print">Request::print</a></li>
    <li><a href="#req-println">Request::println</a></li>
    <li><a href="#req-stream">Migrating Request stream access</a></li>
  </ul></li>
  <li id="res-api"><a href="#res">Response</a><ul id="res-menu">
    <li><a href="#res-status">Response::status</a></li>
    <li><a href="#res-statusSent">Response::statusSent</a></li>
    <li><a href="#res-sendStatus">Response::sendStatus</a></li>
    <li><a href="#res-set">Response::set</a></li>
    <li><a href="#res-get">Response::get</a></li>
    <li><a href="#res-setDefaults">Response::setDefaults</a></li>
    <li><a href="#res-beginHeaders">Response::beginHeaders</a></li>
    <li><a href="#res-endHeaders">Response::endHeaders</a></li>
    <li><a href="#res-headersSent">Response::headersSent</a></li>
    <li><a href="#res-end">Response::end</a></li>
    <li><a href="#res-ended">Response::ended</a></li>
    <li><a href="#res-keepOpen">Response::keepOpen</a></li>
    <li><a href="#res-write">Response::write</a></li>
    <li><a href="#res-availableForWrite">Response::availableForWrite</a></li>
    <li><a href="#res-flush">Response::flush</a></li>
    <li><a href="#res-bytesSent">Response::bytesSent</a></li>
    <li><a href="#res-printP">Response::printP</a></li>
    <li><a href="#res-writeP">Response::writeP</a></li>
    <li><a href="#res-print">Response::print</a></li>
    <li><a href="#res-println">Response::println</a></li>
    <li><a href="#res-writeError">Inherited write errors</a></li>
  </ul></li>
  <li id="ws-api"><a href="#ws">WsChannel</a><ul id="ws-menu">
    <li><a href="#ws-constructor">Constructor and capacities</a></li>
    <li><a href="#ws-upgrade">upgrade</a></li>
    <li><a href="#ws-poll">poll and frame handling</a></li>
    <li><a href="#ws-rooms">Rooms</a></li>
    <li><a href="#ws-send">Sending</a></li>
    <li><a href="#ws-state">Connection state and closure</a></li>
    <li><a href="#ws-callbacks">Callbacks</a></li>
  </ul></li>
  <li id="wsmessage-api"><a href="#wsmessage">WsMessage</a><ul id="wsmessage-menu">
    <li><a href="#wsmessage-constructor">Constructor and opcodes</a></li>
    <li><a href="#wsmessage-read">Reading and Stream methods</a></li>
    <li><a href="#wsmessage-metadata">Metadata and direct payload access</a></li>
  </ul></li>
  <li id="sse-api"><a href="#sse">SseChannel</a><ul id="sse-menu">
    <li><a href="#sse-constructor">Constructor and capacity</a></li>
    <li><a href="#sse-open">open</a></li>
    <li><a href="#sse-add">add</a></li>
    <li><a href="#sse-send">send</a></li>
    <li><a href="#sse-foreach">forEach and onConnect</a></li>
  </ul></li>
  <li id="storage-api"><a href="#storage">Storage and StreamClient</a><ul id="storage-menu">
    <li><a href="#storage-arrayview">ArrayView</a></li>
    <li><a href="#storage-streamclient">StreamClient</a></li>
  </ul></li>
  <li id="configuration-api"><a href="#configuration">Configuration</a><ul id="configuration-menu">
    <li><a href="#configuration-namespace">Headers and namespace</a></li>
    <li><a href="#configuration-callbacks">Callback selection</a></li>
    <li><a href="#configuration-http">HTTP storage macros</a></li>
    <li><a href="#configuration-flash">P and flash strings</a></li>
    <li><a href="#configuration-channels">Channel macros</a></li>
  </ul></li>
</ul>
