// LoRaLink WebSocket client
(function () {
  const chat = document.getElementById('chat');
  const status = document.getElementById('status');
  const meta = document.getElementById('meta');
  const form = document.getElementById('form');
  const input = document.getElementById('input');
  let ws;
  const seen = new Set();

  function connect() {
    const host = location.hostname || '192.168.4.1';
    ws = new WebSocket('ws://' + host + '/ws');
    ws.onopen = () => {
      status.textContent = 'Bağlı';
      status.classList.add('ok');
    };
    ws.onclose = () => {
      status.textContent = 'Koptu — yeniden…';
      status.classList.remove('ok');
      setTimeout(connect, 2000);
    };
    ws.onmessage = (e) => {
      try {
        const m = JSON.parse(e.data);
        if (m.type === 'message') {
          append(m, m.dir === 'out');
        } else if (m.type === 'history' && Array.isArray(m.messages)) {
          chat.innerHTML = '';
          seen.clear();
          m.messages.forEach((msg) => append(msg, msg.dir === 'out'));
        } else if (m.type === 'status') {
          updateStatus(m);
        }
      } catch (_) {}
    };
  }

  function msgKey(m) {
    return (m.id || 0) + ':' + (m.ts || 0) + ':' + (m.text || '');
  }

  function append(m, out) {
    const key = msgKey(m);
    if (seen.has(key)) return;
    seen.add(key);

    const d = document.createElement('div');
    d.className = 'msg' + (out ? ' out' : ' in');
    d.textContent = m.text || '';
    chat.appendChild(d);
    chat.scrollTop = chat.scrollHeight;
  }

  function updateStatus(m) {
    const parts = [];
    if (typeof m.rssi === 'number' && m.rssi !== 0) parts.push('RSSI ' + m.rssi);
    if (m.linked) parts.push('LoRa bağlı');
    else parts.push('LoRa bekleniyor');
    if (typeof m.uptime === 'number') {
      const mins = Math.floor(m.uptime / 60);
      const secs = m.uptime % 60;
      parts.push(String(mins).padStart(2, '0') + ':' + String(secs).padStart(2, '0'));
    }
    meta.textContent = parts.join(' · ');
    status.textContent = m.linked ? 'Bağlı' : 'Bağlı (LoRa yok)';
    status.classList.toggle('ok', true);
  }

  form.onsubmit = (e) => {
    e.preventDefault();
    const t = input.value.trim();
    if (!t || !ws || ws.readyState !== 1) return;
    ws.send(JSON.stringify({ type: 'message', text: t }));
    input.value = '';
  };

  connect();
})();
