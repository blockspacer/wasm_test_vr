'use strict';

function ok(value) {
    return null != value;
}

function get_fbs_pose(builder, pose) {
    var fbs_pose;
    if (ok(pose)) {
        var fbs_position;
        if (ok(pose.position)) {
            fbs_position = VR.Pose.createPositionVector(builder, pose.position);
        }

        var fbs_linearVelocity;
        if (ok(pose.linearVelocity)) {
            fbs_linearVelocity = VR.Pose.createLinearVelocityVector(builder, pose.linearVelocity);
        }

        var fbs_linearAcceleration;
        if (ok(pose.linearAcceleration)) {
            fbs_linearAcceleration = VR.Pose.createLinearAccelerationVector(builder, pose.linearAcceleration);
        }

        var fbs_orientation;
        if (ok(pose.orientation)) {
            fbs_orientation = VR.Pose.createOrientationVector(builder, pose.orientation);
        }

        var fbs_angularVelocity;
        if (ok(pose.angularVelocity)) {
            fbs_angularVelocity = VR.Pose.createAngularVeclocityVector(builder, pose.angularVelocity);
        }

        var fbs_angularAcceleration;
        if (ok(pose.angularAcceleration)) {
            fbs_angularAcceleration = VR.Pose.createAngularAccelerationVector(builder, pose.angularAcceleration);
        }

        VR.Pose.startPose(builder);
        if (ok(fbs_position)) {
            VR.Pose.addPosition(builder, fbs_position);
        }
        if (ok(fbs_linearVelocity)) {
            VR.Pose.addLinearVelocity(builder, fbs_linearVelocity);
        }
        if (ok(fbs_linearAcceleration)) {
            VR.Pose.addLinearAcceleration(builder, fbs_linearAcceleration);
        }
        if (ok(fbs_orientation)) {
            VR.Pose.addOrientation(builder, fbs_orientation);
        }
        if (ok(fbs_angularVelocity)) {
            VR.Pose.addAngularVeclocity(builder, fbs_angularVelocity);
        }
        if (ok(fbs_angularAcceleration)) {
            VR.Pose.addAngularAcceleration(builder, fbs_angularAcceleration);
        }
        fbs_pose = VR.Pose.endPose(builder);
    }
    return fbs_pose;
}

