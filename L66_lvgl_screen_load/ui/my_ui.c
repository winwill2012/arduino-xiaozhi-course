#include "my_ui.h"

#include "src/core/lv_obj.h"
#include "src/widgets/button/lv_button.h"
#include "src/widgets/label/lv_label.h"

static lv_obj_t *screen1;
static lv_obj_t *screen2;

void screen1_event_cb(lv_event_t *event) {
    if (lv_event_get_code(event) == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_LEFT) {
            lv_indev_wait_release(lv_indev_active());
            lv_screen_load_anim(screen2, LV_SCREEN_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
        }
    }
}

void screen2_event_cb(lv_event_t *event) {
    if (lv_event_get_code(event) == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_RIGHT) {
            lv_indev_wait_release(lv_indev_active());
            lv_screen_load_anim(screen1, LV_SCREEN_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
        }
    }
}

void show_ui() {
    screen1 = lv_obj_create(NULL);
    lv_obj_t *button1 = lv_button_create(screen1);
    lv_obj_set_size(button1, 150, 50);
    lv_obj_center(button1);
    lv_obj_t *label1 = lv_label_create(button1);
    lv_obj_center(label1);
    lv_label_set_text(label1, "Click to screen2");

    screen2 = lv_obj_create(NULL);
    lv_obj_t *button2 = lv_button_create(screen2);
    lv_obj_set_size(button2, 150, 50);
    lv_obj_center(button2);
    lv_obj_t *label2 = lv_label_create(button2);
    lv_obj_center(label2);
    lv_label_set_text(label2, "Click to screen1");

    lv_screen_load(screen1);

    lv_obj_add_event_cb(screen1, screen1_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(screen2, screen2_event_cb, LV_EVENT_GESTURE, NULL);
}
