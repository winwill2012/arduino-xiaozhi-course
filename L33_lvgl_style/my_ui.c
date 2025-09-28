//
// Created by Admin on 2025/7/7.
//

#include "my_ui.h"
#include "lvgl.h"

void show_ui()
{
    lv_obj_t* obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 240, 200);
    lv_obj_set_align(obj, LV_ALIGN_CENTER);

    // // //****************** Method 1: Create the style first, then apply the style. The advantage is that it is convenient to reuse the style to other objects*******************************************
    // // // The static here must be added. If it is not added, it is a local variable. It will be destroyed after the show_ui function call is finished, which will cause the program to crash
    // static lv_style_t style;
    // lv_style_init(&style);
    // //
    // // Basic settings
    // lv_style_set_bg_color(&style, lv_color_hex(0xDDDDDD)); // Set the background color to light gray
    // lv_style_set_border_width(&style, 10); // Set the border width to 10
    // lv_style_set_border_color(&style, lv_color_hex(0xFF0000)); // Set the border color to red
    // lv_style_set_radius(&style, 5); // Add 5 pixels of rounded corners
    // lv_style_set_pad_all(&style, 10); // Set the inner margin to 10 pixels
    // //
    // // Set outline
    // lv_style_set_outline_width(&style, 1); // Set the width of the outline to 1
    // lv_style_set_outline_color(&style, lv_color_hex(0x0000FF)); // Set the outline to blue
    // lv_style_set_outline_opa(&style, 128); // Set the transparency of the outline
    // lv_style_set_outline_pad(&style, 10); // Set the filling distance between the outline and the border
    // //
    // // Set edge shadow
    // lv_style_set_shadow_width(&style, 100); // Set edge shadow width to 10
    // lv_style_set_shadow_color(&style, lv_color_hex(0x00FF00)); // Set the edge shadow color to green
    // lv_style_set_shadow_opa(&style, 255); // Set edge shadow transparency to 128
    // lv_style_set_shadow_offset_x(&style, 10); // The shadow is offset by 10 to the right
    // lv_style_set_shadow_offset_y(&style, -10); // Shadow is offset upward by 10
    // //
    // // // Bind styles for objects
    // lv_obj_add_style(obj, &style, 0);

    // **************** Method 2: Use lv_obj_set_style_xxx style**********************************************
    // Basic settings
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xDDDDDD), 0);         // Set the background color to light gray
    lv_obj_set_style_border_width(obj, 10, 0);                          // Set the border width to 10
    lv_obj_set_style_border_color(obj, lv_color_hex(0xFF0000), 0);     // Set the border color to red
    lv_obj_set_style_radius(obj, 5, 0);                                 // Add 5 pixels of rounded corners
    lv_obj_set_style_pad_all(obj, 10, 0);                               // Set the inner margin to 10 pixels

    // Setting outlines
    lv_obj_set_style_outline_width(obj, 1, 0);                          // Set the width of the outline to 1
    lv_obj_set_style_outline_color(obj, lv_color_hex(0x0000FF), 0);    // Set the outline to blue
    lv_obj_set_style_outline_opa(obj, 128, 0);                          // Set the transparency of the outline
    lv_obj_set_style_outline_pad(obj, 20, 0);                           // Set the filling distance between the outline and the border

    // Set edge shadows
    lv_obj_set_style_shadow_width(obj, 10, 0);                          // Set edge shadow width to 10
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x00FF00), 0);     // Set the edge shadow color to green
    lv_obj_set_style_shadow_opa(obj, 128, 0);                           // Set edge shadow transparency to 128
    lv_obj_set_style_shadow_ofs_x(obj, 10, 0);                          // The shadow is offset by 10 to the right
    lv_obj_set_style_shadow_ofs_y(obj, -10, 0);                         // Shadow offset 10 upwards
}
