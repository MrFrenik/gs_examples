// gui.ss 

//*==========================*//
//*======= Description ======*//

/*
    
    GSS is a tiny supported subset of CSS and used to load style sheets
    to be used with gs_gui contexts.

    //=== General Format ===//

    All styles follow the following format:

        <element | id>: [state] { <styles>; } 

        <styles>: style: value | transition: {<transitions}

    All states for an element are optional and inherit from the 
        default state then override with their own styles.  

    //=== Selectors ===//

    All selectors fall into the following categories: 
        * <element>
        * <id>
        * <state> 

    //=== Elements ===//

    List of all available <element>: 
        * container
        * label
        * text
        * panel
        * input
        * button
        * scroll
        * image 

    //=== ID ===//
    
    A single ID selector can be used for each definition.
    ID selectors have the following format:
    
        #your_id 

    //=== Class ===//

    A selector that defines a group of styles that be can used for multiple elements.
    Each gui element can have up to GS_GUI_CLS_SELECTOR_MAX class selectors. This can 
    can increased or decreased by changing this define in gs/util/gs_gui.h.
    Class selectors have the following format: 

        .your_cls

    //=== Wildcard ===//
    
    A selector that applies to all elements.

        * 

    //=== State ===//

    List of all available element states: 
        * hover
        * active

    //=== Order of Operations ===//

    Order of selector style application for final styling: 
        * Element Type
        * Individual Classes
        * Highest Combined Type + Class Hash
        * ID 

    Unlike traditional CSS, the order in which the class attributes appears in the descriptor list
    matters.

    Ex: 
        button        // Type selector
        #green        // ID selector
        .red          // Class selector
        *             // Wildcard selector

        gs_gui_selector_desc_t btn = {
            .id = "green", 
            .classes = {"red", "blue"}  // Apply red first then blue then the combined hash of all of them
            .class_count = 4
        };
        gs_gui_button_ex(gui, "btn_text", &btn, 0x00);
    

    States are OPTIONAL, and an empty state will affect the default style. This default style gets applied to all states.

    //=== Styles ===//

    List of all available styles and their expected inputs: 
        * color_content:    rgb | rgba
        * color_border:     rgb | rgba
        * width:            int16_t
        * height:           int16_t
        * margin:           int16_t
        * margin_left:      int16_t
        * margin_right      int16_t
        * margin_top:       int16_t
        * margin_bottom:    int16_t
        * padding:          int16_t
        * padding_left:     int16_t
        * padding_top:      int16_t
        * padding_bottom:   int16_t
        * padding_right:    int16_t
        * border:           int16_t
        * border_left:      int16_t
        * border_right:     int16_t
        * border_top:       int16_t
        * border_bottom:    int16_t
        * justify_content:  start | center | end
        * align_content:    start | center | end
        * shadow:           int16_t
        * font:             string
        * transition:       {transitions}

    //=== Transitions ===//

    The `transition` style is a special style that allows you to 
        define transitions between element states.

    <transition>:  time_in_ms  delay_in_ms 

    All transitions inherit from default state

    TODO: 
        * Support or/and selectors for multiple elements/classes/ids 
*/ 

//*==========================*//
//*======= Style Sheet ======*//

//=== Constants ===//

$justify:   start; 
$black:     rgb(0 0 0);
$red:       rgb(255 0 0);
$blue:      rgb(0 0 255);
$green:     rgb(0 255 0);
$yellow:    rgb(255 255 0);
$white:     rgb(255 255 255);
$border:    1;
$shadow:    1;
$scol:      rgba(255 0 0 10);
$font:      "mc_regular"; 
$delay:     20;
$time:      200; 
$rel_border: 2;

//=== Wildcard ===//

* {
    color_content: $red;
    justify_content: $justify;
}

//=== Classes ===//

.c0 {
    color_content: $black;
    color_border: $yellow;
    color_shadow: $green;
    shadow: $shadow;
    font: $font;
    border: $border;
    justify_content: center;
}

.reload_btn {
    width: 200;
    height: 50;
    border: 0;
    border_right: $rel_border; 
    border_bottom: $rel_border;
    color_border: rgba(0 0 0 50);
    color_content: rgb(100 100 100);
    justify_content: center;
}

.reload_btn: focus {
    color_background: rgb(30 30 30);
    color_content: $red;
}

.btn {
    color_background: rgb(198 198 198);
    width: 200;
    height: 45;
    transition: { 
        color_background: $time $delay;
        color_content: 300 0;
        height: 100 0;
        margin_top: 150 0;
    }
}

.btn: hover {
    color_background: rgba(168 168 168);
    color_content: $red;
    height: 50;
}

.btn: focus {
    color_background: $green;
    height: 55;
    padding_bottom: 12;
}

//=== IDs ===//

#lbl {
    justify_content: end;
} 

//=== Container ===// 

container {
    justify_content: start;
    color_content: rgb(255 255 255);
} 

//=== Button ===//

button {
    justify_content: center;
}

//=== Label ===// 

label {
    color_content: $yellow;
}


