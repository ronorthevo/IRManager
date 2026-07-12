/* ============================================================
   IRManager — app.js
   Lógica de la interfaz web. Se comunica con la REST API
   del ESP32. Vanilla JS, sin dependencias externas.
   ============================================================ */

'use strict';

// ── Estado global de la app ──────────────────────────────────
const App = {
  selectedDevice: null,
  learnPollTimer: null,
  statusTimer:    null,
  confirmCallback: null,
};

// ── Mapa de iconos por nombre de botón ──────────────────────
const BTN_ICONS = {
  power:'⏻', on:'⏻', off:'⏻',
  'vol+':'🔊', 'vol-':'🔉', volup:'🔊', voldown:'🔉',
  mute:'🔇', silence:'🔇',
  ch:'📺', 'ch+':'⬆', 'ch-':'⬇', channelup:'⬆', channeldown:'⬇',
  menu:'☰', home:'🏠', back:'⬅', exit:'✕',
  ok:'✔', enter:'✔', select:'✔',
  up:'▲', down:'▼', left:'◀', right:'▶',
  play:'▶', pause:'⏸', stop:'⏹', rec:'⏺', record:'⏺',
  prev:'⏮', next:'⏭', rew:'⏪', ff:'⏩',
  'fan+':'💨', 'fan-':'🌬', fan:'💨',
  cool:'❄', heat:'🌡', auto:'♻', dry:'💧',
  timer:'⏱', sleep:'💤', swing:'↕',
  bright:'☀', dim:'🌑', color:'🎨',
  input:'↪', source:'↪', hdmi:'📲',
  info:'ℹ', guide:'📋',
};

function getBtnIcon(name) {
  const key = name.toLowerCase().replace(/\s+/g,'');
  return BTN_ICONS[key] || '●';
}

// ── API helper ───────────────────────────────────────────────
async function api(method, path, body = null) {
  const opts = { method, headers: { 'Content-Type': 'application/json' } };
  if (body) opts.body = JSON.stringify(body);
  try {
    const res = await fetch('/api' + path, opts);
    const text = await res.text();
    try { return { ok: res.ok, status: res.status, data: JSON.parse(text) }; }
    catch { return { ok: res.ok, status: res.status, data: { message: text } }; }
  } catch (err) {
    return { ok: false, status: 0, data: { message: 'Sin conexión: ' + err.message } };
  }
}

// ── Toast ────────────────────────────────────────────────────
function toast(msg, type = 'info', ms = 3000) {
  const el = document.createElement('div');
  el.className = `toast ${type}`;
  el.textContent = msg;
  document.getElementById('toast-container').appendChild(el);
  setTimeout(() => {
    el.style.animation = 'toastOut 0.3s ease forwards';
    setTimeout(() => el.remove(), 320);
  }, ms);
}

// ── Status ───────────────────────────────────────────────────
async function refreshStatus() {
  const { ok, data } = await api('GET', '/status');
  if (!ok) return;

  // Versión
  document.getElementById('fw-version').textContent = 'v' + (data.version || '');

  // WiFi pill
  const pill  = document.getElementById('wifi-indicator');
  const label = document.getElementById('wifi-label');
  pill.className = 'status-pill';
  if (data.wifi_mode === 'sta') {
    pill.classList.add('connected');
    label.textContent = data.wifi_ssid + ' ' + (data.wifi_rssi || '') + 'dBm';
    document.getElementById('footer-ip').textContent = '🌐 ' + data.wifi_ip;
  } else if (data.wifi_mode === 'ap') {
    pill.classList.add('ap');
    label.textContent = 'AP: ' + data.wifi_ssid;
    document.getElementById('footer-ip').textContent = '📡 ' + data.wifi_ip;
  } else {
    pill.classList.add('disconnected');
    label.textContent = 'Sin WiFi';
  }

  // Heap
  const heapFree = Math.round(data.heap_free / 1024);
  document.getElementById('heap-label').textContent = `Heap: ${heapFree}KB`;

  // FS
  if (data.fs_total > 0) {
    const fsFreeKB  = Math.round(data.fs_free  / 1024);
    const fsTotalKB = Math.round(data.fs_total / 1024);
    document.getElementById('fs-label').textContent = `FS: ${fsFreeKB}/${fsTotalKB}KB`;
  }

  // Uptime
  const u = data.uptime_s || 0;
  const h = Math.floor(u / 3600), m = Math.floor((u % 3600) / 60), s = u % 60;
  document.getElementById('footer-uptime').textContent =
    `⏱ ${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}:${String(s).padStart(2,'0')}`;
}

