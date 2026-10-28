#ifndef PANDA_UI_H
#define PANDA_UI_H

#include <lvgl.h>
#include "img_fiat_logo.h"

// ===================================================================
// Schermate e widget aggiornabili — dichiarazioni (extern).
// Le variabili vere sono definite nel file .ino principale.
// ===================================================================
extern lv_obj_t *scr_splash;
extern lv_obj_t *scr_clock;
extern lv_obj_t *scr_oil;
extern lv_obj_t *scr_water_batt;
extern lv_obj_t *scr_set_luminosita;
extern lv_obj_t *scr_set_orario;
extern lv_obj_t *scr_set_data;

extern lv_obj_t *label_time;
extern lv_obj_t *label_date;
extern lv_obj_t *lbl_oil_pressure_val;
extern lv_obj_t *lbl_oil_temp_val;
extern lv_obj_t *lbl_water_temp_val;
extern lv_obj_t *lbl_battery_val;

extern lv_obj_t *slider_luminosita;
extern lv_obj_t *lbl_luminosita_pct;
extern lv_obj_t *lbl_set_ora;
extern lv_obj_t *lbl_set_min;
extern lv_obj_t *lbl_set_giorno;
extern lv_obj_t *lbl_set_mese;
extern lv_obj_t *lbl_set_weekday;

// Callback implementate in Test_panda.ino (serve la logica RTC/display)
extern void cb_slider_luminosita(lv_event_t *e);
extern void cb_ora_su(lv_event_t *e);
extern void cb_ora_giu(lv_event_t *e);
extern void cb_min_su(lv_event_t *e);
extern void cb_min_giu(lv_event_t *e);
extern void cb_giorno_su(lv_event_t *e);
extern void cb_giorno_giu(lv_event_t *e);
extern void cb_mese_su(lv_event_t *e);
extern void cb_mese_giu(lv_event_t *e);
extern void cb_weekday_su(lv_event_t *e);
extern void cb_weekday_giu(lv_event_t *e);
extern void cb_settings_page_loaded(lv_event_t *e);

// Colore FIAT blu (campionato dal logo originale) e sfondo bianco
#define COL_BG      lv_color_black()
#define COL_TXT     lv_color_white()
#define COL_GRAY    lv_color_hex(0x888888)

// Spostamento orizzontale del logo di avvio, in pixel (negativo = verso sinistra).
// Se il logo ti sembra ancora decentrato, cambia solo questo numero.
#define LOGO_OFFSET_X (-8)

// -------------------------------------------------------------------
// Swipe ciclico tra le 3 pagine principali (0=clock,1=oil,2=water+batt)
// -------------------------------------------------------------------
static int panda_screen_index = 0;
#define PANDA_NUM_SCREENS 3

static void panda_go_to_screen(int index) {
    if (index < 0) index = PANDA_NUM_SCREENS - 1;
    if (index >= PANDA_NUM_SCREENS) index = 0;
    panda_screen_index = index;
}

static void screen_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_GESTURE) {
        lv_obj_t *screens[PANDA_NUM_SCREENS] = { scr_clock, scr_oil, scr_water_batt };
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) {
            panda_go_to_screen(panda_screen_index + 1);
            lv_scr_load_anim(screens[panda_screen_index], LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
        } else if (dir == LV_DIR_RIGHT) {
            panda_go_to_screen(panda_screen_index - 1);
            lv_scr_load_anim(screens[panda_screen_index], LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
        }
    }
}

// -------------------------------------------------------------------
// Swipe ciclico tra le 3 pagine di impostazioni
// (0=luminosità, 1=orario, 2=data)
// -------------------------------------------------------------------
static int settings_index = 0;
#define SETTINGS_NUM_SCREENS 3

static void settings_go_to(int index) {
    if (index < 0) index = SETTINGS_NUM_SCREENS - 1;
    if (index >= SETTINGS_NUM_SCREENS) index = 0;
    settings_index = index;
}

static void settings_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_GESTURE) {
        lv_obj_t *screens[SETTINGS_NUM_SCREENS] = { scr_set_luminosita, scr_set_orario, scr_set_data };
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) {
            settings_go_to(settings_index + 1);
            lv_scr_load_anim(screens[settings_index], LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
        } else if (dir == LV_DIR_RIGHT) {
            settings_go_to(settings_index - 1);
            lv_scr_load_anim(screens[settings_index], LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
        }
    }
}

