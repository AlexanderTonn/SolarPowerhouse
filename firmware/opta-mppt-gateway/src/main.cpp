#include "OptaMpptGateway.hpp"

void setup() {
    solar::opta::gateway().setup();
}

void loop() {
    solar::opta::gateway().loop();
}
