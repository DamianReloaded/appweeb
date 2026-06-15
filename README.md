# AppWeeb

AppWeeb is a lightweight desktop application framework built around a simple idea:

- Write your application entirely in HTML, CSS and JavaScript.
- Serve those files locally from a native C++ executable.
- Expose local APIs implemented in C++.
- Store data using files and SQLite.
- Avoid Electron, Node.js and large runtimes.

The executable contains a small HTTP server that serves files from the application's directory and exposes local API endpoints to the frontend.

The browser becomes the user interface while C++ provides local services.

## Goals

- Small executable size
- No external runtime requirements
- Fast startup
- Simple deployment
- Native access to local files and databases
- Web technologies for UI development

## Current Features

### Static File Hosting

Any file located within the application directory can be served through HTTP.

Example layout:

```text
appweeb.exe

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

### Path Traversal Protection

Requests are restricted to files inside the application directory.

Attempts such as:

```text
../../Windows/System32
```

are rejected.

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

The specified file is written to disk, replacing any existing content.

### Large Request Support

Requests are read using the HTTP `Content-Length` header.

This allows multi-megabyte payloads rather than being limited to a single socket receive operation.

## Planned Features

### SQLite API

A generic SQLite endpoint is planned.

Request:

```json
{
    "database": "data/app.db",
    "sql": "select * from Users"
}
```

Response:

```json
{
    "success": true,
    "columns":
    [
        "Id",
        "Name"
    ],
    "rows":
    [
        [1, "John"]
    ]
}
```

This allows the entire application backend to be implemented using SQLite and JavaScript.

### JSON File Reading

Endpoint:

```text
POST /api/read-json
```

Read arbitrary JSON files stored within the application directory.

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
/api/sql
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
      |
      +---- SQLite APIs
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
- SQLite-based applications

Instead of:

```text
Electron
    + Chromium
    + Node.js
```

the application consists of:

```text
appweeb.exe
html
css
js
sqlite
```

which can significantly reduce deployment size and complexity.

## Building

Requirements:

- C++23 compiler
- WinSock2
- GNU Make (or equivalent)

Build:

```bash
make
```

Clean:

```bash
make clean
```

Rebuild:

```bash
make rebuild
```

Output:

```text
appweeb.exe
```

## Example Workflow

Directory structure:

```text
appweeb.exe

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
- Simple API endpoints
- Lightweight deployment

The long-term goal is to provide a compact foundation for desktop applications built with web technologies while keeping the native backend as small and straightforward as possible.