// ── Dispositivos ─────────────────────────────────────────────
async function loadDevices() {
  const { ok, data } = await api('GET', '/devices');
  if (!ok) { toast('Error cargando dispositivos', 'error'); return; }

  const list  = document.getElementById('device-list');
  const count = document.getElementById('device-count');
  list.innerHTML = '';
  count.textContent = data.count || 0;

  if (!data.devices || data.devices.length === 0) {
    list.innerHTML = '<li style="padding:12px 10px;color:var(--text-muted);font-size:12px">Sin dispositivos</li>';
    return;
  }

  for (const dev of data.devices) {
    const li = document.createElement('li');
    li.className = 'device-item' + (dev.name === App.selectedDevice ? ' active' : '');
    li.innerHTML = `<span class="device-name">${esc(dev.name)}</span>
                    <span class="device-count">${dev.buttons}</span>`;
    li.onclick = () => selectDevice(dev.name);
    list.appendChild(li);
  }

  // Si había un dispositivo seleccionado, recargarlo
  if (App.selectedDevice) selectDevice(App.selectedDevice);
}

// ── Seleccionar dispositivo ──────────────────────────────────
async function selectDevice(name) {
  App.selectedDevice = name;

  // Actualizar sidebar
  document.querySelectorAll('.device-item').forEach(li => {
    li.classList.toggle('active', li.querySelector('.device-name')?.textContent === name);
  });

  const { ok, data } = await api('GET', `/device/${encodeURIComponent(name)}/buttons`);
  if (!ok) { toast('Error cargando botones', 'error'); return; }

  document.getElementById('empty-state').classList.add('hidden');
  const panel = document.getElementById('device-panel');
  panel.classList.remove('hidden');

  document.getElementById('panel-device-name').textContent = name;
  document.getElementById('panel-btn-count').textContent =
    (data.count || 0) + ' botones';

  renderButtons(name, data.buttons || []);
}

// ── Renderizar botones ───────────────────────────────────────
function renderButtons(device, buttons) {
  const grid = document.getElementById('button-grid');
  grid.innerHTML = '';

  if (buttons.length === 0) {
    grid.innerHTML = '<p style="color:var(--text-muted);font-size:13px;padding:20px 0">Sin botones. Aprende el primero ↗</p>';
    return;
  }

  for (const btn of buttons) {
    const card = document.createElement('div');
    card.className = 'ir-button-card';
    card.setAttribute('data-device', device);
    card.setAttribute('data-button', btn);
    card.innerHTML = `
      <div class="ir-btn-icon">${getBtnIcon(btn)}</div>
      <div class="ir-btn-name">${esc(btn)}</div>
      <div class="ir-btn-actions">
        <button class="btn btn-success btn-xs"
                onclick="sendBtn(event,'${esc(device)}','${esc(btn)}')">▶</button>
        <button class="btn btn-danger btn-xs"
                onclick="deleteBtn(event,'${esc(device)}','${esc(btn)}')">✕</button>
      </div>`;

    // Click en la tarjeta (no en botones) → enviar señal
    card.addEventListener('click', (e) => {
      if (e.target.tagName === 'BUTTON') return;
      sendBtn(e, device, btn);
    });

    grid.appendChild(card);
  }
}

// ── Enviar señal ─────────────────────────────────────────────
async function sendBtn(e, device, button) {
  e.stopPropagation();
  const { ok, data } = await api('POST', '/send', { device, button });
  if (ok) toast(`✔ ${device} / ${button}`, 'success', 2000);
  else    toast(`Error enviando: ${data.message || ''}`, 'error');
}

// ── Borrar botón ─────────────────────────────────────────────
function deleteBtn(e, device, button) {
  e.stopPropagation();
  openConfirm(
    'Eliminar botón',
    `¿Eliminar "${button}" de "${device}"?`,
    async () => {
      const { ok, data } =
        await api('DELETE', `/button/${encodeURIComponent(device)}/${encodeURIComponent(button)}`);
      if (ok) {
        toast(`Eliminado: ${button}`, 'success');
        await loadDevices();
      } else {
        toast(`Error: ${data.message}`, 'error');
      }
    }
  );
}

// ── Learn modal ──────────────────────────────────────────────
function openLearnModal() {
  if (App.selectedDevice) {
    document.getElementById('learn-device').value = App.selectedDevice;
  }
  document.getElementById('learn-modal').classList.remove('hidden');
  showLearnForm();
}

function closeLearnModal() {
  if (App.learnPollTimer) clearInterval(App.learnPollTimer);
  document.getElementById('learn-modal').classList.add('hidden');
  resetLearnModal();
}

