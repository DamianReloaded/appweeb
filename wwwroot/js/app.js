function UploadFile(path, bytes, success, failure)
{
    var request = new XMLHttpRequest();

    request.open("POST", "/api/upload", true);
    request.setRequestHeader("X-Path", path);
    request.setRequestHeader("Content-Type", "application/octet-stream");

    request.onreadystatechange = function()
    {
        if (request.readyState !== 4)
        {
            return;
        }

        try
        {
            success(JSON.parse(request.responseText));
        }
        catch (e)
        {
            failure(request.responseText);
        }
    };

    request.onerror = function()
    {
        failure("Network error");
    };

    request.send(bytes);
}

function SetResult(text)
{
    document.getElementById("result").textContent = text;
}

function LoadSettings()
{
    var output = document.getElementById("settings");

    var request = new XMLHttpRequest();

    request.open("GET", "data/settings.json", true);

    request.onreadystatechange = function()
    {
        if (request.readyState !== 4)
        {
            return;
        }

        if (request.status !== 200)
        {
            output.textContent = "settings.json not found";
            return;
        }

        output.textContent = request.responseText;
    };

    request.onerror = function()
    {
        output.textContent = "Network error";
    };

    request.send();
}

function StringToUtf8ArrayBuffer(text)
{
    var utf8 = unescape(encodeURIComponent(text));
    var bytes = new Uint8Array(utf8.length);

    var i;

    for (i = 0; i < utf8.length; i++)
    {
        bytes[i] = utf8.charCodeAt(i);
    }

    return bytes.buffer;
}

document.getElementById("uploadButton").addEventListener("click", function()
{
    var button = document.getElementById("uploadButton");
    var fileInput = document.getElementById("uploadFile");

    if (fileInput.files.length === 0)
    {
        SetResult("Please select a file.");
        return;
    }

    var file = fileInput.files[0];

    var reader = new FileReader();

    reader.onload = function()
    {
        var path = document.getElementById("uploadPath").value.replace(/^\s+|\s+$/g, "");

        if (path === "")
        {
            path = "uploads/" + file.name;
        }

        button.disabled = true;
        SetResult("Uploading...");

        UploadFile(
            path,
            reader.result,
            function(result)
            {
                button.disabled = false;
                SetResult(JSON.stringify(result, null, 4));
            },
            function(error)
            {
                button.disabled = false;
                SetResult(error.toString());
            });
    };

    reader.onerror = function()
    {
        SetResult("Failed to read file.");
    };

    reader.readAsArrayBuffer(file);
});

document.getElementById("uploadFile").addEventListener("change", function(event)
{
    var file = event.target.files[0];

    if (!file)
    {
        return;
    }

    document.getElementById("uploadPath").value = "uploads/" + file.name;
});

document.getElementById("writeButton").addEventListener("click", function()
{
    try
    {
        var path = document.getElementById("path").value;
        var jsonText = document.getElementById("json").value;

        JSON.parse(jsonText);

        var bytes = StringToUtf8ArrayBuffer(jsonText);

        UploadFile(
            path,
            bytes,
            function(result)
            {
                SetResult(JSON.stringify(result, null, 4));
                LoadSettings();
            },
            function(error)
            {
                SetResult(error.toString());
            });
    }
    catch (error)
    {
        SetResult(error.toString());
    }
});

window.addEventListener("load", function()
{
    LoadSettings();
});