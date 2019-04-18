
struct LButton : SVGSwitch, MomentarySwitch {
    LButton() {
        addFrame(SVG::load(assetPlugin(plugin, "res/L.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/Ldown.svg")));
        sw->wrap();
        box.size = sw->box.size;
    }

    void onMouseDown(EventMouseDown &e) override {
       //if ( rack::windowIsShiftPressed() ) { 
           //printf("shift pressed\n");
        //}
#ifdef DEBUG
       if (e.button == 1) {
            printf("Button pressed down\n");
       } else {
            printf("Button value %d\n", e.button);
       }
#endif
       SVGSwitch::onMouseDown(e);
    }

    void onHoverKey(EventHoverKey &e) override {
        printf("Hovering!\n");
        EventMouseDown emd;
        emd.button = 1;
        SVGSwitch::onMouseDown(emd);
        SVGSwitch::onHoverKey(e);
    }
    
    void onChange(EventChange &e) override {
      // Do your own thang.
      //printf("Button change detected!\n");
      // Pass event to super class.
      SVGSwitch::onChange(e);
    }
};

