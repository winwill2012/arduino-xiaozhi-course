<a href="https://github.com/sponsors/lvgl" target="_blank"><img align="left" src="https://lvgl.io/github-assets/sponsor.png" height="32px"></a>

<p align="right">
<a href="../README.md">English</a> | <b>Chinese</b> | <a href="./README_pt_BR.md">Português do Brasil</a> | <a href="./README_jp.md">Japanese</a>
</p>
<br>

<p align="center">
  <img src="https://lvgl.io/github-assets/logo-colored.png" width=300px>
</p>


<h1 align="center">
  Light and Versatile Graphics Library
</h1>

<h1 align="center">
Lightweight universal graphics library
</h1>

<div align="center">
  <img src="https://raw.githubusercontent.com/kisvegabor/test/master/smartwatch_demo.gif">
  &nbsp;
  <img border="1px" src="https://lvgl.io/github-assets/widgets-demo.gif">
</div>

<br>

<p align="center">
<a href="https://lvgl.io" title="Homepage of LVGL">Official Website </a> |
<a href="https://docs.lvgl.io/" title="Detailed documentation with 100+ examples">Document</a> |
<a href="https://forum.lvgl.io" title="Get help and help others">Forum</a> |
<a href="https://lvgl.io/demos" title="Demos running in your browser">Example</a> |
<a href="https://lvgl.io/services" title="Graphics design, UI implementation and consulting">Services</a>
</p>


[Chinese flyer](./flyers/LVGL-Chinese-Flyer.pdf)

## :ledger: directory

