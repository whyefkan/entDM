const BRIDGE = "http://127.0.0.1:8765";
const DEFAULTS = { intercept: true };

function getSettings() {
  return new Promise((resolve) => chrome.storage.sync.get(DEFAULTS, resolve));
}

async function bridgeAlive() {
  try {
    const res = await fetch(BRIDGE + "/ping", { cache: "no-store" });
    return res.ok;
  } catch (e) {
    return false;
  }
}

async function sendToApp(url, filename) {
  const res = await fetch(BRIDGE + "/add", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ url, filename: filename || "" })
  });
  if (!res.ok) {
    throw new Error("bridge error " + res.status);
  }
  return res.json();
}

function flash(text, color) {
  chrome.action.setBadgeBackgroundColor({ color });
  chrome.action.setBadgeText({ text });
  setTimeout(() => chrome.action.setBadgeText({ text: "" }), 2000);
}

chrome.runtime.onInstalled.addListener(() => {
  chrome.contextMenus.create({
    id: "send-to-entdm",
    title: "Send link to entDM",
    contexts: ["link", "video", "audio"]
  });
});

chrome.contextMenus.onClicked.addListener(async (info) => {
  const url = info.linkUrl || info.srcUrl;
  if (!url) {
    return;
  }
  try {
    await sendToApp(url, "");
    flash("OK", "#16a34a");
  } catch (e) {
    flash("!", "#dc2626");
  }
});

const handledDownloads = new Set();
const MAX_AGE_MS = 20000;

chrome.downloads.onCreated.addListener(async (item) => {
  if (!item || !item.url) {
    return;
  }
  if (!/^(https?|ftp):\/\//i.test(item.url)) {
    return;
  }
  if (item.byExtensionId) {
    return;
  }
  if (item.state && item.state !== "in_progress") {
    return;
  }

  const startedAt = Date.parse(item.startTime);
  if (Number.isFinite(startedAt) && Date.now() - startedAt > MAX_AGE_MS) {
    return;
  }

  if (handledDownloads.has(item.id)) {
    return;
  }
  handledDownloads.add(item.id);
  if (handledDownloads.size > 500) {
    handledDownloads.clear();
    handledDownloads.add(item.id);
  }

  const settings = await getSettings();
  if (!settings.intercept) {
    return;
  }
  if (!(await bridgeAlive())) {
    return;
  }

  const filename = (item.filename || "").split(/[\\/]/).pop();
  try {
    await sendToApp(item.url, filename);
  } catch (e) {
    flash("!", "#dc2626");
    return;
  }

  try {
    await chrome.downloads.cancel(item.id);
    await chrome.downloads.erase({ id: item.id });
  } catch (e) {
  }
  flash("OK", "#16a34a");
});
