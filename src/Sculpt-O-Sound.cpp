#include "Sculpt-O-Sound.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
    pluginInstance = p;

    // Add all Models defined throughout the plugin
    p->addModel(modelVocode_O_Matic);    // Basic vocoder version
    p->addModel(modelVocode_O_Matic_XL); // Vocoder version allowing a lot more control by the user.
}
