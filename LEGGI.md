📊 Dashboard per Fiat Panda 141

Creazione di una dashboard su dispositivo Raspberry Pi Pico 2 RP2350 base Arduino IDE con creazione grafica dedicata

---

🎯 Problema & Obiettivi
- **Contesto:** Dispositivo di visione e controllo parametri auto
- **Obiettivo:** Monitoraggio accessibile, preciso e veloce dei dati riguardanti temperatura acqua, voltaggio impianto e batteria e temperatura e pressione olio oltre a datario e orario 

---

🛠️ Strumenti & Tecnologie Usate
**Linguaggi:** C++ / C
**Ambiente di Sviluppo:** Arduino IDE & Visual Studio Code
**Controllo Versione:** Plug debug e upload

---

**ATTENZIONE: PROGETTO PER UTILIZZO HOBBISTICO E NON A FINI COMMERCIALI DA TERMINARE E PERFEZIONARE E TESTARE SULL'AUTO STESSA!**

---

🖼️ Anteprima & Interfaccia
<img width="1024" height="559" alt="ui" src="https://github.com/user-attachments/assets/c6d53ab0-2612-4d30-877a-ae313d35f956" />

---

## 📁 Struttura della Repository
```text
Test_panda/
│
├── src/                        # Tutti i file sorgente del codice
│   ├── Test_panda.ino          # File principale INO
│   ├── panda_ui.h
│   ├── panda_types.h
│   ├── lv_port_disp.c
│   └── lv_port_disp.h
│
├── drivers/                    # Driver hardware e schede (BSP / PIO / QSPI)
│   ├── bsp_co5300.c
│   ├── bsp_co5300.h
│   ├── bsp_display.h
│   ├── bsp_dma_channel_irq.c
│   ├── bsp_dma_channel_irq.h
│   ├── pio_qspi.c
│   ├── pio_qspi.h
│   └── pio_qspi.pio.h
│
├── assets/                     # Grafica e risorse visuali (loghi e icone)
│   ├── img_fiat_logo.c
│   ├── img_fiat_logo.h
│   ├── img_icon_batteria.c
│   ├── img_icon_batteria.h
│   ├── img_icon_olio.c
│   ├── img_icon_olio.h
│   ├── img_icon_termometro.c
│   └── img_icon_termometro.h
│
├── docs/                       # Eventuali note o schemi
│   └── note/                   # La tua cartella di note
│
├── LEGGI.md                   # Il file principale che spiega il progetto
└── .gitignore                  # File di build/temporanei da ignorare
