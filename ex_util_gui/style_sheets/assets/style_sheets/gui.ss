// gui.ss

/*
    // All styles follow the following format (very similar to css)
    <element>: [state] { <styles> } 

    <styles>: style: value | transition: {<transitions}

    // All states for an element are optional and inherit from the 
        default state then override with their own styles

    // List of all available elements: 
        * container
        * label
        * text
        * panel
        * input
        * button
        * scroll
        * image

    // List of all available styles and their expected inputs: 
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

    // The `transition` style is a special style that allows you to 
        define transitions between element states.

    <transition>:  time_in_ms  delay_in_ms 

    // All transitions inherit from default state

*/

button {
    color_background: rgb(198 198 198)
    color_content: rgb(255 0 0)
    color_border: rgb(0 255 0)
    color_shadow: rgb(0 255 0)
    width: 200
    height: 45
    margin: 0
    margin_top: 10
    margin_right: 20
    border: 2
    shadow: 1
    justify_content: center
    align_content: center
    font: mc_regular
    transition: { 
        color_background: 200 20
        color_content: 200 0
        height: 100 0
        margin_top: 150 0
    }
}

button : hover {
    color_background: rgba(168 168 168 255)
    height: 47
}

button : focus {
    color_content: rgb(255 255 255)
    color_background: rgb(49 174 31)
    height: 50
    padding_bottom: 12
} 

label { 
    color_background: rgba(0 0 255 255)
    color_content: rgb(255 0 0)
    width: 150
    padding: 10
    font: mc_regular
    transition: {
        width: 100 0
        height: 100 0
    }
}

label : hover { 
    color_background: rgba(255 0 255 255)
}

label : focus {
    width: 150
    height: 100
}

