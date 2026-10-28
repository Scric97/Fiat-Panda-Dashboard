#include <stdio.h>
#include "pico/stdlib.h"
#include <lvgl.h>
#include "lv_port_disp.h"
#include <Wire.h>
#include <Arduino.h>
#include "TouchDrvFT6X36.hpp"
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>
#include "bsp_display.h"
#include "panda_types.h"
#include "panda_ui.h"

// Interfaccia display definita in lv_port_disp.c (non statica, quindi
// accessibile da qui): serve per controllare la luminosità a runtime.
extern bsp_display_interface_t *display_if;

// Definizione reale delle variabili globali dichiarate "extern" in
// panda_ui.h (lì c'è solo la dichiarazione, qui l'allocazione vera:
// senza questo blocco il linker non trova a chi si riferiscono).
lv_obj_t *scr_splash;
lv_obj_t *scr_clock;
lv_obj_t *scr_oil;
lv_obj_t *scr_water_batt;
lv_obj_t *scr_set_luminosita;
lv_obj_t *scr_set_orario;
lv_obj_t *scr_set_data;
lv_obj_t *label_time;
lv_obj_t *label_date;
lv_obj_t *lbl_oil_pressure_val;
lv_obj_t *lbl_oil_temp_val;
lv_obj_t *lbl_water_temp_val;
lv_obj_t *lbl_battery_val;
lv_obj_t *slider_luminosita;
lv_obj_t *lbl_luminosita_pct;
lv_obj_t *lbl_set_ora;
lv_obj_t *lbl_set_min;
lv_obj_t *lbl_set_giorno;
lv_obj_t *lbl_set_mese;
lv_obj_t *lbl_set_weekday;

// ===================================================================
// TOUCH — invariato dal demo ufficiale Waveshare (bus Wire1 interno)
// ===================================================================
#define TOUCH_SDA 6
#define TOUCH_SCL 7
#define TOUCH_IRQ 20
#define TOUCH_RST 17

#define DISP_HOR_RES 466
#define DISP_VER_RES 466

TouchDrvFT6X36 touch;

