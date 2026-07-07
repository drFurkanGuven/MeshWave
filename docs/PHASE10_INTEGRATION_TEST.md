# LoRaLink — Phase 10: Integration Testing (E220 ↔ SX1278)

**Status:** In progress / Ready to run

Bu fazın amacı **E220 (Cardputer) ↔ SX1278 (Gateway)** arasında gerçek sahada:
- **Paket alışverişinin** çalıştığını,
- **ACK / retry / dedup** mekanizmasının stabil olduğunu,
- **RSSI / SNR / link_up** metriklerinin anlamlı olduğunu
kanıtlamak ve sorun çıkarsa hızlı teşhis adımlarını netleştirmektir.

---

## 1) Hazırlık ve flash

### 1.1 Firmware

```bash
pio run -e cardputer -t upload
pio run -e gateway -t upload
```

### 1.2 Gateway web UI (LittleFS)

```bash
pio run -e gateway -t buildfs && pio run -e gateway -t uploadfs
```

---

## 2) Kablolama / fiziksel kontrol

### 2.1 Cardputer + E220
- E220 UART: **TX=GPIO1**, **RX=GPIO2** (board tanımı üzerinden)
- E220 DIP: **M0=OFF, M1=OFF** (normal TX/RX)
- Anten: 433 MHz anten takılı olmalı

### 2.2 Gateway (T3 V1.6.1 + SX1278)
- SPI + DIO0 + RST pinleri `boards/t3_v161/board.h` üzerinden
- Anten: 433 MHz anten takılı olmalı

---

## 3) Mutlu yol (happy path) testleri

### 3.1 Seri üzerinden temel chat
1. Gateway seri monitör: bir satır yaz → Enter
2. Cardputer ekranda görmeli
3. Cardputer’dan Enter ile cevapla → Gateway seri monitörde görmeli

### 3.2 Web üzerinden chat
1. Telefonda gateway AP’ye bağlan (`LoRaLink-GW`)
2. Tarayıcıdan `http://192.168.4.1` aç
3. Mesaj gönder → Cardputer’da görünmeli
4. Cardputer cevap → telefonda görünmeli

### 3.3 Link probe (PING/PONG)
- Gateway arka planda PING gönderir.
- Loglarda PONG ve RSSI görülmeli.

---

## 4) Metrikler ve beklenen değerler

### 4.1 `/api/status`
`/api/status` JSON’unda:
- **linked**: `true` olmalı (peer görüldüyse)
- **rssi**: tipik olarak \(-40\) ile \(-120\) arası
- **snr**: SX1278 tarafında anlamlı (E220 tarafında genelde 0)
- **lastPeerMs**: sürekli güncellenmeli
- **tx/rx/ack/retries/crc/duplicates**: trafik ile artmalı

---

## 5) Sorun giderme (Troubleshooting)

### 5.1 linked hiç true olmuyor
- Anten var mı? 433 MHz mi?
- Frekans/SF/BW/CR/sync word aynı mı?
- E220 DIP M0/M1 doğru modda mı?
- Çok yakın mesafe (0–2 m) ile dene, sonra uzaklaştır

### 5.2 Mesaj gidiyor ama ACK timeout / retries çok
- SF12 + BW125 uzun airtime → parazit/çakışma ihtimali
- TX power çok düşük olabilir (gateway default 14 dBm)
- Anten/konnektör temassızlığı

### 5.3 CRC errors artıyor
- Sync word / BW / CR uyumsuzluğu
- Gürültü/parazit (başka 433 cihazları)
- E220 ↔ SX1278 interop gerçek sahada zor olabilir (tasarım notu)

### 5.4 Web UI açılıyor ama chat çalışmıyor
- `uploadfs` yapıldı mı?
- Tarayıcı WebSocket engelliyor mu? (çok nadir)
- Gateway seri monitörde WS connect logları var mı?

---

## 6) Sonuç kriterleri

Bu faz **başarılı** sayılır:
- Seri + web üzerinden karşılıklı chat çalışıyor
- `linked=true` stabil
- `retries` düşük, `crc_errors` düşük (ortama göre değişebilir)
- 10–15 dk boyunca kopmadan çalışıyor

