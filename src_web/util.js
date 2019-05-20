'use strict';

function impl_get_canvas_client_width() {
    return Module.canvas.clientWidth;
}

function impl_get_canvas_client_height() {
    return Module.canvas.clientHeight;
}

function impl_set_canvas_size(width, height) {
    var c = Module.canvas;

    if (c.width !== width) {
        c.width = width;
    }

    if (c.height !== height) {
        c.height = height;
    }
}