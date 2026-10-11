"use strict";
(async () => {
  const select = document.getElementById("file-select");
  const status = document.getElementById("file-status");
  const code = document.getElementById("source-code");
  const raw = document.getElementById("raw-file");
  try {
    const response = await fetch("assets/source-manifest.json");
    if (!response.ok) throw new Error("File list could not be loaded.");
    const files = await response.json();
    select.replaceChildren();
    for (const file of files) {
      const option = document.createElement("option");
      option.value = file;
      option.textContent = file;
      select.append(option);
    }
    const requested = new URLSearchParams(location.search).get("file");
    select.value = files.includes(requested) ? requested : files[0];
    let requestNumber = 0;
    async function showFile() {
      const current = ++requestNumber;
      const file = select.value;
      if (!files.includes(file)) return;
      const path = "artifacts/" + file.split("/").map(encodeURIComponent).join("/");
      status.textContent = "Loading " + file;
      code.textContent = "";
      raw.href = path;
      const url = new URL(location.href);
      url.searchParams.set("file", file);
      history.replaceState(null, "", url);
      try {
        const result = await fetch(path);
        if (!result.ok) throw new Error("File could not be loaded.");
        const content = await result.text();
        if (current !== requestNumber) return;
        code.textContent = content;
        status.textContent = file;
      } catch (error) {
        if (current === requestNumber) status.textContent = error.message;
      }
    }
    select.addEventListener("change", showFile);
    await showFile();
  } catch (error) { status.textContent = error.message; }
})();