function showLearnForm() {
  document.getElementById('learn-form').classList.remove('hidden');
  document.getElementById('learn-waiting').classList.add('hidden');
  document.getElementById('learn-result').classList.add('hidden');
}

function resetLearnModal() { showLearnForm(); }

async function startLearn() {
  const device = document.getElementById('learn-device').value.trim();
  const button = document.getElementById('learn-button').value.trim();
  if (!device || !button) {
    toast('Rellena dispositivo y botón', 'error'); return;
  }

  const { ok, data } = await api('POST', '/learn', { device, button });
  if (!ok) { toast('Error: ' + (data.message || ''), 'error'); return; }

  document.getElementById('learn-form').classList.add('hidden');
  document.getElementById('learn-waiting').classList.remove('hidden');
  document.getElementById('learn-hint-text').textContent =
    `Apunta el mando y pulsa "${button}"…`;

  // Sondear /api/status hasta que last_learn cambie
  App.learnPollTimer = setInterval(async () => {
    const { ok, data } = await api('GET', '/status');
    if (!ok || data.learn_pending) return;

    clearInterval(App.learnPollTimer);
    document.getElementById('learn-waiting').classList.add('hidden');
    const resultDiv = document.getElementById('learn-result');
    resultDiv.classList.remove('hidden');

    const success = data.last_learn === 'ok';
    document.getElementById('learn-result-icon').textContent = success ? '✅' : '❌';
    document.getElementById('learn-result-text').textContent =
      success ? `Señal "${button}" guardada en "${device}"` : 'Error al guardar la señal';

    if (success) {
      toast(`Guardado: ${device} / ${button}`, 'success');
      await loadDevices();
      if (App.selectedDevice === device) selectDevice(device);
    }
  }, 800);
}

function cancelLearn() {
  if (App.learnPollTimer) clearInterval(App.learnPollTimer);
  closeLearnModal();
}

// ── WiFi modal ───────────────────────────────────────────────
async function openWifiModal() {
  document.getElementById('wifi-modal').classList.remove('hidden');
  // Mostrar estado actual en el card
  const { ok, data } = await api('GET', '/status');
  if (ok) {
    let info = '';
    if (data.wifi_mode === 'sta') {
      info = `✅ Conectado a: ${data.wifi_ssid}\nIP: ${data.wifi_ip}\nRSSI: ${data.wifi_rssi} dBm`;
    } else if (data.wifi_mode === 'ap') {
      info = `📡 Modo AP: ${data.wifi_ssid}\nIP: ${data.wifi_ip}\nIntroduce credenciales para conectar al router.`;
    } else {
      info = '⚠ Sin conexión WiFi.';
    }
    const card = document.getElementById('wifi-status-detail');
    card.textContent = info;
  }
}

function closeWifiModal() {
  document.getElementById('wifi-modal').classList.remove('hidden');
  document.getElementById('wifi-modal').classList.add('hidden');
}

async function saveWifi() {
  const ssid = document.getElementById('wifi-ssid').value.trim();
  const pass  = document.getElementById('wifi-pass').value;
  if (!ssid || !pass) { toast('Rellena SSID y contraseña', 'error'); return; }

  // Usamos el comando via consola (no hay endpoint POST /wifi en la API).
  // El endpoint /api/send no aplica, así que llamamos a un endpoint especial.
  // Por ahora mostramos instrucción de consola como fallback.
  toast('Usa la consola: wifi ' + ssid + ' <password>', 'info', 5000);
  closeWifiModal();
}

// ── Confirm modal ────────────────────────────────────────────
function openConfirm(title, text, onOk) {
  App.confirmCallback = onOk;
  document.getElementById('confirm-title').textContent = title;
  document.getElementById('confirm-text').textContent  = text;
  document.getElementById('confirm-modal').classList.remove('hidden');
  document.getElementById('btn-confirm-ok').onclick = async () => {
    closeConfirm();
    if (App.confirmCallback) await App.confirmCallback();
  };
}

function closeConfirm() {
  document.getElementById('confirm-modal').classList.add('hidden');
  App.confirmCallback = null;
}

// ── Utilidades ───────────────────────────────────────────────
function esc(str) {
  return String(str)
    .replace(/&/g,'&amp;').replace(/</g,'&lt;')
    .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

// ── Arranque ─────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', async () => {
  await refreshStatus();
  await loadDevices();

  // Actualizar status cada 8 s
  App.statusTimer = setInterval(refreshStatus, 8000);
});
