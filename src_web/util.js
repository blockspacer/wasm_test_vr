'use strict';

function impl_get_timestamp() {
    // We could lie to ourselves about the time and just use C/C++ techniques
    // but this is more honest about provided precision.
    // https://developer.mozilla.org/en-US/docs/Web/API/Performance/now
    return window.performance.now();
}

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