- [:ledger: directory](#ledger-directory)
- [:rocket: Overview and Overview](#rocket-Overview and Overview)
- [Hardware Requirements](#Hardware Requirements)
- [:package: already supported platform](#package-already supported platform)
- [How to get started](#How to get started)
- [:robot: routine](#robot-routine)
- [Button with Click Event button and click event](#button-with-click-event button and click event)
  - [Micropython](#micropython)
- [Checkboxes with Layout Checkboxes with Layout](#checkboxes-with-layout-checkboxes with Layout)
- [Styling a Slider Sets the style of the slider](#styling-a-slider-Sets the style of the slider)
- [English, Hebrew (mixed LRT-RTL) and Chinese texts English, Hebrew (two-way text typesetting) and Chinese](#english-hebrew-mixed-lrt-rtl-and-chinese-texts-English Hebrew-two-way text typesetting-and Chinese)
- [:handshake: Service](#handshake-Service)
- [:star: How to contribute to the community](#star-How to contribute to the community)

## :rocket: Overview and Overview

**characteristic**

- Rich and powerful modular [graphic components] (https://docs.lvgl.io/master/details/widgets/index.html): button
(buttons), charts, lists, sliders, pictures
(images) etc.
- Advanced graphics engine: animation, anti-aliasing, transparency, smooth scrolling, layer mixing and other effects
- Supports multiple [input devices] (https://docs.lvgl.io/master/details/main-modules/indev.html): touch screen, keyboard, encoder, keys, etc.
- Support [multiple display devices](https://docs.lvgl.io/master/details/main-modules/display/index.html)
- No dependency on a specific hardware platform, can run on any display
- Configure clipping (minimum resource occupancy: 64 kB Flash, 16 kB RAM)
- Multilingual support based on UTF-8, such as Chinese, Japanese, Korean, Arabic, etc.
- You can design and layout the graphical interface through [CSS](https://docs.lvgl.io/master/details/common-widget-features/styles/style.html) (for example: [Flexbox](https://docs.lvgl.io/master/details/common-widget-features/layouts/flex.html), [Grid](https://docs.lvgl.io/master/details/common-widget-features/layouts/grid.html))
- Supports operating system, external memory, and hardware acceleration (LVGL has built-in support for STM32 DMA2D, SWM341 DMA2D, NXP PXP and VGLite)
- Even if there is only [frame buffer](https://docs.lvgl.io/master/details/main-modules/display/index.html), the rendering can be guaranteed to be as smooth as silky
- All written by C and supports C++ calls
- Support Micropython programming, see: [LVGL API in Micropython](https://blog.lvgl.io/2019-02-20/micropython-bindings)
- Supports [Simulator](https://docs.lvgl.io/master/details/integration/ide/pc-simulator.html) simulation, and can be developed without hardware support.
- Rich and detailed [routines](https://github.com/lvgl/lvgl/tree/master/examples)
- Detailed [documents] (http://docs.lvgl.io/) and API reference manual, available for online viewing or downloadable in PDF format

### Hardware Requirements

<table>
   <tr>
<td><strong>Requirements</strong></td>
<td><strong>Minimum Requirements</strong></td>
<td><strong>Suggested Requirements</strong></td>
   </tr>
   <tr>
<td><strong>Architecture</strong></td>
<td colspan="2">16, 32, 64-bit microcontroller or microprocessor</td>
   </tr>
   <tr>
<td><strong>Clock</strong></td>
      <td> &gt; 16 MHz</td>
      <td> &gt; 48 MHz</td>
   </tr>
   <tr>
      <td><strong>Flash/ROM</strong></td>
      <td> &gt; 64 kB</td>
      <td> &gt; 180 kB</td>
   </tr>
   <tr>
      <td><strong>Static RAM</strong></td>
      <td> &gt; 16 kB</td>
      <td> &gt; 48 kB</td>
   </tr>
   <tr>
      <td><strong>Draw buffer</strong></td>
      <td> &gt; 1 × <em>hor. res.</em> pixels</td>
<td> &gt; 1/10 screen size </td>
   </tr>
   <tr>
<td><strong>Compiler</strong></td>
<td colspan="2">C99 or updated </td>
   </tr>
</table>

*Note: Resource usage is related to specific hardware platform, compiler and other factors. Only reference values ​​are given in the above table*

### :package: already supported platforms

LVGL itself does not rely on a specific hardware platform, and any microcontroller that meets the LVGL hardware configuration requirements can run LVGL.
Only some of them are listed below:

-  NXP: Kinetis, LPC, iMX, iMX RT
-  STM32F1, STM32F3, STM32F4, STM32F7, STM32L4, STM32L5, STM32H7
-  Microchip dsPIC33, PIC24, PIC32MX, PIC32MZ
-  [Linux frame buffer](https://blog.lvgl.io/2018-01-03/linux_fb) (/dev/fb)
-  [Espressif ESP32](https://github.com/lvgl/lv_port_esp32)
-  [Infineon Aurix](https://github.com/lvgl/lv_port_aurix)
-  Nordic NRF52 Bluetooth modules
-  Quectel modems
-  [SYNWIT SWM341](https://www.synwit.cn/)

LVGL also supports:
- [Arduino library](https://docs.lvgl.io/master/details/integration/framework/arduino.html)
- [PlatformIO package](https://registry.platformio.org/libraries/lvgl/lvgl)
- [Zephyr library](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_LVGL)
- [ESP32 component](https://docs.lvgl.io/master/details/integration/chip/espressif.html)
- [NXP MCUXpresso component](https://www.nxp.com/design/software/embedded-software/lvgl-open-source-graphics-library:LITTLEVGL-OPEN-SOURCE-GRAPHICS-LIBRARY)
- [NuttX library](https://docs.lvgl.io/master/details/integration/os/nuttx.html)
- [RT-Thread RTOS](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/packages-manual/lvgl-docs/introduction)



## How to get started

Please learn LVGL in the following order:
1. Use [Web Online Routine] (https://lvgl.io/demos) to experience LVGL (3 minutes)
2. Read the [Introduction](https://docs.lvgl.io/master/intro/introduction.html) chapter to gain a preliminary understanding of LVGL (5 minutes)
3. Read the quick [Quick Overview] (https://docs.lvgl.io/master/intro/getting_started.html) chapter to understand the basic knowledge of LVGL (15 minutes)
4. Learn how to use [Simulator] (https://docs.lvgl.io/master/details/integration/ide/pc-simulator.html) to simulate LVGL on your computer (10 minutes)
5. Try to practice some [routines](https://github.com/lvgl/lvgl/tree/master/examples)
6. Refer to [Migration Guide] (https://docs.lvgl.io/master/details/integration/adding-lvgl-to-your-project/index.html) to try to port LVGL to a development board. LVGL has also provided some portable [Projects] (https://github.com/lvgl?q=lv_port_)
7. Read the document [Overview](https://docs.lvgl.io/master/details/main-modules/index.html) chapters to gain a deeper understanding and familiarity with LVGL (2-3 hours)
8. Browse the documentation [Components (Widgets)](https://docs.lvgl.io/master/details/widgets/index.html) section to learn how to use them
9. If you have any questions, you can ask questions at LVGL [forum] (http://forum.lvgl.io/)
10. Read the document [How to contribute to the community] (https://docs.lvgl.io/master/contributing/index.html) chapter to see what you can do to the LVGL community to promote the continuous improvement of the quality of LVGL software (15 minutes)


## :robot: routine

For more routines, see the [examples](https://github.com/lvgl/lvgl/tree/master/examples) folder.

### Button with Click Event

![Button with Click Event](https://raw.githubusercontent.com/kisvegabor/test/master/readme_example_2.gif)

<details>
  <summary>C code</summary>

```c
lv_obj_t * btn = lv_button_create(lv_screen_active());       /*Add a button to the current screen*/
lv_obj_center(btn);                                          /*Set its position*/
lv_obj_set_size(btn, 100, 50);                               /*Set its size*/
lv_obj_add_event(btn, btn_event_cb, LV_EVENT_CLICKED, NULL); /*Assign a callback to the button*/

lv_obj_t * label = lv_label_create(btn);                     /*Add a label to the button*/
lv_label_set_text(label, "Button");                          /*Set the labels text*/
lv_obj_center(label);                                        /*Align the label to the center*/
...

void btn_event_cb(lv_event_t * e)
{
  printf("Clicked\n");
}
```

</details>


### Micropython

For more information, please go to [Micropython official website](https://docs.lvgl.io/master/get-started/bindings/micropython.html) to query.

<details>
  <summary>MicroPython code | <a href="https://sim.lvgl.io/v8.3/micropython/ports/javascript/index.html?script_startup=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/header.py&script=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/widgets/slider/lv_example_slider_2.py&script_direct=926bde43ec7af0146c486de470c53f11f167491e" target="_blank">Online Simulator</a> :gb:</summary>

```python
def btn_event_cb(e):
  print("Clicked")

# Create a Button and a Label
btn = lv.btn(lv.screen_active())
btn.center()
btn.set_size(100, 50)
btn.add_event(btn_event_cb, lv.EVENT.CLICKED, None)

label = lv.label(btn)
label.set_text("Button")
label.center()
```

</details>
<br>


### Checkboxes with Layout Checkboxes with Layout
![Checkboxes with layout in LVGL](https://raw.githubusercontent.com/kisvegabor/test/master/readme_example_3.gif)

<details>
  <summary>C code</summary>

```c

lv_obj_set_flex_flow(lv_screen_active(), LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(lv_screen_active(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

lv_obj_t * cb;
cb = lv_checkbox_create(lv_screen_active());
lv_checkbox_set_text(cb, "Apple");
lv_obj_add_event(cb, event_handler, LV_EVENT_ALL, NULL);

cb = lv_checkbox_create(lv_screen_active());
lv_checkbox_set_text(cb, "Banana");
lv_obj_add_state(cb, LV_STATE_CHECKED);
lv_obj_add_event(cb, event_handler, LV_EVENT_ALL, NULL);

cb = lv_checkbox_create(lv_screen_active());
lv_checkbox_set_text(cb, "Lemon");
lv_obj_add_state(cb, LV_STATE_DISABLED);
lv_obj_add_event(cb, event_handler, LV_EVENT_ALL, NULL);

cb = lv_checkbox_create(lv_screen_active());
lv_obj_add_state(cb, LV_STATE_CHECKED | LV_STATE_DISABLED);
lv_checkbox_set_text(cb, "Melon\nand a new line");
lv_obj_add_event(cb, event_handler, LV_EVENT_ALL, NULL);
```

</details>


<details>
  <summary>MicroPython code | <a href="https://sim.lvgl.io/v8.3/micropython/ports/javascript/index.html?script_startup=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/header.py&script=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/widgets/slider/lv_example_slider_2.py&script_direct=311d37e5f70daf1cb0d2cad24c7f72751b5f1792" target="_blank">Online Simulator</a> :gb:</summary>

```python
def event_handler(e):
    code = e.get_code()
    obj = e.get_target_obj()
    if code == lv.EVENT.VALUE_CHANGED:
        txt = obj.get_text()
        if obj.get_state() & lv.STATE.CHECKED:
            state = "Checked"
        else:
            state = "Unchecked"
        print(txt + ":" + state)


lv.scr_act().set_flex_flow(lv.FLEX_FLOW.COLUMN)
lv.scr_act().set_flex_align(lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.START, lv.FLEX_ALIGN.CENTER)

cb = lv.checkbox(lv.screen_active())
cb.set_text("Apple")
cb.add_event(event_handler, lv.EVENT.ALL, None)

cb = lv.checkbox(lv.screen_active())
cb.set_text("Banana")
cb.add_state(lv.STATE.CHECKED)
cb.add_event(event_handler, lv.EVENT.ALL, None)

cb = lv.checkbox(lv.screen_active())
cb.set_text("Lemon")
cb.add_state(lv.STATE.DISABLED)
cb.add_event(event_handler, lv.EVENT.ALL, None)

cb = lv.checkbox(lv.screen_active())
cb.add_state(lv.STATE.CHECKED | lv.STATE.DISABLED)
cb.set_text("Melon")
cb.add_event(event_handler, lv.EVENT.ALL, None)
```

</details>


### Style a Slider Sets the slider style
![Styling a slider with LVGL](https://raw.githubusercontent.com/kisvegabor/test/master/readme_example_4.gif)


<details>
  <summary>C code</summary>

```c
lv_obj_t * slider = lv_slider_create(lv_screen_active());
lv_slider_set_value(slider, 70, LV_ANIM_OFF);
lv_obj_set_size(slider, 300, 20);
lv_obj_center(slider);

/*Add local styles to MAIN part (background rectangle)*/
lv_obj_set_style_bg_color(slider, lv_color_hex(0x0F1215), LV_PART_MAIN);
lv_obj_set_style_bg_opa(slider, 255, LV_PART_MAIN);
lv_obj_set_style_border_color(slider, lv_color_hex(0x333943), LV_PART_MAIN);
lv_obj_set_style_border_width(slider, 5, LV_PART_MAIN);
lv_obj_set_style_pad_all(slider, 5, LV_PART_MAIN);

/*Create a reusable style sheet for the INDICATOR part*/
static lv_style_t style_indicator;
lv_style_init(&style_indicator);
lv_style_set_bg_color(&style_indicator, lv_color_hex(0x37B9F5));
lv_style_set_bg_grad_color(&style_indicator, lv_color_hex(0x1464F0));
lv_style_set_bg_grad_dir(&style_indicator, LV_GRAD_DIR_HOR);
lv_style_set_shadow_color(&style_indicator, lv_color_hex(0x37B9F5));
lv_style_set_shadow_width(&style_indicator, 15);
lv_style_set_shadow_spread(&style_indicator, 5);

/*Add the style sheet to the slider's INDICATOR part*/
lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);

/*Add the same style to the KNOB part too and locally overwrite some properties*/
lv_obj_add_style(slider, &style_indicator, LV_PART_KNOB);

lv_obj_set_style_outline_color(slider, lv_color_hex(0x0096FF), LV_PART_KNOB);
lv_obj_set_style_outline_width(slider, 3, LV_PART_KNOB);
lv_obj_set_style_outline_pad(slider, -5, LV_PART_KNOB);
lv_obj_set_style_shadow_spread(slider, 2, LV_PART_KNOB);
```

</details>

<details>
  <summary>MicroPython code |
<a href="https://sim.lvgl.io/v8.3/micropython/ports/javascript/index.html?script_startup=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/header.py&script=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/widgets/slider/lv_example_slider_2.py&script_direct=c431c7b4dfd2cc0dd9c392b74365d5af6ea986f0" target="_blank">Online Simulator</a> :gb:
</summary>


```python
# Create a slider and add the style
slider = lv.slider(lv.screen_active())
slider.set_value(70, lv.ANIM.OFF)
slider.set_size(300, 20)
slider.center()

# Add local styles to MAIN part (background rectangle)
slider.set_style_bg_color(lv.color_hex(0x0F1215), lv.PART.MAIN)
slider.set_style_bg_opa(255, lv.PART.MAIN)
slider.set_style_border_color(lv.color_hex(0x333943), lv.PART.MAIN)
slider.set_style_border_width(5, lv.PART.MAIN)
slider.set_style_pad_all(5, lv.PART.MAIN)

# Create a reusable style sheet for the INDICATOR part
style_indicator = lv.style_t()
style_indicator.init()
style_indicator.set_bg_color(lv.color_hex(0x37B9F5))
style_indicator.set_bg_grad_color(lv.color_hex(0x1464F0))
style_indicator.set_bg_grad_dir(lv.GRAD_DIR.HOR)
style_indicator.set_shadow_color(lv.color_hex(0x37B9F5))
style_indicator.set_shadow_width(15)
style_indicator.set_shadow_spread(5)

# Add the style sheet to the slider's INDICATOR part
slider.add_style(style_indicator, lv.PART.INDICATOR)
slider.add_style(style_indicator, lv.PART.KNOB)

# Add the same style to the KNOB part too and locally overwrite some properties
slider.set_style_outline_color(lv.color_hex(0x0096FF), lv.PART.KNOB)
slider.set_style_outline_width(3, lv.PART.KNOB)
slider.set_style_outline_pad(-5, lv.PART.KNOB)
slider.set_style_shadow_spread(2, lv.PART.KNOB)
```
</details>
<br>


### English, Hebrew (mixed LRT-RTL) and Chinese texts English, Hebrew (two-way text typesetting) and Chinese

![English, Hebrew and Chinese texts with LVGL](https://raw.githubusercontent.com/kisvegabor/test/master/readme_example_5.png)

<details>
  <summary>C code</summary>

```c
lv_obj_t * ltr_label = lv_label_create(lv_screen_active());
lv_label_set_text(ltr_label, "In modern terminology, a microcontroller is similar to a system on a chip (SoC).");
lv_obj_set_style_text_font(ltr_label, &lv_font_montserrat_16, 0);
lv_obj_set_width(ltr_label, 310);
lv_obj_align(ltr_label, LV_ALIGN_TOP_LEFT, 5, 5);

lv_obj_t * rtl_label = lv_label_create(lv_screen_active());
lv_label_set_text(rtl_label,"מעבד, או בשמו המלא יחידת עיבוד מרכזית (באנגלית: CPU - Central Processing Unit).");
lv_obj_set_style_base_dir(rtl_label, LV_BASE_DIR_RTL, 0);
lv_obj_set_style_text_font(rtl_label, &lv_font_dejavu_16_persian_hebrew, 0);
lv_obj_set_width(rtl_label, 310);
lv_obj_align(rtl_label, LV_ALIGN_LEFT_MID, 5, 0);

lv_obj_t * cz_label = lv_label_create(lv_screen_active());
lv_label_set_text(cz_label,
"Embedded System,\n is a computer system embedded in mechanical or electrical systems, with dedicated functions and real-time computing performance.");
lv_obj_set_style_text_font(cz_label, &lv_font_source_han_sans_sc_16_cjk, 0);
lv_obj_set_width(cz_label, 310);
lv_obj_align(cz_label, LV_ALIGN_BOTTOM_LEFT, 5, -5);
```

</details>

<details>
  <summary>MicroPython code | <a href="https://sim.lvgl.io/v8.3/micropython/ports/javascript/index.html?script_startup=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/header.py&script=https://raw.githubusercontent.com/lvgl/lvgl/0d9ab4ee0e591aad1970e3c9164fd7c544ecce70/examples/widgets/slider/lv_example_slider_2.py&script_direct=18bb38200a64e10ead1aa17a65c977fc18131842" target="_blank">Online Simulator</a> :gb:</summary>

```python
ltr_label = lv.label(lv.screen_active())
ltr_label.set_text("In modern terminology, a microcontroller is similar to a system on a chip (SoC).")
ltr_label.set_style_text_font(lv.font_montserrat_16, 0);

ltr_label.set_width(310)
ltr_label.align(lv.ALIGN.TOP_LEFT, 5, 5)

rtl_label = lv.label(lv.screen_active())
rtl_label.set_text("מעבד, או בשמו המלא יחידת עיבוד מרכזית (באנגלית: CPU - Central Processing Unit).")
rtl_label.set_style_base_dir(lv.BASE_DIR.RTL, 0)
rtl_label.set_style_text_font(lv.font_dejavu_16_persian_hebrew, 0)
rtl_label.set_width(310)
rtl_label.align(lv.ALIGN.LEFT_MID, 5, 0)

font_han_sans_16_cjk = lv.font_load("S:../../assets/font/lv_font_source_han_sans_sc_16_cjk.fnt")

cz_label = lv.label(lv.screen_active())
cz_label.set_style_text_font(font_han_sans_16_cjk, 0)
cz_label.set_text("Embedded System",\n is a computer system embedded in mechanical or electrical systems with dedicated functions and real-time computing performance.")
cz_label.set_width(310)
cz_label.align(lv.ALIGN.BOTTOM_LEFT, 5, -5)

```
</details>



## :handshake: Service

LVGL
The purpose of the establishment of Responsibility Co., Ltd. is to provide additional technical support for users to use the LVGL graphics library. We are committed to providing the following services:

- Graphic Design
- UI Design
- Technical consultation and technical support

For more information, please see https://lvgl.io/services, please feel free to contact us if you have any questions.


## :star: How to contribute to the community

LVGL is an open source project, and you are very welcome to participate in community contributions. There are many ways you can contribute to your efforts to improve LVGL, including but not limited to:

- Introduce your works or projects based on LVGL design
- Write some routines
- Modify and improve documents
- Fix bug

Please refer to the documentation [How to contribute to the community] (https://docs.lvgl.io/master/contributing/index.html) section for more information.
