<h3 id='res-beginHeaders'>Response::beginHeaders</h3>
<h4 class='variant'>void beginHeaders();</h4>

Calling the function will send the headers set by [Response:set](/en/3x/api.html#res-set) or [Response:setDefaults](/en/3x/api.html#res-setDefaults). The function does not terminate the headers and enables additional headers to be sent by writing them manually to the response. Finalize the headers by calling [Response:endHeaders](/en/3x/api.html#res-endHeaders).

##### Example

```arduino
res.set("Content-Encoding", "gzip");
res.beginHeaders();
res.write("Some-Header: value\r\n");
res.endHeaders();
```
