#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include "gamepad_generated.h"

class Controllers {
public:
    Controllers();
    virtual ~Controllers();

    static bool get_controllers( Controllers* controllers );

    const GamepadList* gamepad_list() const;

private:
    uint8_t*           gamepad_list_array_;
    const GamepadList* gamepad_list_;
};

#endif // CONTROLLERS_H