function impl_get_vr_state(vr_state, vr_display_handle) {
    var builder = new flatbuffers.Builder(0);

    var frame_data;
    try {
        var vr_display = WebVR.dereferenceDisplayHandle(vr_display_handle); // Defined by Emscripten.
        frame_data = new VRFrameData();
        vr_display.getFrameData(frame_data);
    } catch (err) {}

    var gamepads = navigator.getGamepads();

    // Begin serializing data.

    // https://developer.mozilla.org/en-US/docs/Web/API/Performance/now
    // Because some platforms don't actually fill in VRFrameData.timestamp we set it ourselves.
    var fbs_timestamp = window.performance.now();

    var fbs_hmd;
    if (ok(frame_data)) {
        var fbs_leftProjectionMatrix;
        if (ok(frame_data.leftProjectionMatrix)) {
            fbs_leftProjectionMatrix = VR.HMD.createLeftProjectionMatrixVector(builder, frame_data.leftProjectionMatrix);
        }

        var fbs_leftViewMatrix;
        if (ok(frame_data.leftViewMatrix)) {
            fbs_leftViewMatrix = VR.HMD.createLeftViewMatrixVector(builder, frame_data.leftViewMatrix);
        }

        var fbs_rightProjectionMatrix;
        if (ok(frame_data.rightProjectionMatrix)) {
            fbs_rightProjectionMatrix = VR.HMD.createRightProjectionMatrixVector(builder, frame_data.rightProjectionMatrix);
        }

        var fbs_rightViewMatrix;
        if (ok(frame_data.rightViewMatrix)) {
            fbs_rightViewMatrix = VR.HMD.createRightViewMatrixVector(builder, frame_data.rightViewMatrix);
        }

        var fbs_pose = get_fbs_pose(builder, frame_data.pose);

        VR.HMD.startHMD(builder);
        if (ok(fbs_leftProjectionMatrix)) {
            VR.HMD.addLeftProjectionMatrix(builder, fbs_leftProjectionMatrix);
        }
        if (ok(fbs_leftViewMatrix)) {
            VR.HMD.addLeftViewMatrix(builder, fbs_leftViewMatrix);
        }
        if (ok(fbs_rightProjectionMatrix)) {
            VR.HMD.addRightProjectionMatrix(builder, fbs_rightProjectionMatrix);
        }
        if (ok(fbs_rightViewMatrix)) {
            VR.HMD.addRightViewMatrix(builder, fbs_rightViewMatrix);
        }
        if (ok(fbs_pose)) {
            VR.HMD.addPose(builder, fbs_pose);
        }
        fbs_hmd = VR.HMD.endHMD(builder);
    }

    var fbs_gamepads;
    if (ok(gamepads)) {
        fbs_gamepads = [];
        for (var i = 0; i < gamepads.length; ++i) {
            var gamepad = gamepads[i];
            if (ok(gamepad)) {
                var fbs_id;
                if (ok(gamepad.id)) {
                    fbs_id = builder.createString(gamepad.id);
                }

                var fbs_index = gamepad.index;

                var fbs_connected = gamepad.connected;

                var fbs_mapping;
                if (ok(gamepad.mapping)) {
                    fbs_mapping = builder.createString(gamepad.mapping);
                }

                var fbs_axes;
                if (ok(gamepad.axes)) {
                    fbs_axes = VR.Gamepad.createAxesVector(builder, gamepad.axes);
                }

                var fbs_buttons;
                if (ok(gamepad.buttons)) {
                    var buttons = gamepad.buttons;
                    fbs_buttons = [];
                    for (var j = 0; j < buttons.length; ++j) {
                        var button = buttons[j];

                        var fbs_pressed = button.pressed;

                        var fbs_touched = button.touched;

                        var fbs_value = button.value;

                        VR.GamepadButton.startGamepadButton(builder);
                        if (ok(fbs_pressed)) {
                            VR.GamepadButton.addPressed(builder, fbs_pressed);
                        }
                        if (ok(fbs_touched)) {
                            VR.GamepadButton.addTouched(builder, fbs_touched);
                        }
                        if (ok(fbs_value)) {
                            VR.GamepadButton.addValue(builder, fbs_value);
                        }
                        fbs_buttons.push(VR.GamepadButton.endGamepadButton(builder));
                    }
                    fbs_buttons = VR.Gamepad.createButtonsVector(builder, fbs_buttons);
                }

                var fbs_pose = get_fbs_pose(builder, gamepad.pose);

                VR.Gamepad.startGamepad(builder);
                if (ok(fbs_id)) {
                    VR.Gamepad.addId(builder, fbs_id);
                }
                if (ok(fbs_index)) {
                    VR.Gamepad.addIndex(builder, fbs_index);
                }
                if (ok(fbs_connected)) {
                    VR.Gamepad.addConnected(builder, fbs_connected);
                }
                if (ok(fbs_mapping)) {
                    VR.Gamepad.addMapping(builder, fbs_mapping);
                }
                if (ok(fbs_axes)) {
                    VR.Gamepad.addAxes(builder, fbs_axes);
                }
                if (ok(fbs_buttons)) {
                    VR.Gamepad.addButtons(builder, fbs_buttons);
                }
                if (ok(fbs_pose)) {
                    VR.Gamepad.addPose(builder, fbs_pose);
                }
                fbs_gamepads.push(VR.Gamepad.endGamepad(builder));
            }
        }
        fbs_gamepads = VR.State.createGamepadsVector(builder, fbs_gamepads)
    }

    VR.State.startState(builder);
    if (ok(fbs_timestamp)) {
        VR.State.addTimestamp(builder, fbs_timestamp);
    }
    if (ok(fbs_hmd)) {
        VR.State.addHmd(builder, fbs_hmd);
    }
    if (ok(fbs_gamepads)) {
        VR.State.addGamepads(builder, fbs_gamepads);
    }
    builder.finish(VR.State.endState(builder));

    // Copy array to a C/C++-readable buffer.
    var array = builder.asUint8Array();
    var uchar_array = _malloc(array.length);
    Module.HEAPU8.set(array, uchar_array);

    setValue(vr_state, uchar_array, 'uint8_t**'); // Set the char double-pointer to point at the array of chars.
    return array.length;
}