// Apre il gruppo impostazioni partendo dalla pagina Luminosità
// (usato dall'ingranaggio sotto l'ora)
static void apri_impostazioni_cb(lv_event_t *e) {
    settings_index = 0;
    lv_scr_load_anim(scr_set_luminosita, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}

// "X" — chiude le impostazioni e torna sempre all'orologio
static void chiudi_impostazioni_cb(lv_event_t *e) {
    lv_scr_load_anim(scr_clock, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}

// ===================================================================
// Helper di layout
// ===================================================================

// Piccolo rettangolo pieno bianco (mattone base per disegnare le icone)
static lv_obj_t *panda_rect_pieno(lv_obj_t *parent, int w, int h, int radius, int x, int y) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_radius(o, radius, 0);
    lv_obj_set_style_bg_color(o, COL_TXT, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(o, LV_ALIGN_CENTER, x, y);
    return o;
}

// Rettangolo con solo il contorno bianco (interno trasparente)
static lv_obj_t *panda_rect_contorno(lv_obj_t *parent, int w, int h, int radius, int spessore, int x, int y) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_radius(o, radius, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(o, COL_TXT, 0);
    lv_obj_set_style_border_width(o, spessore, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(o, LV_ALIGN_CENTER, x, y);
    return o;
}

// Termometro: stelo col solo contorno, bulbo pieno e colonnina di mercurio dentro
static void panda_disegna_termometro(lv_obj_t *parent, int x, int y) {
    panda_rect_contorno(parent, 14, 40, 7, 3, x, y - 8);   // stelo
    panda_rect_pieno(parent, 26, 26, LV_RADIUS_CIRCLE, x, y + 14); // bulbo
    panda_rect_pieno(parent, 4, 22, 2, x, y - 2);          // mercurio
}

// Goccia (per l'olio): punta fatta con strisce sempre piu' larghe + cerchio sotto
static void panda_disegna_goccia(lv_obj_t *parent, int x, int y) {
    int cy = y + 8;            // centro del cerchio
    int punta = cy - 30;       // altezza della punta
    for (int k = 0; k < 12; k++) {
        int larghezza = (int)(0.872f * (2 * k + 1) + 0.5f);
        if (larghezza < 2) larghezza = 2;
        panda_rect_pieno(parent, larghezza, 2, 1, x, punta + 1 + 2 * k);
    }
    panda_rect_pieno(parent, 24, 24, LV_RADIUS_CIRCLE, x, cy);
}

// Batteria: corpo col contorno, terminale a destra e tre tacche di carica
static void panda_disegna_batteria(lv_obj_t *parent, int x, int y) {
    panda_rect_contorno(parent, 40, 26, 5, 3, x, y);       // corpo
    panda_rect_pieno(parent, 5, 11, 2, x + 23, y);         // terminale +
    for (int i = -1; i <= 1; i++) {
        panda_rect_pieno(parent, 8, 14, 2, x + i * 11, y); // tacche
    }
}

// Riga "titolo sopra, icona + valore grande sotto", per Olio/Acqua/Batteria.
// tipo_icona: 0=termometro, 1=goccia(olio), 2=batteria
static lv_obj_t *panda_crea_riga(lv_obj_t *parent, const char *titolo,
                                  int tipo_icona, const char *valore_iniziale,
                                  int offset_y) {
    lv_obj_t *lbl_titolo = lv_label_create(parent);
    lv_label_set_text(lbl_titolo, titolo);
    lv_obj_set_style_text_color(lbl_titolo, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_titolo, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_titolo, LV_ALIGN_CENTER, 0, offset_y - 38);

    int icon_x = -105;
    if (tipo_icona == 0) panda_disegna_termometro(parent, icon_x, offset_y + 18);
    else if (tipo_icona == 1) panda_disegna_goccia(parent, icon_x, offset_y + 18);
    else panda_disegna_batteria(parent, icon_x, offset_y + 18);

    lv_obj_t *lbl_valore = lv_label_create(parent);
    lv_label_set_text(lbl_valore, valore_iniziale);
    lv_obj_set_style_text_color(lbl_valore, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_valore, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_valore, LV_ALIGN_CENTER, 30, offset_y + 18);

    return lbl_valore;
}

// Linea divisoria orizzontale sottile, usata tra le due righe di Olio/Acqua-Batteria
static void panda_crea_divisore(lv_obj_t *parent, int offset_y) {
    static lv_point_t punti[2] = { {0, 1}, {220, 1} };
    lv_obj_t *linea = lv_line_create(parent);
    lv_line_set_points(linea, punti, 2);
    lv_obj_set_style_line_color(linea, lv_color_hex(0x555555), 0);
    lv_obj_set_style_line_width(linea, 2, 0);
    lv_obj_align(linea, LV_ALIGN_CENTER, 0, offset_y);
}

// Piccola "X" in alto al centro (NON nell'angolo: sullo schermo tondo
// gli angoli del framebuffer sono tagliati via dalla cornice e
// diventano invisibili/intoccabili), per chiudere le pagine impostazioni
static void panda_aggiungi_chiudi_x(lv_obj_t *parent) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 44, 44);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(lbl, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl);
    lv_obj_add_event_cb(btn, chiudi_impostazioni_cb, LV_EVENT_CLICKED, NULL);
}

// Pallini di paginazione in basso: quello della pagina corrente è vuoto,
// gli altri due pieni (come nei mockup).
static void panda_disegna_pallini(lv_obj_t *parent, int corrente) {
    for (int i = 0; i < 3; i++) {
        lv_obj_t *dot = lv_obj_create(parent);
        lv_obj_set_size(dot, 10, 10);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(dot, 2, 0);
        lv_obj_set_style_border_color(dot, COL_TXT, 0);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
        if (i == corrente) {
            lv_obj_set_style_bg_opa(dot, LV_OPA_TRANSP, 0);
        } else {
            lv_obj_set_style_bg_color(dot, COL_TXT, 0);
            lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        }
        lv_obj_align(dot, LV_ALIGN_BOTTOM_MID, (i - 1) * 22, -25);
    }
}

// Colonna verticale "freccia su / valore / freccia giù" per regolare
// ora, minuti, giorno o mese. Ritorna il label del valore.
static lv_obj_t *panda_crea_colonna(lv_obj_t *parent, int x_offset,
                                     lv_event_cb_t cb_su, lv_event_cb_t cb_giu) {
    lv_obj_t *btn_su = lv_btn_create(parent);
    lv_obj_set_size(btn_su, 40, 30);
    lv_obj_set_style_bg_opa(btn_su, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_su, 0, 0);
    lv_obj_set_style_border_width(btn_su, 0, 0);
    lv_obj_align(btn_su, LV_ALIGN_CENTER, x_offset, -42);
    lv_obj_t *lbl_su = lv_label_create(btn_su);
    lv_label_set_text(lbl_su, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(lbl_su, COL_TXT, 0);
    lv_obj_center(lbl_su);
    lv_obj_add_event_cb(btn_su, cb_su, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_val = lv_label_create(parent);
    lv_obj_set_style_text_color(lbl_val, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_val, &lv_font_montserrat_40, 0);
    lv_label_set_text(lbl_val, "--");
    lv_obj_align(lbl_val, LV_ALIGN_CENTER, x_offset, 0);

    lv_obj_t *btn_giu = lv_btn_create(parent);
    lv_obj_set_size(btn_giu, 40, 30);
    lv_obj_set_style_bg_opa(btn_giu, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_giu, 0, 0);
    lv_obj_set_style_border_width(btn_giu, 0, 0);
    lv_obj_align(btn_giu, LV_ALIGN_CENTER, x_offset, 42);
    lv_obj_t *lbl_giu = lv_label_create(btn_giu);
    lv_label_set_text(lbl_giu, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_color(lbl_giu, COL_TXT, 0);
    lv_obj_center(lbl_giu);
    lv_obj_add_event_cb(btn_giu, cb_giu, LV_EVENT_CLICKED, NULL);

    return lbl_val;
}

inline void panda_ui_create(void) {
    // -----------------------------------------------------------------
    // 1. SPLASH — logo FIAT vero su sfondo bianco (10 secondi)
    // -----------------------------------------------------------------
    scr_splash = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_splash, COL_BG, 0);

    // Logo FIAT vero (immagine, byte alto per primo per LV_COLOR_16_SWAP=1)
    lv_obj_t *logo = lv_img_create(scr_splash);
    lv_img_set_src(logo, &img_fiat_logo);
    lv_obj_align(logo, LV_ALIGN_CENTER, LOGO_OFFSET_X, 0);

    // -----------------------------------------------------------------
    // 2. OROLOGIO — pagina principale: ora, data, ingranaggio
    // -----------------------------------------------------------------
    scr_clock = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_clock, COL_BG, 0);
    lv_obj_add_event_cb(scr_clock, screen_event_cb, LV_EVENT_GESTURE, NULL);

    label_time = lv_label_create(scr_clock);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(label_time, COL_TXT, 0);
    lv_label_set_text(label_time, "--:--");
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, -20);

    label_date = lv_label_create(scr_clock);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_date, COL_TXT, 0);
    lv_label_set_text(label_date, "---");
    lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 40);

    // Ingranaggio sotto la data (unico punto d'ingresso alle impostazioni)
    lv_obj_t *btn_gear = lv_btn_create(scr_clock);
    lv_obj_set_size(btn_gear, 52, 52);
    lv_obj_set_style_radius(btn_gear, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_gear, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_gear, 0, 0);
    lv_obj_set_style_border_width(btn_gear, 0, 0);
    lv_obj_align(btn_gear, LV_ALIGN_CENTER, 0, 80);
    lv_obj_t *lbl_gear = lv_label_create(btn_gear);
    lv_label_set_text(lbl_gear, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_font(lbl_gear, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_gear, COL_TXT, 0);
    lv_obj_center(lbl_gear);
    lv_obj_add_event_cb(btn_gear, apri_impostazioni_cb, LV_EVENT_CLICKED, NULL);

    // -----------------------------------------------------------------
    // 3. PRESSIONE OLIO + TEMPERATURA OLIO (stessa pagina)
    // -----------------------------------------------------------------
    scr_oil = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_oil, COL_BG, 0);
    lv_obj_add_event_cb(scr_oil, screen_event_cb, LV_EVENT_GESTURE, NULL);

    lbl_oil_pressure_val = panda_crea_riga(scr_oil, "PRESSIONE OLIO", 1, "-- BAR", -82);
    panda_crea_divisore(scr_oil, 2);
    lbl_oil_temp_val     = panda_crea_riga(scr_oil, "TEMPERATURA OLIO", 0, "-- \xC2\xB0" "C", 88);

    // -----------------------------------------------------------------
    // 4. TEMPERATURA ACQUA + TENSIONE BATTERIA (stessa pagina)
    // -----------------------------------------------------------------
    scr_water_batt = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_water_batt, COL_BG, 0);
    lv_obj_add_event_cb(scr_water_batt, screen_event_cb, LV_EVENT_GESTURE, NULL);

    lbl_water_temp_val = panda_crea_riga(scr_water_batt, "TEMPERATURA ACQUA", 0, "-- \xC2\xB0" "C", -82);
    panda_crea_divisore(scr_water_batt, 2);
    lbl_battery_val     = panda_crea_riga(scr_water_batt, "TENSIONE BATTERIA", 2, "-- V", 88);

    // -----------------------------------------------------------------
    // 5. IMPOSTAZIONI: LUMINOSITÀ (pagina 1 di 3, scorrevole)
    // -----------------------------------------------------------------
    scr_set_luminosita = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_set_luminosita, COL_BG, 0);
    lv_obj_add_event_cb(scr_set_luminosita, settings_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr_set_luminosita, cb_settings_page_loaded, LV_EVENT_SCREEN_LOADED, NULL);
    panda_aggiungi_chiudi_x(scr_set_luminosita);

    lv_obj_t *lbl_l1 = lv_label_create(scr_set_luminosita);
    lv_label_set_text(lbl_l1, "IMPOSTAZIONI");
    lv_obj_set_style_text_color(lbl_l1, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_l1, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_l1, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t *lbl_l2 = lv_label_create(scr_set_luminosita);
    lv_label_set_text(lbl_l2, "LUMINOSITA'");
    lv_obj_set_style_text_color(lbl_l2, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_l2, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_l2, LV_ALIGN_CENTER, 0, -35);

    lv_obj_t *lbl_meno = lv_label_create(scr_set_luminosita);
    lv_label_set_text(lbl_meno, "-");
    lv_obj_set_style_text_color(lbl_meno, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_meno, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_meno, LV_ALIGN_CENTER, -110, 5);

    slider_luminosita = lv_slider_create(scr_set_luminosita);
    lv_obj_set_size(slider_luminosita, 170, 12);
    lv_obj_align(slider_luminosita, LV_ALIGN_CENTER, 0, 5);
    lv_obj_set_style_bg_color(slider_luminosita, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_luminosita, COL_TXT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_luminosita, COL_TXT, LV_PART_KNOB);
    lv_slider_set_range(slider_luminosita, 10, 100);
    lv_slider_set_value(slider_luminosita, 80, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_luminosita, cb_slider_luminosita, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *lbl_piu = lv_label_create(scr_set_luminosita);
    lv_label_set_text(lbl_piu, "+");
    lv_obj_set_style_text_color(lbl_piu, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_piu, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_piu, LV_ALIGN_CENTER, 110, 5);

    lbl_luminosita_pct = lv_label_create(scr_set_luminosita);
    lv_obj_set_style_text_color(lbl_luminosita_pct, COL_GRAY, 0);
    lv_obj_set_style_text_font(lbl_luminosita_pct, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_luminosita_pct, "80%");
    lv_obj_align(lbl_luminosita_pct, LV_ALIGN_CENTER, 0, 35);

    panda_disegna_pallini(scr_set_luminosita, 0);

    // -----------------------------------------------------------------
    // 6. IMPOSTAZIONI: ORARIO (pagina 2 di 3)
    // -----------------------------------------------------------------
    scr_set_orario = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_set_orario, COL_BG, 0);
    lv_obj_add_event_cb(scr_set_orario, settings_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr_set_orario, cb_settings_page_loaded, LV_EVENT_SCREEN_LOADED, NULL);
    panda_aggiungi_chiudi_x(scr_set_orario);

    lv_obj_t *lbl_o1 = lv_label_create(scr_set_orario);
    lv_label_set_text(lbl_o1, "IMPOSTAZIONI");
    lv_obj_set_style_text_color(lbl_o1, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_o1, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_o1, LV_ALIGN_CENTER, 0, -112);

    lv_obj_t *lbl_o2 = lv_label_create(scr_set_orario);
    lv_label_set_text(lbl_o2, "ORARIO");
    lv_obj_set_style_text_color(lbl_o2, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_o2, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_o2, LV_ALIGN_CENTER, 0, -87);

    lbl_set_ora = panda_crea_colonna(scr_set_orario, -55, cb_ora_su, cb_ora_giu);
    lbl_set_min = panda_crea_colonna(scr_set_orario, 55, cb_min_su, cb_min_giu);

    lv_obj_t *lbl_due_punti = lv_label_create(scr_set_orario);
    lv_label_set_text(lbl_due_punti, ":");
    lv_obj_set_style_text_color(lbl_due_punti, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_due_punti, &lv_font_montserrat_40, 0);
    lv_obj_align(lbl_due_punti, LV_ALIGN_CENTER, 0, 0);

    panda_disegna_pallini(scr_set_orario, 1);

    // -----------------------------------------------------------------
    // 7. IMPOSTAZIONI: DATA (pagina 3 di 3)
    // -----------------------------------------------------------------
    scr_set_data = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_set_data, COL_BG, 0);
    lv_obj_add_event_cb(scr_set_data, settings_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr_set_data, cb_settings_page_loaded, LV_EVENT_SCREEN_LOADED, NULL);
    panda_aggiungi_chiudi_x(scr_set_data);

    lv_obj_t *lbl_d1 = lv_label_create(scr_set_data);
    lv_label_set_text(lbl_d1, "IMPOSTAZIONI");
    lv_obj_set_style_text_color(lbl_d1, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_d1, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_d1, LV_ALIGN_CENTER, 0, -112);

    lv_obj_t *lbl_d2 = lv_label_create(scr_set_data);
    lv_label_set_text(lbl_d2, "DATA");
    lv_obj_set_style_text_color(lbl_d2, COL_TXT, 0);
    lv_obj_set_style_text_font(lbl_d2, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_d2, LV_ALIGN_CENTER, 0, -87);

    // Tre campi regolabili, tutti con lo stesso font e la stessa struttura:
    // giorno della settimana | giorno del mese | mese
    lbl_set_weekday = panda_crea_colonna(scr_set_data, -118, cb_weekday_su, cb_weekday_giu);
    lbl_set_giorno  = panda_crea_colonna(scr_set_data, 0,    cb_giorno_su,  cb_giorno_giu);
    lbl_set_mese    = panda_crea_colonna(scr_set_data, 118,  cb_mese_su,    cb_mese_giu);

    panda_disegna_pallini(scr_set_data, 2);

    // Parte dallo splash; Test_panda.ino passa a scr_clock dopo 10 secondi
    lv_scr_load(scr_splash);
}

#endif