void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  int16_t x[1], y[1];
  if (digitalRead(TOUCH_IRQ) == LOW) {
    uint8_t touched = touch.getPoint(x, y, 1);
    if (touched) {
      data->state = LV_INDEV_STATE_PR;
      data->point.x = x[0];
      data->point.y = y[0];
    } else {
      data->state = LV_INDEV_STATE_REL;
    }
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void touch_init(void) {
  pinMode(TOUCH_IRQ, INPUT);
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(10);
  digitalWrite(TOUCH_RST, HIGH);
  delay(100);
  if (!touch.begin(Wire1, FT6X36_SLAVE_ADDRESS)) {
    Serial.println("Touch non trovato - controlla i collegamenti!");
    while (1) { delay(1000); }
  }
  touch.interruptTrigger();
}

// ===================================================================
// SENSORI — ADS1115 sul bus I2C0 esterno ("Wire", header esposto sulla
// scheda, separato dal bus interno "Wire1" usato da touch/RTC/IMU).
// ===================================================================
Adafruit_ADS1115 ads;

#define CH_BATTERIA   0
#define CH_PRESSIONE  1
#define CH_TEMP_ACQUA 2
#define CH_TEMP_OLIO  3

const float RAPPORTO_PARTITORE_BATT = 33000.0 / (100000.0 + 33000.0);
const float R_SERIE_NTC = 4700.0;
const float V_RIF_NTC   = 3.3;
const float V_MIN_PRESS = 0.5;
const float V_MAX_PRESS = 4.5;
const float PSI_MAX     = 200.0;

// Coefficienti Steinhart-Hart NTC — PLACEHOLDER, da calibrare
// (vedi schema_collegamenti.md sezione 6) e sostituire qui.
float SH_A = 0.001129148;
float SH_B = 0.000234125;
float SH_C = 0.0000000876741;

bool ads_ok = false;

float leggiTensioneBatteria() {
  if (!ads_ok) return 0;
  int16_t raw = ads.readADC_SingleEnded(CH_BATTERIA);
  return ads.computeVolts(raw) / RAPPORTO_PARTITORE_BATT;
}

float leggiPressioneOlio() {
  if (!ads_ok) return 0;
  int16_t raw = ads.readADC_SingleEnded(CH_PRESSIONE);
  float vLetta = ads.computeVolts(raw);
  if (vLetta < V_MIN_PRESS) vLetta = V_MIN_PRESS;
  float psi = (vLetta - V_MIN_PRESS) * (PSI_MAX / (V_MAX_PRESS - V_MIN_PRESS));
  return psi * 0.0689476;
}

float leggiResistenzaNTC(uint8_t canale) {
  int16_t raw = ads.readADC_SingleEnded(canale);
  float vLetta = ads.computeVolts(raw);
  if (vLetta <= 0.001) vLetta = 0.001;
  return R_SERIE_NTC * (vLetta / (V_RIF_NTC - vLetta));
}

// Sensore NTC scollegato o in corto => il calcolo dà NaN o valori assurdi
bool temperaturaValida(float t) {
  return !isnan(t) && t > -40.0f && t < 200.0f;
}

float leggiTemperaturaNTC(uint8_t canale) {
  if (!ads_ok) return 0;
  float r = leggiResistenzaNTC(canale);
  float logR = log(r);
  float invT = SH_A + SH_B * logR + SH_C * logR * logR * logR;
  return (1.0 / invT) - 273.15;
}

// ===================================================================
// RTC PCF85063 — sul bus Wire1 (stesso bus interno di touch/IMU),
// NON sul bus Wire esterno. Registri diretti via I2C.
// ===================================================================
#define PCF85063_ADDR 0x51
#define REG_SECONDS   0x04

uint8_t bcdToDec(uint8_t bcd) { return ((bcd / 16) * 10) + (bcd % 16); }
uint8_t decToBcd(uint8_t dec) { return ((dec / 10) * 16) + (dec % 10); }

DataOra leggiOra() {
  DataOra d = {0, 0, 0, 1, 1, 2026};
  Wire1.beginTransmission(PCF85063_ADDR);
  Wire1.write(REG_SECONDS);
  if (Wire1.endTransmission(false) != 0) return d; // RTC non risponde
  Wire1.requestFrom(PCF85063_ADDR, (uint8_t)7);
  if (Wire1.available() < 7) return d;

  d.secondi = bcdToDec(Wire1.read() & 0x7F);
  d.minuti  = bcdToDec(Wire1.read() & 0x7F);
  d.ore     = bcdToDec(Wire1.read() & 0x3F);
  d.giorno  = bcdToDec(Wire1.read() & 0x3F);
  Wire1.read(); // weekday, non usato
  d.mese    = bcdToDec(Wire1.read() & 0x1F);
  d.anno    = 2000 + bcdToDec(Wire1.read());
  return d;
}

void impostaOraRTC(uint8_t sec, uint8_t min, uint8_t ore, uint8_t giorno, uint8_t mese, uint16_t anno) {
  Wire1.beginTransmission(PCF85063_ADDR);
  Wire1.write(REG_SECONDS);
  Wire1.write(decToBcd(sec));
  Wire1.write(decToBcd(min));
  Wire1.write(decToBcd(ore));
  Wire1.write(decToBcd(giorno));
  Wire1.write(0); // weekday, non critico (lo ricalcoliamo sempre da data)
  Wire1.write(decToBcd(mese));
  Wire1.write(decToBcd(anno - 2000));
  Wire1.endTransmission();
}

// Calcola il giorno della settimana (0=domenica..6=sabato) dalla data,
// algoritmo di Sakamoto — non dipende dal registro weekday della RTC
// (che avrebbe bisogno di essere impostato coerentemente a parte).
int giornoSettimana(int anno, int mese, int giorno) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (mese < 3) anno -= 1;
  return (anno + anno / 4 - anno / 100 + anno / 400 + t[mese - 1] + giorno) % 7;
}

const char *NOMI_GIORNI[] = {"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"};
const char *NOMI_MESI[] = {"", "GEN", "FEB", "MAR", "APR", "MAG", "GIU",
                            "LUG", "AGO", "SET", "OTT", "NOV", "DIC"};

// ===================================================================
// IMPOSTAZIONI — luminosità manuale e regolazione ora/data
// ===================================================================
// Luminosità: salvata nella flash (EEPROM emulata) così sopravvive allo
// spegnimento. Se non c'è ancora nessun valore salvato parte al MINIMO.
#define LUM_MINIMA        10
#define EEPROM_MAGIC      0xA5
uint8_t luminosita_manuale = LUM_MINIMA;
bool lum_da_salvare = false;
uint32_t lum_ultimo_cambio = 0;

