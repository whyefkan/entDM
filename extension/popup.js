const BRIDGE = "http://127.0.0.1:8765";
const dot = document.getElementById("dot");
const statusEl = document.getElementById("status");
const msg = document.getElementById("msg");
const urlInput = document.getElementById("url");
const sendBtn = document.getElementById("send");
const intercept = document.getElementById("intercept");

async function checkBridge() {
  try {
    const res = await fetch(BRIDGE + "/ping", { cache: "no-store" });
    const ok = res.ok;
    dot.classList.toggle("on", ok);
    statusEl.textContent = ok ? "connected" : "offline";
    sendBtn.disabled = !ok;
    return ok;
  } catch (e) {
    dot.classList.remove("on");
    statusEl.textContent = "offline";
    sendBtn.disabled = true;
    return false;
  }
}

async function loadSettings() {
  const settings = await chrome.storage.sync.get({ intercept: true });
  intercept.checked = !!settings.intercept;
}

intercept.addEventListener("change", () => {
  chrome.storage.sync.set({ intercept: intercept.checked });
});

sendBtn.addEventListener("click", async () => {
  const url = urlInput.value.trim();
  if (!url) {
    return;
  }
  msg.textContent = "sending...";
  msg.style.color = "#9ca3af";
  try {
    const res = await fetch(BRIDGE + "/add", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ url })
    });
    if (!res.ok) {
      throw new Error("bad status");
    }
    msg.textContent = "Sent to entDM";
    msg.style.color = "#22c55e";
    urlInput.value = "";
  } catch (e) {
    msg.textContent = "Could not reach entDM";
    msg.style.color = "#ef4444";
  }
});

urlInput.addEventListener("keydown", (event) => {
  if (event.key === "Enter") {
    sendBtn.click();
  }
});

checkBridge();
loadSettings();
setInterval(checkBridge, 2000);
