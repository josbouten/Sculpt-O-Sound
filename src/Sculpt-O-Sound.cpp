#include "Sculpt-O-Sound.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
    plugin = p;
    p->slug = TOSTRING(SLUG);
    p->version = TOSTRING(VERSION);
    p->website = "https://hithub.com/josbouten/Sculpt-O-Sound";
    p->manual = "https://hithub.com/josbouten/Sculpt-O-Sound/README.md";

    // Add all Models defined throughout the plugin
    p->addModel(modelVocode_O_Matic);    // simple version
    p->addModel(modelVocode_O_Matic_XL); // version allowing a lot more control by the user.
}