void luminosita_carica() {
  EEPROM.begin(16);
  if (EEPROM.read(0) == EEPROM_MAGIC) {
    uint8_t v = EEPROM.read(1);
    if (v >= LUM_MINIMA && v <= 100) luminosita_manuale = v;
  }
}

void luminosita_salva_se_serve() {
  // Scrive solo dopo 2 secondi senza altri cambiamenti, per non
  // consumare cicli di scrittura della flash mentre si trascina il cursore
  if (lum_da_salvare && (millis() - lum_ultimo_cambio > 2000)) {
    EEPROM.write(0, EEPROM_MAGIC);
    EEPROM.write(1, luminosita_manuale);
    EEPROM.commit();
    lum_da_salvare = false;
  }
}
uint8_t pend_ora = 12, pend_min = 0, pend_giorno = 1, pend_mese = 1;
int pend_anno = 2026;   // anno memorizzato nell'RTC (non mostrato): serve a far tornare il giorno della settimana

void cb_slider_luminosita(lv_event_t *e) {
  int val = lv_slider_get_value(slider_luminosita);
  luminosita_manuale = val;
  lum_da_salvare = true;
  lum_ultimo_cambio = millis();
  lv_label_set_text_fmt(lbl_luminosita_pct, "%d%%", val);
  if (display_if && display_if->set_brightness) {
    display_if->set_brightness(val);
  }
}

void cb_ora_su(lv_event_t *e)  { pend_ora = (pend_ora + 1) % 24;  lv_label_set_text_fmt(lbl_set_ora, "%02d", pend_ora); impostaOraRTC(0, pend_min, pend_ora, pend_giorno, pend_mese, pend_anno); }
void cb_ora_giu(lv_event_t *e) { pend_ora = (pend_ora + 23) % 24; lv_label_set_text_fmt(lbl_set_ora, "%02d", pend_ora); impostaOraRTC(0, pend_min, pend_ora, pend_giorno, pend_mese, pend_anno); }
void cb_min_su(lv_event_t *e)  { pend_min = (pend_min + 1) % 60;  lv_label_set_text_fmt(lbl_set_min, "%02d", pend_min); impostaOraRTC(0, pend_min, pend_ora, pend_giorno, pend_mese, pend_anno); }
void cb_min_giu(lv_event_t *e) { pend_min = (pend_min + 59) % 60; lv_label_set_text_fmt(lbl_set_min, "%02d", pend_min); impostaOraRTC(0, pend_min, pend_ora, pend_giorno, pend_mese, pend_anno); }

bool annoBisestile(int a) {
  return (a % 4 == 0 && a % 100 != 0) || (a % 400 == 0);
}

