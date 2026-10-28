📊 Fiat Panda 141 Digital Dashboard

An embedded telemetry and display system built on the Raspberry Pi Pico 2 RP2350 using the Arduino IDE framework, featuring a custom-designed graphical interface.

---

🎯 Overview & Objectives
- **Context:** Vehicle data telemetry and real-time parameters monitoring unit.
- **Objective:** Provide accessible, precise, and fast monitoring of critical engine parameters, including coolant temperature, electrical system voltage, battery voltage, oil temperature, oil pressure, plus date and time.

---

🛠️ Tech Stack & Tools Used
- **Languages:** C / C++
- **Microcontroller:** Raspberry Pi Pico 2 (RP2040)
- **Development Environment:** VS Code & Arduino IDE
- **Graphics & Drivers:** Custom QSPI Display Driver & LVGL Interface

---

**DISCLAIMER: This is a personal hobby project created for testing and learning purposes. It is currently under active development and testing on a physical vehicle.**

---

🖼️ Preview & Interface
<img width="1024" height="559" alt="ui" src="https://github.com/user-attachments/assets/f910e625-026a-43e8-9834-e065583eff0c" />

---

## 📁 Repository Structure
```text
Test_panda/
│
├── src/                        # Source code files (Main logic & UI)
│   ├── Test_panda.ino          # Main Arduino entry point
│   ├── panda_ui.h
│   ├── panda_types.h
│   ├── lv_port_disp.c
│   └── lv_port_disp.h
│
├── drivers/                    # Hardware drivers (BSP, PIO, QSPI)
│   ├── bsp_co5300.c
│   ├── bsp_co5300.h
│   ├── bsp_display.h
│   ├── bsp_dma_channel_irq.c
│   ├── bsp_dma_channel_irq.h
│   ├── pio_qspi.c
│   ├── pio_qspi.h
│   └── pio_qspi.pio.h
│
├── assets/                     # Graphical assets (Logos and status icons)
│   ├── img_fiat_logo.c
│   ├── img_fiat_logo.h
│   ├── img_icon_batteria.c
│   ├── img_icon_batteria.h
│   ├── img_icon_olio.c
│   ├── img_icon_olio.h
│   ├── img_icon_termometro.c
│   └── img_icon_termometro.h
│
├── docs/                       # Project documentation and schematics
│   └── note/                   # Personal notes and pinouts
│
├── README.md                   # Main documentation
└── .gitignore                  # Build files to exclude
