!!! IMPORTANT !!!
- To be able to upload files you need to change pluginDir in config.json to point to the directory containing the plugins. For this repo the values often are "pluginDir":"bin/linux/debug/plugins" and "pluginDir":"bin/windows/debug/plugins" 
- Set your toolchain in the Makefiles (./Makefile and ./plugins/upload/Makefile)

# AppWeeb

AppWeeb is a lightweight desktop application framework built around a simple idea:

- Write your application entirely in HTML, CSS and JavaScript.
- Serve those files locally from a native C++ executable.
- Extend the server through native C++ plugins.
- Store application data using files.
- Avoid Electron, Node.js and large runtimes.

The executable contains a small HTTP server that serves files from a configurable web root and exposes local API endpoints to the frontend.

The browser becomes the user interface while C++ provides local services.

## Index

- [Goals](#goals)
- [Supported Platforms](#supported-platforms)
- [Current Features](#current-features)
  - [Static File Hosting](#static-file-hosting)
  - [Configuration](#configuration)
  - [Plugin System](#plugin-system)
  - [Built-in Upload Plugin](#built-in-upload-plugin)
  - [Cross-Platform](#cross-platform)
- [Configuration File](#configuration-file)
- [Why Use This?](#why-use-this)
- [Building](#building)
  - [Linux](#linux)
  - [Windows](#windows)
  - [Clean](#clean)
  - [Rebuild](#rebuild)
- [Build Output](#build-output)
- [Example Directory Layout](#example-directory-layout)
- [Project Status](#project-status)

## Goals

- Small executable size
- No external runtime requirements
- Fast startup
- Simple deployment
- Plugin-based extensibility
- Cross-platform support
- Native access to local files
- Web technologies for UI development

## Supported Platforms

- Windows
- Linux

## Current Features

### Static File Hosting

Any file located within the configured web root can be served through HTTP.

Example requests:

```text
GET /
GET /app.js
GET /images/logo.png
GET /video/intro.mp4
```

Files are served directly from disk.

### Configuration

AppWeeb loads its configuration from an optional `config.json` located beside the executable.

Configuration property names are case-insensitive.

Currently supported settings are:

| Property | Description |
|----------|-------------|
| `wwwroot` | Directory used for serving static files. |
| `plugindir` | Directory from which plugins are loaded. |
| `httpport` | HTTP server port. |

If `config.json` is missing, sensible defaults are used.

### Plugin System

AppWeeb now supports dynamically loaded plugins.

Plugins are compiled as:

- `.dll` on Windows
- `.so` on Linux

Each plugin implements the `IEndpoint` interface and exports two functions:

```cpp
extern "C"
{
    appweeb::IEndpoint* CreateEndpoint();
    void DestroyEndpoint(appweeb::IEndpoint*);
}
```

Every loaded plugin registers one or more HTTP endpoints that become available immediately when the server starts.

This allows application functionality to be added without modifying or rebuilding the main executable.

Typical plugin uses include:

- Database access
- File processing
- Image generation
- REST APIs
- Authentication
- Communication with external services
- Application-specific business logic

### Built-in Upload Plugin

The repository includes an upload plugin exposing:

```text
POST /api/upload
```

Example request:

```javascript
await fetch("/api/upload",
{
    method: "POST",
    headers:
    {
        "X-Path": "data/settings.json",
        "Content-Type": "application/octet-stream"
    },
    body: fileBytes
});
```

Response:

```json
{
    "success": true
}
```

The uploaded file is written relative to the configured `wwwroot`.

### Cross-Platform

Platform-specific implementations are provided for:

- Windows
  - WinSock
  - LoadLibrary
- Linux
  - POSIX sockets
  - dlopen

The public API remains platform independent.

## Configuration File

Example:

```json
{
    "wwwroot": "../../wwwroot",
    "plugindir": "../../plugins",
    "httpport": 8080
}
```

Example layout:

```text
bin/
    windows/
        debug/
            appweeb.exe
            config.json
            plugins/
                upload.dll

wwwroot/
    index.html
    app.js
    style.css
```

or

```text
bin/
    linux/
        debug/
            appweeb
            config.json
            plugins/
                upload.so

wwwroot/
    index.html
    app.js
    style.css
```

If omitted:

- `wwwroot` defaults to the executable directory.
- `plugindir` defaults to a `plugins` directory beside the executable.
- `httpport` defaults to the value specified when constructing `WebServer`.

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

Instead of shipping:

```text
Electron
    + Chromium
    + Node.js
```

an application can consist of:

```text
appweeb
plugins/
wwwroot/
html
css
js
```

This significantly reduces deployment size and complexity while keeping the backend fully native.

## Building

Requirements:

- C++23 compiler
- GNU Make

### Linux

Debug:

```bash
make PLATFORM=linux MODE=debug
```

Release:

```bash
make PLATFORM=linux MODE=release
```

### Windows

Debug:

```bash
make PLATFORM=windows MODE=debug
```

Release:

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

Executable:

```text
bin/linux/debug/appweeb
bin/windows/debug/appweeb.exe

bin/linux/release/appweeb
bin/windows/release/appweeb.exe
```

Plugins:

```text
bin/linux/debug/plugins/*.so
bin/windows/debug/plugins/*.dll

bin/linux/release/plugins/*.so
bin/windows/release/plugins/*.dll
```

## Example Directory Layout

```text
appweeb.exe
config.json

plugins/
    upload.dll

wwwroot/
    index.html
    app.js
    style.css

    data/
        settings.json
```

## Project Status

AppWeeb is currently a lightweight native framework focused on:

- HTTP static file serving
- Configurable web roots
- Plugin-based HTTP endpoints
- Native file access
- Built-in upload endpoint
- Cross-platform support
- Lightweight deployment

The long-term goal is to provide a compact foundation for desktop applications built with web technologies while allowing native functionality to be extended through dynamically loaded plugins.