#!/usr/bin/env python3
"""Exercise a built native HelloWorld executable over HTTP using Python's stdlib.

Native Windows: python tests/http_smoke.py build/HelloWorld.exe
Cross-built exe: python tests/http_smoke.py build/HelloWorld.exe --runner wine
Native Linux: python ../WindowsNative/tests/http_smoke.py build/HelloWorld
"""

import argparse
import concurrent.futures
import http.client
import os
import queue
import re
import signal
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path


LISTENING = re.compile(r"Server listening on port (\d+)\b")
EXPECTED_BODY = (
    b"<html>\n"
    b"<head>\n"
    b"<title>Hello World!</title>\n"
    b"</head>\n"
    b"<body>\n"
    b"<h1>Greetings middle earth!</h1>\n"
    b"</body>\n"
    b"</html>"
)


def check(condition, message):
    if not condition:
        raise AssertionError(message)


def spawn(command):
    options = {}
    if os.name != "nt":
        # Terminate a runner and any child executable together on POSIX.
        options["start_new_session"] = True
    return subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        **options,
    )


def stop(process):
    if process.poll() is None:
        if os.name == "nt":
            process.terminate()
        else:
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            if os.name == "nt":
                process.kill()
            else:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
            process.wait(timeout=5)


def read_output(process, lines, events):
    for line in process.stdout:
        lines.append(line)
        events.put(line)


def wait_started(process, events):
    deadline = time.monotonic() + 15
    while time.monotonic() < deadline:
        check(process.poll() is None, "HelloWorld exited before it became ready")
        try:
            line = events.get(timeout=0.1)
        except queue.Empty:
            continue
        match = LISTENING.search(line)
        if match:
            port = int(match.group(1))
            check(0 < port <= 65535, "The ephemeral listening port is invalid")
            return port
    raise AssertionError("HelloWorld did not report a listening port within 15 seconds")


def request(port, path="/", barrier=None):
    connection = http.client.HTTPConnection("127.0.0.1", port, timeout=5)
    try:
        if barrier is not None:
            barrier.wait(timeout=5)
        connection.request("GET", path, headers={"Connection": "close"})
        response = connection.getresponse()
        body = response.read()
        return response.status, response.getheader("Content-Type"), body
    finally:
        connection.close()


def check_index(response):
    status, content_type, body = response
    check(status == 200, f"GET / returned HTTP {status}, expected 200")
    check(content_type == "text/html", f"Unexpected Content-Type: {content_type!r}")
    check(body == EXPECTED_BODY, f"Unexpected HelloWorld response body: {body!r}")


def trickle_header(connection, stopped, errors):
    # Stream timeouts restart after each byte. Keep an unfinished unknown
    # header active until the complete requests have finished.
    while not stopped.wait(0.1):
        try:
            connection.sendall(b"x")
        except OSError as error:
            errors.append(error)
            return


def check_port_released(port):
    deadline = time.monotonic() + 3
    while time.monotonic() < deadline:
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=0.2):
                pass
        except OSError:
            return
        time.sleep(0.05)
    raise AssertionError("HelloWorld still accepts connections after process cleanup")


def check_bind_failure(command):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as occupied:
        if hasattr(socket, "SO_EXCLUSIVEADDRUSE"):
            occupied.setsockopt(socket.SOL_SOCKET, socket.SO_EXCLUSIVEADDRUSE, 1)
        occupied.bind(("0.0.0.0", 0))
        occupied.listen(1)
        port = occupied.getsockname()[1]
        process = spawn(command + ["--port", str(port)])
        try:
            output, _ = process.communicate(timeout=15)
            check(process.returncode != 0, "HelloWorld succeeded while its port was occupied")
            check(
                LISTENING.search(output) is None,
                "HelloWorld reported readiness despite its port being occupied",
            )
        except subprocess.TimeoutExpired as error:
            raise AssertionError("HelloWorld did not exit after failing to bind") from error
        finally:
            stop(process)
            process.stdout.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--runner", help="Optional executable runner, such as wine")
    args = parser.parse_args()
    executable = args.executable.resolve()
    check(executable.is_file(), f"Executable does not exist: {executable}")
    command = ([args.runner] if args.runner else []) + [str(executable)]

    # This verifies that startup failure reaches main()'s exit status, before
    # launching the working server on its own fresh ephemeral port.
    check_bind_failure(command)

    process = spawn(command + ["--port", "0"])
    lines = []
    events = queue.Queue()
    reader = threading.Thread(target=read_output, args=(process, lines, events), daemon=True)
    reader.start()
    port = None
    try:
        port = wait_started(process, events)
        check_index(request(port))
        status, _, _ = request(port, "/missing-route")
        check(status == 404, f"Missing route returned HTTP {status}, expected 404")

        # Keep an incomplete request active while complete requests arrive.
        # A sequential handler remains occupied beyond the quick requests'
        # deadlines; it cannot simply time out the first request and catch up.
        with socket.create_connection(("127.0.0.1", port), timeout=5) as slow:
            slow.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Slow: ")
            stopped = threading.Event()
            trickle_errors = []
            trickler = threading.Thread(
                target=trickle_header, args=(slow, stopped, trickle_errors), daemon=True
            )
            trickler.start()
            try:
                workers = 8
                barrier = threading.Barrier(workers)
                with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
                    futures = [pool.submit(request, port, "/", barrier) for _ in range(workers)]
                    for future in futures:
                        check_index(future.result(timeout=10))
                check(not trickle_errors, f"Incomplete request closed early: {trickle_errors}")
            finally:
                stopped.set()
                trickler.join(timeout=2)
        check(process.poll() is None, "HelloWorld exited while handling requests")
    except BaseException:
        # Include the executable's output when CTest reports a failure.
        if lines:
            print("HelloWorld output:\n" + "".join(lines), file=sys.stderr)
        raise
    finally:
        stop(process)
        reader.join(timeout=2)
        process.stdout.close()
    if port is not None:
        check_port_released(port)
    print("Native HTTP smoke test passed (200, HTML body, 404, concurrent requests, bind failure)")


if __name__ == "__main__":
    main()
