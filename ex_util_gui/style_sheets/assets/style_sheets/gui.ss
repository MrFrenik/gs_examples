// gui.ss

/*
    //=== Description ===//
    
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
        * GSS Variables
        * Support wildcard selector
        * Support or/and selectors for multiple elements/classes/ids 

*/ 

//=== Button ===// 

button {
    justify_content: center;
    align_content: center;
    font: "mc_regular";
} 

.button {
    color_background: rgb(198 198 198);
    color_content: rgb(255 0 0);
    color_border: rgb(255 255 0);
    color_shadow: rgb(0 255 0);
    border: 2
    width: 200;
    height: 45;
    transition: { 
        color_background: 200 20;
        color_content: 200 0;
        height: 100 0;
        margin_top: 150 0;
    }
}

.button: hover {
    color_background: rgba(168 168 168 255);
    height: 20;
}

.button: focus {
    color_content: rgb(255 255 255);
    color_background: rgb(49 174 31);
    height: 50;
    padding_bottom: 12;
} 

.red {
    color_background: rgb(255 0 0);
}

.blue {
    color_shadow: rgb(0 0 255);
}

.green { 
    color_border: rgb(0 255 0);
    transition: {
        width: 150 0 
    }
}

#btn { 
    color_content: rgb(0 0 0);
} 

#btn: hover {
    color_content: rgb(120 120 120);
    width: 250
}

#btn: focus {
    color_content: rgb(255 255 255);
}

//=== Label ===// 

label: hover {
    color_content: rgb(255 120 10);
} 

#lbl { 
    color_background: rgb(0 255 0);
    color_content: rgb(0 0 255);
    width: 150;
    padding: 10;
    font: "mc_regular";
    transition: {
        color_background: 150 0;
        width: 100 0;
        height: 100 0;
    }
}

#lbl: hover { 
    color_background: rgb(255 0 0);
    color_background: rgba(255 0 255 255);
    width: 150;
    height: 100;
} 

