# Native tests

These tests run aWOT on the host with an Arduino compatibility layer and
in-memory transports. No board or listening network socket is required.

## Find an example

`http/` contains small handlers, complete HTTP requests, and exact expected
responses. They test the public API through `App::process()` and also serve as
examples:

| Example | What it demonstrates |
| --- | --- |
| [route-params.cpp](http/route-params.cpp) | Read named and positional route parameters, including mounted routers. |
| [routing.cpp](http/routing.cpp) | Compose middleware and routers with `req.next()`. |
| [authentication.cpp](http/authentication.cpp) | Capture an authorization header and allow or reject a protected request. |
| [form-params.cpp](http/form-params.cpp) | Read form fields with bounded name/value storage. |
| [chunked-form.cpp](http/chunked-form.cpp) | Decode a chunked form request using the ordinary request API. |
| [keep-alive.cpp](http/keep-alive.cpp) | Consume a request body and use `ProcessResult` to manage connection reuse. |
| [server-sent-events.cpp](http/server-sent-events.cpp) | Open an SSE response, retain the client, and send an event. |
| [headers-reading.cpp](http/headers-reading.cpp) | Capture request headers into caller-owned buffers. |
| [headers-writing.cpp](http/headers-writing.cpp) | Set response headers and inspect HTTP chunk framing. |
| [process-options.cpp](http/process-options.cpp) | Supply per-request buffers, captured headers, and context. |
| [error-handler.cpp](http/error-handler.cpp) | Write a custom error response. |

`regression/` contains focused API, parser, buffer, transport-failure, timeout,
SSE, and WebSocket checks. Many of these also use `App::process()` to reach the
behavior under test. The folders separate readable HTTP examples from focused
regressions; they do not imply that every regression isolates one class.

Shared support stays in `mocks/` (in-memory clients and streams), `unittest/`
(assertions and test runner), and `epoxy/` (host Arduino compatibility).

## Build and run

From the repository root, with CMake 3.18 or newer and a C++11 compiler:

```sh
cmake -S .extras/test -B .extras/test/build
cmake --build .extras/test/build --parallel 4
ctest --test-dir .extras/test/build --output-on-failure --parallel 4

# Run either group, or one example.
ctest --test-dir .extras/test/build -L http --output-on-failure
ctest --test-dir .extras/test/build -L regression --output-on-failure
ctest --test-dir .extras/test/build -R '^route-params$' --output-on-failure
```

Both groups run by default, including through the existing `run_tests` target.
Each `.cpp` file builds as a separate executable. CTest names use the filename
without its extension; use distinct filenames across the two folders.

The CI configurations also exercise sanitizers and the Uno-sized buffers with
function-pointer middleware:

```sh
cmake -S .extras/test -B .extras/test/build/sanitized -DAWOT_ENABLE_SANITIZERS=ON
cmake --build .extras/test/build/sanitized --parallel 4
ctest --test-dir .extras/test/build/sanitized --output-on-failure --parallel 4

cmake -S .extras/test -B .extras/test/build/small \
  -DAWOT_ENABLE_SANITIZERS=ON \
  -DAWOT_NO_STD_FUNCTION_MIDDLEWARE=ON -DAWOT_SMALL_BUFFERS=ON
cmake --build .extras/test/build/small --parallel 4
ctest --test-dir .extras/test/build/small --output-on-failure --parallel 4
```

The member-function example is omitted when function-pointer middleware is
selected. All translation units must use the same configuration; let CMake
apply the definitions to the library and tests together.

## Add a test

Put application examples in `http/`. Keep the handler, raw request, and complete
expected response easy to read together. Assert the status line, headers, blank
line, and body, including framing when relevant. Prefer literal expected bytes
over constructing expectations with the code being tested. Check failed reads
and bounded parameter extraction in handlers so the examples can be reused.

Put boundary matrices, injected failures, internal state observations, and
protocol regressions in `regression/`. Keep shared test support outside both
groups. CMake discovers new `.cpp` files automatically and assigns their folder
name as a CTest label.
