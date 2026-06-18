async function UploadFile(path, bytes)
{
    const response = await fetch("/api/upload", {
        method: "POST",
        headers: {
            "X-Path": path,
            "Content-Type": "application/octet-stream"
        },
        body: bytes
    });
    const text = await response.text();
    try
    {
        return JSON.parse(text);
    }
    catch
    {
        throw new Error(text);
    }
}

function SetResult(text)
{
    document.getElementById("result").textContent = text;
}

async function LoadSettings()
{
    const output = document.getElementById("settings");
    try
    {
        const response = await fetch("/data/settings.json");
        if (!response.ok)
        {
            output.textContent = "settings.json not found";
            return;
        }
        const text = await response.text();
        output.textContent = text;
    }
    catch (error)
    {
        output.textContent = error.toString();
    }
}

document.getElementById("uploadButton").addEventListener("click", async () =>
{
    try
    {
        const button = document.getElementById("uploadButton");
        const fileInput = document.getElementById("uploadFile");
        if (fileInput.files.length === 0)
        {
            SetResult("Please select a file.");
            return;
        }
        const file = fileInput.files[0];
        const bytes = await file.arrayBuffer();
        let path = document.getElementById("uploadPath").value.trim();
        if (!path)
        {
            path = "uploads/" + file.name;
        }
        button.disabled = true;
        SetResult("Uploading...");
        const result = await UploadFile(path, bytes);
        SetResult(JSON.stringify(result, null, 4));
    }
    catch (error)
    {
        console.error(error);
        SetResult(error.toString());
    }
    finally
    {
        document.getElementById("uploadButton").disabled = false;
    }
});

document.getElementById("uploadFile").addEventListener("change", event =>
{
    const file = event.target.files[0];
    if (!file)
    {
        return;
    }
    document.getElementById("uploadPath").value = "uploads/" + file.name;
});

document.getElementById("writeButton").addEventListener("click", async () =>
{
    try
    {
        const path = document.getElementById("path").value;
        const jsonText = document.getElementById("json").value;
        JSON.parse(jsonText);
        const encoder = new TextEncoder();
        const bytes = encoder.encode(jsonText);
        const result = await UploadFile(path, bytes);
        SetResult(JSON.stringify(result, null, 4));
        await LoadSettings();
    }
    catch (error)
    {
        SetResult(error.toString());
    }
});

window.addEventListener("load", async () =>
{
    await LoadSettings();
});