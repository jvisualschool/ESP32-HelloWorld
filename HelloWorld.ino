#include <Arduino.h>
#include <lvgl.h>
#include "display.h"
#include "esp_bsp.h"
#include "lv_port.h"

#define LVGL_PORT_ROTATION_DEGREE (90)

enum Theme {
    THEME_DARK,
    THEME_LIGHT,
    THEME_LOW_POWER
};

static Theme current_theme = THEME_DARK;
static int target_brightness = 100;
static int current_brightness = 100;

static lv_obj_t *hello_label;
static lv_obj_t *title;
static lv_obj_t *ok_icon;
static lv_obj_t *status_label;
static lv_obj_t *dev_info;
static lv_obj_t *line;

// 트랜지션 설정 (애니메이션 효과)
static lv_style_transition_dsc_t trans_dsc;
static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, LV_STYLE_TEXT_COLOR, LV_STYLE_LINE_COLOR, (lv_style_prop_t)0};

void update_theme()
{
    switch (current_theme) {
        case THEME_DARK:
            target_brightness = 100;
            lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
            lv_obj_set_style_text_color(hello_label, lv_color_hex(0xFF4500), 0);
            lv_obj_set_style_text_color(title, lv_color_hex(0x00BFFF), 0);
            lv_obj_set_style_text_color(ok_icon, lv_color_hex(0x00FF00), 0);
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x00FF00), 0);
            lv_obj_set_style_text_color(dev_info, lv_color_hex(0xAAAAAA), 0);
            lv_obj_set_style_line_color(line, lv_color_hex(0x444444), 0);
            break;

        case THEME_LIGHT:
            target_brightness = 100;
            lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_color(hello_label, lv_color_hex(0xD32F2F), 0);
            lv_obj_set_style_text_color(title, lv_color_hex(0x1976D2), 0);
            lv_obj_set_style_text_color(ok_icon, lv_color_hex(0x388E3C), 0);
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x388E3C), 0);
            lv_obj_set_style_text_color(dev_info, lv_color_hex(0x616161), 0);
            lv_obj_set_style_line_color(line, lv_color_hex(0xBDBDBD), 0);
            break;

        case THEME_LOW_POWER:
            target_brightness = 5; // 아주 낮게
            lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
            lv_obj_set_style_text_color(hello_label, lv_color_hex(0x331100), 0);
            lv_obj_set_style_text_color(title, lv_color_hex(0x002244), 0);
            lv_obj_set_style_text_color(ok_icon, lv_color_hex(0x00BF00), 0);
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x004400), 0);
            lv_obj_set_style_text_color(dev_info, lv_color_hex(0x222222), 0);
            lv_obj_set_style_line_color(line, lv_color_hex(0x111111), 0);
            break;
    }
}

static void screen_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        if (current_theme == THEME_DARK) current_theme = THEME_LIGHT;
        else if (current_theme == THEME_LIGHT) current_theme = THEME_LOW_POWER;
        else current_theme = THEME_DARK;

        update_theme();

        const char* theme_name = "";
        if (current_theme == THEME_DARK) theme_name = "DARK";
        else if (current_theme == THEME_LIGHT) theme_name = "LIGHT";
        else theme_name = "LOW_POWER";

        Serial.printf("Theme changed: %s\n", theme_name);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== LVGL Re-Design v1.5 Starting ===");

    Serial.println("Initialize panel device");
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
        .rotate = LV_DISP_ROT_90,
#elif LVGL_PORT_ROTATION_DEGREE == 270
        .rotate = LV_DISP_ROT_270,
#elif LVGL_PORT_ROTATION_DEGREE == 180
        .rotate = LV_DISP_ROT_180,
#elif LVGL_PORT_ROTATION_DEGREE == 0
        .rotate = LV_DISP_ROT_NONE,
#endif
    };

    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    Serial.println("Create Commemoration UI");
    bsp_display_lock(0);

    // 트랜지션 초기화 (300ms 동안 선형적으로 변화)
    lv_style_transition_dsc_init(&trans_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    // 0. 화면 전체 클릭 이벤트 등록
    lv_obj_add_flag(lv_scr_act(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lv_scr_act(), screen_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_transition(lv_scr_act(), &trans_dsc, 0);

    // 배경 초기 설정
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    // 1. [상단] HELLO WORLD!
    hello_label = lv_label_create(lv_scr_act());
    lv_label_set_text(hello_label, "HELLO WORLD!");
    lv_obj_set_style_text_font(hello_label, &lv_font_montserrat_48, 0);
    lv_obj_align(hello_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_transition(hello_label, &trans_dsc, 0);

    // 2. [중앙] ESP32-S3 CORE
    title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "ESP32-S3 SYSTEM");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_transition(title, &trans_dsc, 0);

    // 3. [중앙하단] OK 아이콘 및 상태 메시지
    ok_icon = lv_label_create(lv_scr_act());
    lv_label_set_text(ok_icon, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(ok_icon, &lv_font_montserrat_48, 0);
    lv_obj_align(ok_icon, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_transition(ok_icon, &trans_dsc, 0);

    status_label = lv_label_create(lv_scr_act());
    lv_label_set_text(status_label, "Hardware Driver Initialized Successful");
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_align_to(status_label, ok_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_transition(status_label, &trans_dsc, 0);

    // 4. [하단] 날짜 및 개발자 정보
    dev_info = lv_label_create(lv_scr_act());
    lv_label_set_text(dev_info, "Date: 2026-02-08  |  Developer: Jinho Jung");
    lv_obj_set_style_text_font(dev_info, &lv_font_montserrat_16, 0);
    lv_obj_align(dev_info, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_transition(dev_info, &trans_dsc, 0);

    // 5. 장식용 하단 구분선
    line = lv_line_create(lv_scr_act());
    static lv_point_t line_points[] = { {0, 0}, {400, 0} };
    lv_line_set_points(line, line_points, 2);
    lv_obj_set_style_line_width(line, 2, 0);
    lv_obj_align(line, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_set_style_transition(line, &trans_dsc, 0);

    // 초기 테마 적용
    update_theme();

    bsp_display_unlock();

    Serial.println("=== Commemoration UI Complete ===");
}

void loop()
{
    // 부드러운 백라이트 전환 처리 (10ms 마다 1%씩 목표치로 이동)
    if (current_brightness != target_brightness) {
        if (current_brightness < target_brightness) current_brightness++;
        else current_brightness--;
        
        bsp_display_brightness_set(current_brightness);
    }
    delay(10);
}