int giorniNelMese(int mese, int anno) {
  static const uint8_t g[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (mese == 2 && annoBisestile(anno)) return 29;
  return g[mese - 1];
}

void aggiorna_label_weekday() {
  int wd = giornoSettimana(pend_anno, pend_mese, pend_giorno);
  lv_label_set_text(lbl_set_weekday, NOMI_GIORNI[wd]);
}

void salva_data_rtc() {
  impostaOraRTC(0, pend_min, pend_ora, pend_giorno, pend_mese, pend_anno);
}

void cb_giorno_su(lv_event_t *e) {
  pend_giorno = (pend_giorno >= giorniNelMese(pend_mese, pend_anno)) ? 1 : pend_giorno + 1;
  lv_label_set_text_fmt(lbl_set_giorno, "%02d", pend_giorno);
  aggiorna_label_weekday();
  salva_data_rtc();
}
void cb_giorno_giu(lv_event_t *e) {
  pend_giorno = (pend_giorno <= 1) ? giorniNelMese(pend_mese, pend_anno) : pend_giorno - 1;
  lv_label_set_text_fmt(lbl_set_giorno, "%02d", pend_giorno);
  aggiorna_label_weekday();
  salva_data_rtc();
}

// Dopo un cambio di mese il giorno non deve superare i giorni di quel mese
// (niente piu' "31 NOV" o "30 FEB")
void correggi_giorno_per_mese() {
  int massimo = giorniNelMese(pend_mese, pend_anno);
  if (pend_giorno > massimo) {
    pend_giorno = massimo;
    lv_label_set_text_fmt(lbl_set_giorno, "%02d", pend_giorno);
  }
}

void cb_mese_su(lv_event_t *e) {
  pend_mese = (pend_mese >= 12) ? 1 : pend_mese + 1;
  lv_label_set_text(lbl_set_mese, NOMI_MESI[pend_mese]);
  correggi_giorno_per_mese();
  aggiorna_label_weekday();
  salva_data_rtc();
}
void cb_mese_giu(lv_event_t *e) {
  pend_mese = (pend_mese <= 1) ? 12 : pend_mese - 1;
  lv_label_set_text(lbl_set_mese, NOMI_MESI[pend_mese]);
  correggi_giorno_per_mese();
  aggiorna_label_weekday();
  salva_data_rtc();
}

// Giorno della settimana modificabile: giorno e mese restano quelli
// impostati e cambia solo l'anno (non mostrato), scegliendo il PRIMO anno
// da 2026 in poi in cui quella data cade nel giorno della settimana voluto.
// Cosi' l'anno resta sempre in un intervallo ristretto e il valore mostrato
// e' identico anche dopo aver chiuso e riaperto le impostazioni.
void cambia_giorno_settimana(int verso) {
  int target = (giornoSettimana(pend_anno, pend_mese, pend_giorno) + verso + 7) % 7;
  for (int a = 2026; a <= 2099; a++) {
    if (pend_giorno > giorniNelMese(pend_mese, a)) continue; // es. 29 FEB in anno non bisestile
    if (giornoSettimana(a, pend_mese, pend_giorno) == target) {
      pend_anno = a;
      break;
    }
  }
  aggiorna_label_weekday();
  salva_data_rtc();
}
void cb_weekday_su(lv_event_t *e)  { cambia_giorno_settimana(+1); }
void cb_weekday_giu(lv_event_t *e) { cambia_giorno_settimana(-1); }

// Richiamata ogni volta che si apre una delle 3 pagine impostazioni:
// risincronizza tutti i campi con l'ora corrente letta dall'RTC e con
// la luminosità attuale, indipendentemente da quale sottopagina è.
void cb_settings_page_loaded(lv_event_t *e) {
  DataOra ora = leggiOra();
  pend_ora = ora.ore;
  pend_min = ora.minuti;
  pend_giorno = ora.giorno;
  pend_mese = ora.mese;
  pend_anno = (ora.anno >= 2026 && ora.anno <= 2099) ? ora.anno : 2026;
  lv_label_set_text_fmt(lbl_set_ora, "%02d", pend_ora);
  lv_label_set_text_fmt(lbl_set_min, "%02d", pend_min);
  lv_label_set_text_fmt(lbl_set_giorno, "%02d", pend_giorno);
  lv_label_set_text(lbl_set_mese, NOMI_MESI[pend_mese]);
  aggiorna_label_weekday();
  lv_slider_set_value(slider_luminosita, luminosita_manuale, LV_ANIM_OFF);
  lv_label_set_text_fmt(lbl_luminosita_pct, "%d%%", luminosita_manuale);
}

// ===================================================================
// SETUP / LOOP — sequenza di init identica al demo ufficiale,
// con l'aggiunta di ADS1115 e della UI/logica Panda
// ===================================================================
void setup() {
  Serial.begin(115200);

  Serial.println("### BOOT: setup() avviato ###");

  // Touch + display: IDENTICO al demo ufficiale funzionante, non toccato
  Serial.println("Inizializzo Wire1 + touch...");
  Wire1.setSCL(TOUCH_SCL);
  Wire1.setSDA(TOUCH_SDA);
  Wire1.begin();
  touch_init();
  Serial.println("Touch OK.");

  Serial.println("lv_init()...");
  lv_init();
  Serial.println("lv_init() OK.");

  Serial.println("lv_port_disp_init()...");
  luminosita_carica(); // legge la luminosità salvata (default: minimo)
  lv_port_disp_init(DISP_HOR_RES, DISP_VER_RES, 0, false);
  if (display_if && display_if->set_brightness) {
    display_if->set_brightness(luminosita_manuale);
  }
  Serial.println("lv_port_disp_init() OK.");

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  Serial.println("Indev (touch LVGL) registrato.");

  // Sensori esterni: bus I2C0 ("Wire"), separato dal bus interno Wire1
  Serial.println("Inizializzo Wire (I2C esterno) + ADS1115...");
  Wire.begin();
  ads_ok = ads.begin();
  if (!ads_ok) {
    Serial.println("ADS1115 non trovato sul bus I2C esterno (Wire). Proseguo comunque.");
  } else {
    Serial.println("ADS1115 OK.");
  }
  ads.setGain(GAIN_TWOTHIRDS);

  Serial.println("Chiamo panda_ui_create()...");
  panda_ui_create();
  Serial.println("panda_ui_create() OK. Setup terminato, entro nel loop().");
}

void loop() {
  static uint32_t last_tick = 0;
  uint32_t current_tick = millis();
  lv_tick_inc(current_tick - last_tick);
  last_tick = current_tick;

  static uint32_t last_heartbeat = 0;
  if (millis() - last_heartbeat >= 1000) {
    last_heartbeat = millis();
    Serial.print("Loop attivo, uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" s");
  }

  luminosita_salva_se_serve();

  uint16_t lvgl_ms = lv_timer_handler();
  if (lvgl_ms > 500) lvgl_ms = 500;
  if (lvgl_ms < 1) lvgl_ms = 1;

  uint32_t now = millis();

  // Dopo 5s passa dallo splash alla schermata orologio
  static bool splash_done = false;
  if (!splash_done && now > 10000) {
    Serial.println(">>> Passo dallo splash all'orologio ora.");
    lv_scr_load_anim(scr_clock, LV_SCR_LOAD_ANIM_FADE_ON, 400, 0, false);
    splash_done = true;
  }

  // Aggiornamento sensori/orologio ogni 500 ms — SOLO dopo lo splash,
  // così un eventuale blocco nella lettura I2C (RTC non ancora
  // collegato, bus che non risponde) non impedisce mai la transizione
  // iniziale dallo splash all'orologio.
  if (splash_done) {
    static uint32_t last_sensor_update = 0;
    if (now - last_sensor_update >= 500) {
      last_sensor_update = now;

      float vBatt  = leggiTensioneBatteria();
      float press  = leggiPressioneOlio();
      float tAcqua = leggiTemperaturaNTC(CH_TEMP_ACQUA);
      float tOlio  = leggiTemperaturaNTC(CH_TEMP_OLIO);
      DataOra ora  = leggiOra();

      // NB: LVGL qui non sa stampare i numeri con la virgola (%f), che
      // comparivano come "f". Li formatto quindi come numeri interi.
      if (ads_ok) {
        int v10  = (int)(vBatt * 10.0f + 0.5f);   if (v10 < 0) v10 = 0;
        int p100 = (int)(press * 100.0f + 0.5f);  if (p100 < 0) p100 = 0;
        lv_label_set_text_fmt(lbl_battery_val, "%d.%d V", v10 / 10, v10 % 10);
        lv_label_set_text_fmt(lbl_oil_pressure_val, "%d.%02d BAR", p100 / 100, p100 % 100);
        if (temperaturaValida(tAcqua))
          lv_label_set_text_fmt(lbl_water_temp_val, "%d \xC2\xB0" "C", (int)(tAcqua + 0.5f));
        else
          lv_label_set_text(lbl_water_temp_val, "-- \xC2\xB0" "C");
        if (temperaturaValida(tOlio))
          lv_label_set_text_fmt(lbl_oil_temp_val, "%d \xC2\xB0" "C", (int)(tOlio + 0.5f));
        else
          lv_label_set_text(lbl_oil_temp_val, "-- \xC2\xB0" "C");
      } else {
        // modulo ADS1115 non collegato: niente numeri inventati
        lv_label_set_text(lbl_battery_val, "-- V");
        lv_label_set_text(lbl_oil_pressure_val, "-- BAR");
        lv_label_set_text(lbl_water_temp_val, "-- \xC2\xB0" "C");
        lv_label_set_text(lbl_oil_temp_val, "-- \xC2\xB0" "C");
      }
      lv_label_set_text_fmt(label_time, "%02d:%02d", ora.ore, ora.minuti);

      int wd = giornoSettimana(ora.anno, ora.mese, ora.giorno);
      lv_label_set_text_fmt(label_date, "%s %d %s", NOMI_GIORNI[wd], ora.giorno, NOMI_MESI[ora.mese]);
    }
  }

  delay(lvgl_ms);
}
