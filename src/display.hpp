

#include "main.hpp"


const int menuitems = 5;
extern char DISPLAY_TEXT[menuitems][17];
extern double DISPLAY_VARIABLES [menuitems];
extern int DISPLAY_INDEX [menuitems];

extern void updateDisplay( void );
extern void updateDisplayVariables( void );

extern void checkAutoUpSwitch( void );
extern void checkAutoDownSwitch( void );
extern void checkManualUpSwitch( void );
extern void checkManualDownSwitch( void );
