
async function UploadFile(
    path,
    file)
{
    console.log(
        "Uploading:",
        file);

    console.log(
        "Name:",
        file.name);

    console.log(
        "Size:",
        file.size);

    const bytes = await file.arrayBuffer();

    const response =
        await fetch(
            "/api/write-file",
            {
                method: "POST",
                headers:
                {
                    "X-Path":
                        path,

                    "Content-Type":
                        "application/octet-stream"
                },
                body: bytes
            });

    const text =
        await response.text();

    try
    {
        return JSON.parse(
            text);
    }
    catch
    {
        throw new Error(
            text);
    }
}

async function WriteJson(
    path,
    jsonText)
{
    const response =
        await fetch(
            "/api/write-json",
            {
                method: "POST",
                headers:
                {
                    "Content-Type":
                        "application/json"
                },
                body: JSON.stringify(
                {
                    path: path,
                    json: jsonText
                })
            });

    return await response.json();
}

function SetResult(text)
{
    document
        .getElementById("result")
        .textContent = text;
}

async function LoadSettings()
{
    const output =
        document.getElementById(
            "settings");

    try
    {
        const response =
            await fetch(
                "/data/settings.json");

        if (!response.ok)
        {
            output.textContent =
                "settings.json not found";

            return;
        }

        const text =
            await response.text();

        output.textContent =
            text;
    }
    catch (error)
    {
        output.textContent =
            error.toString();
    }
}

document
    .getElementById("uploadButton")
    .addEventListener(
        "click",
        async () =>
        {
            try
            {
                const button =
                    document.getElementById(
                        "uploadButton");

                const fileInput =
                    document.getElementById(
                        "uploadFile");

                if (
                    fileInput.files.length === 0)
                {
                    SetResult(
                        "Please select a file.");

                    return;
                }

                const file =
                    fileInput.files[0];

                let path =
                    document
                        .getElementById(
                            "uploadPath")
                        .value
                        .trim();

                if (!path)
                {
                    path =
                        "uploads/" +
                        file.name;
                }

                button.disabled = true;

                SetResult(
                    "Uploading...");

                const result =
                    await UploadFile(
                        path,
                        file);

                SetResult(
                    JSON.stringify(
                        result,
                        null,
                        4));
            }
            catch (error)
            {
                console.error(
                    error);

                SetResult(
                    error.toString());
            }
            finally
            {
                document
                    .getElementById(
                        "uploadButton")
                    .disabled = false;
            }
        });

document
    .getElementById("uploadFile")
    .addEventListener(
        "change",
        event =>
        {
            const file =
                event.target.files[0];

            if (!file)
            {
                return;
            }

            document
                .getElementById(
                    "uploadPath")
                .value =
                    "uploads/" +
                    file.name;
        });

document
    .getElementById("writeButton")
    .addEventListener(
        "click",
        async () =>
        {
            try
            {
                const path =
                    document
                        .getElementById(
                            "path")
                        .value;

                const jsonText =
                    document
                        .getElementById(
                            "json")
                        .value;

                JSON.parse(
                    jsonText);

                const result =
                    await WriteJson(
                        path,
                        jsonText);

                SetResult(
                    JSON.stringify(
                        result,
                        null,
                        4));

                await LoadSettings();
            }
            catch (error)
            {
                SetResult(
                    error.toString());
            }
        });

document
    .getElementById("largeButton")
    .addEventListener(
        "click",
        () =>
        {
            const data =
            {
                name: "Large Test",
                values: []
            };

            while (
                JSON.stringify(data)
                    .length <
                5 * 1024 * 1024)
            {
                data.values.push(
                {
                    id:
                        data.values.length,

                    text:
                        "abcdefghijklmnopqrstuvwxyz0123456789",

                    timestamp:
                        Date.now()
                });
            }

            document
                .getElementById(
                    "json")
                .value =
                    JSON.stringify(
                        data,
                        null,
                        4);

            SetResult(
                "Generated approximately 5 MB JSON document.");
        });

window.addEventListener(
    "load",
    async () =>
    {
        await LoadSettings();
    });