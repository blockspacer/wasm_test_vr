'use strict';

function impl_get_gamepad_list(gamepad_list) {
    function ok(value) {
        return null != value;
    }

    var gamepads = navigator.getGamepads();

    var builder = new flatbuffers.Builder(0);

    var fbs_list;
    if (ok(gamepads)) {
        fbs_list = [];
        for (var i = 0; i < gamepads.length; ++i) {
            var gamepad = gamepads[i];
            if (ok(gamepad)) {
                var fbs_id;
                if (ok(gamepad.id)) {
                    fbs_id = builder.createString(gamepad.id);
                }

                var fbs_index = gamepad.index;

                var fbs_connected = gamepad.connected;

                var fbs_timestamp = gamepad.timestamp;

                var fbs_mapping;
                if (ok(gamepad.mapping)) {
                    fbs_mapping = builder.createString(gamepad.mapping);
                }

                var fbs_axes;
                if (ok(gamepad.axes)) {
                    fbs_axes = Gamepad.createAxesVector(builder, gamepad.axes);
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

                        GamepadButton.startGamepadButton(builder);
                        if (ok(fbs_pressed)) {
                            GamepadButton.addPressed(builder, fbs_pressed);
                        }
                        if (ok(fbs_touched)) {
                            GamepadButton.addTouched(builder, fbs_touched);
                        }
                        if (ok(fbs_value)) {
                            GamepadButton.addValue(builder, fbs_value);
                        }
                        fbs_buttons.push(GamepadButton.endGamepadButton(builder));
                    }
                    fbs_buttons = Gamepad.createButtonsVector(builder, fbs_buttons);
                }

                var fbs_pose;
                if (ok(gamepad.pose)) {
                    var pose = gamepad.pose;

                    var fbs_hasOrientation = pose.hasOrientation;

                    var fbs_hasPosition = pose.hasPosition;

                    var fbs_position;
                    if (ok(pose.position)) {
                        fbs_position = GamepadPose.createPositionVector(builder, pose.position);
                    }

                    var fbs_linearVelocity;
                    if (ok(pose.linearVelocity)) {
                        fbs_linearVelocity = GamepadPose.createLinearVelocityVector(builder, pose.linearVelocity);
                    }

                    var fbs_linearAcceleration;
                    if (ok(pose.linearAcceleration)) {
                        fbs_linearAcceleration = GamepadPose.createLinearAccelerationVector(builder, pose.linearAcceleration);
                    }

                    var fbs_orientation;
                    if (ok(pose.orientation)) {
                        fbs_orientation = GamepadPose.createOrientationVector(builder, pose.orientation);
                    }

                    var fbs_angularVelocity;
                    if (ok(pose.angularVelocity)) {
                        fbs_angularVelocity = GamepadPose.createAngularVeclocityVector(builder, pose.angularVelocity);
                    }

                    var fbs_angularAcceleration;
                    if (ok(pose.angularAcceleration)) {
                        fbs_angularAcceleration = GamepadPose.createAngularAccelerationVector(builder, pose.angularAcceleration);
                    }

                    GamepadPose.startGamepadPose(builder);
                    if (ok(fbs_hasOrientation)) {
                        GamepadPose.addHasOrientation(builder, fbs_hasOrientation);
                    }
                    if (ok(fbs_hasPosition)) {
                        GamepadPose.addHasPosition(builder, fbs_hasPosition);
                    }
                    if (ok(fbs_position)) {
                        GamepadPose.addPosition(builder, fbs_position);
                    }
                    if (ok(fbs_linearVelocity)) {
                        GamepadPose.addLinearVelocity(builder, fbs_linearVelocity);
                    }
                    if (ok(fbs_linearAcceleration)) {
                        GamepadPose.addLinearAcceleration(builder, fbs_linearAcceleration);
                    }
                    if (ok(fbs_orientation)) {
                        GamepadPose.addOrientation(builder, fbs_orientation);
                    }
                    if (ok(fbs_angularVelocity)) {
                        GamepadPose.addAngularVeclocity(builder, fbs_angularVelocity);
                    }
                    if (ok(fbs_angularAcceleration)) {
                        GamepadPose.addAngularAcceleration(builder, fbs_angularAcceleration);
                    }
                    fbs_pose = GamepadPose.endGamepadPose(builder);
                }

                Gamepad.startGamepad(builder);
                if (ok(fbs_id)) {
                    Gamepad.addId(builder, fbs_id);
                }
                if (ok(fbs_index)) {
                    Gamepad.addIndex(builder, fbs_index);
                }
                if (ok(fbs_connected)) {
                    Gamepad.addConnected(builder, fbs_connected);
                }
                if (ok(fbs_timestamp)) {
                    Gamepad.addTimestamp(builder, fbs_timestamp);
                }
                if (ok(fbs_mapping)) {
                    Gamepad.addMapping(builder, fbs_mapping);
                }
                if (ok(fbs_axes)) {
                    Gamepad.addAxes(builder, fbs_axes);
                }
                if (ok(fbs_buttons)) {
                    Gamepad.addButtons(builder, fbs_buttons);
                }
                if (ok(fbs_pose)) {
                    Gamepad.addPose(builder, fbs_pose);
                }
                fbs_list.push(Gamepad.endGamepad(builder));
            }
        }
        fbs_list = GamepadList.createListVector(builder, fbs_list)
    }

    GamepadList.startGamepadList(builder);
    if (ok(fbs_list)) {
        GamepadList.addList(builder, fbs_list);
    }
    builder.finish(GamepadList.endGamepadList(builder));

    // Copy array to a C/C++-readable buffer.
    var array = builder.asUint8Array();
    var uchar_array = _malloc(array.length);
    Module.HEAPU8.set(array, uchar_array);

    setValue(gamepad_list, uchar_array, 'uint8_t**'); // Set the char double-pointer to point at the array of chars.
    return array.length;
}