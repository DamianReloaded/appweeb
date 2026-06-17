# AppWeeb

AppWeeb is a lightweight desktop application framework built around a simple idea:

- Write your application entirely in HTML, CSS and JavaScript.
- Serve those files locally from a native C++ executable.
- Expose local APIs implemented in C++.
- Store application data using files.
- Avoid Electron, Node.js and large runtimes.

The executable contains a small HTTP server that serves files from a configurable web root and exposes local API endpoints to the frontend.

The browser becomes the user interface while C++ provides local services.

## Goals

- Small executable size
- No external runtime requirements
- Fast startup
- Simple deployment
- Cross-platform support
- Native access to local files
- Web technologies for UI development

## Supported Platforms

- Windows
- Linux

## Current Features

### Static File Hosting

Any file located within the configured web root can be served through HTTP.

Example layout:

```text
appweeb

index.html
app.js
style.css

images/
audio/
video/

data/
```

Requests:

```text
GET /
GET /app.js
GET /images/logo.png
GET /video/intro.mp4
```

Files are served directly from disk.

### Configurable Web Root

By default AppWeeb serves files from the directory containing the executable.

Example:

```text
appweeb
index.html
app.js
style.css
```

You may optionally create a `config.json` file beside the executable:

```json
{
    "wwwroot": "../../wwwroot"
}
```

The `wwwroot` value is resolved relative to the executable directory.

Example:

```text
bin/
    linux/
        debug/
            appweeb
            config.json

wwwroot/
    index.html
    app.js
    style.css
```

In this case all file serving and file-writing APIs operate relative to the configured web root instead of the executable directory.

If `config.json` does not exist, the `wwwroot` property is missing, or the configured path is invalid, AppWeeb falls back to the executable directory.

### Path Traversal Protection

Requests are restricted to files inside the configured web root.

Attempts such as:

```text
../../Windows/System32
```

or

```text
../../../etc/passwd
```

are rejected.

This protection remains in effect even when using a custom `wwwroot` directory.

### JSON File Writing API

Endpoint:

```text
POST /api/write-json
```

Request:

```json
{
    "path": "data/settings.json",
    "json": "{\"theme\":\"dark\"}"
}
```

Response:

```json
{
    "success": true
}
```

The specified file is written relative to the configured web root, replacing any existing content.

### Large Request Support

Requests are read using the HTTP `Content-Length` header.

This allows multi-megabyte payloads rather than being limited to a single socket receive operation.

### Cross-Platform Networking

AppWeeb includes platform-specific socket implementations for:

- Windows (WinSock2)
- Linux (POSIX sockets)

The public API remains platform-independent.

## Planned Features

### JSON File Reading

Endpoint:

```text
POST /api/read-json
```

Read arbitrary JSON files stored within the configured web root.

### Browser Auto Launch

Automatically open:

```text
http://localhost:8080
```

when the application starts.

### Additional APIs

Potential future endpoints:

```text
/api/read-json
/api/write-json
/api/list-files
/api/delete-file
/api/move-file
```

## Architecture

```text
Browser
    |
    | HTTP
    v
+-----------+
| AppWeeb   |
| WebServer |
+-----------+
      |
      +---- Static Files
      |
      +---- JSON APIs
```

## Why Use This?

AppWeeb is useful when building:

- Internal tools
- CRUD applications
- Dashboard software
- Inventory systems
- Media catalogs
- Personal productivity tools
- Offline desktop applications
- Configuration editors

Instead of:

```text
Electron
    + Chromium
    + Node.js
```

the application consists of:

```text
appweeb
html
css
js
```

which can significantly reduce deployment size and complexity.

## Building

Requirements:

- C++23 compiler
- GNU Make

### Linux

Debug build:

```bash
make PLATFORM=linux MODE=debug
```

Release build:

```bash
make PLATFORM=linux MODE=release
```

### Windows

Debug build:

```bash
make PLATFORM=windows MODE=debug
```

Release build:

```bash
make PLATFORM=windows MODE=release
```

### Clean

```bash
make clean
```

### Rebuild

```bash
make rebuild
```

## Build Output

Debug:

```text
bin/linux/debug/appweeb
bin/windows/debug/appweeb.exe
```

Release:

```text
bin/linux/release/appweeb
bin/windows/release/appweeb.exe
```

## VS Code

The repository includes VS Code configurations for:

- Linux IntelliSense
- Windows IntelliSense
- Linux debugging
- Windows debugging
- Linux build tasks
- Windows build tasks

The debugger automatically rebuilds the selected target before launching.

Select the active IntelliSense configuration using:

```text
Ctrl+Shift+P
C/C++: Select a Configuration
```

Select the desired debug target from:

```text
Run and Debug
```

Examples:

```text
Linux Debug
Linux Release
Windows Debug
Windows Release
```

## Example Workflow

Directory structure:

```text
appweeb

index.html
app.js
style.css

data/
    settings.json
```

JavaScript:

```javascript
await fetch(
    "/api/write-json",
    {
        method: "POST",
        headers:
        {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(
        {
            path: "data/settings.json",
            json: JSON.stringify(
            {
                theme: "dark",
                width: 1280,
                height: 720
            })
        })
    });
```

Result:

```text
data/settings.json
```

contains:

```json
{
    "theme": "dark",
    "width": 1280,
    "height": 720
}
```

## Project Status

AppWeeb is currently a minimal proof-of-concept focused on:

- HTTP serving
- Local file access
- JSON APIs
- Configurable web roots
- Cross-platform support
- Lightweight deployment

The long-term goal is to provide a compact foundation for desktop applications built with web technologies while keeping the native backend as small and straightforward as possible.