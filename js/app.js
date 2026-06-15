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
    .getElementById("writeButton")
    .addEventListener(
        "click",
        async () =>
        {
            try
            {
                const path =
                    document
                        .getElementById("path")
                        .value;

                const jsonText =
                    document
                        .getElementById("json")
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
                .getElementById("json")
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