#include "gpio.h"
#include "logTask.h"

#define OUT 1
#define IN 0

#define HIGH 1
#define LOW 0

#define LED_RED 23
#define LED_GREEN 24
#define LED_BLUE 22
#define BUTTON 27

bool led_active = false;

color BLACK = {.R = false, .G = false, .B = false};
color BLUE = {.R = false, .G = false, .B = true};
color GREEN = {.R = false, .G = true, .B = false};
color CYAN = {.R = false, .G = true, .B = true};
color RED = {.R = true, .G = false, .B = false};
color MAGENTA = {.R = true, .G = false, .B = true};
color YELLOW = {.R = true, .G = true, .B = false};
color WHITE = {.R = true, .G = true, .B = true};

// BLACK,     R0    G0    B0
// BLUE,      R0    G0    B1
// GREEN,     R0    G1    B0
// CYAN,      R0    G1    B1
// RED,       R1    G0    B0
// MAGENTA,   R1    G0    B1
// YELLOW,    R1    G1    B0
// WHITE,     R1    G1    B1

// TODO: controllare se è possibile usare SetDir e Free solo all'inizio e alla fine
// dell'esecuzione e non ad ogni chiamata

void changeLedColor(status_color node_status_col){
    // Se attivi disallochiamo i canali RGB dei led
    if(led_active){
        deactivateLed();
    }
    // Poi si deve reiniziallizare ogni canale come OUTPUT
    vxbGpioSetDir(LED_RED, OUT);
    vxbGpioSetDir(LED_GREEN, OUT);
    vxbGpioSetDir(LED_BLUE, OUT);

    // Infine si può impostare il colore di stato 
    led_active = true;

    // OFF_COL -> BLACK,
    // INIT_COL -> CYAN,
    // NOT_RESERVED_COL -> BLUE,
    // MESSAGE_EXCHANGE_COL -> YELLOW,
    // POSITIONING_COL -> WHITE,
    // RESERVED_COL -> GREEN,
    // TRAIN_IN_TRANSITION_COL -> MAGENTA,
    // FAIL_COL -> RED
    switch(node_status_col){
    case OFF_COL:
        setLedColor(BLACK);
        break;
    case INIT_COL:
        setLedColor(CYAN);
        break;
    case NOT_RESERVED_COL:
        setLedColor(BLUE);
        break;
    case MESSAGE_EXCHANGE_COL:
        setLedColor(YELLOW);
        break;
    case POSITIONING_COL:
        setLedColor(WHITE);
        break;
    case RESERVED_COL:
        setLedColor(GREEN);
        break;
    case TRAIN_IN_TRANSITION_COL:
        setLedColor(MAGENTA);
        break;
    case FAIL_COL:
        setLedColor(RED);
        break;
    default:
        logMessage("Errore nel settare colore led", taskName(0), 1);
        break;
    }
    
}

void setLedColor(color new_color){
    vxbGpioSetValue(LED_RED, new_color.R);
    vxbGpioSetValue(LED_GREEN, new_color.G);
    vxbGpioSetValue(LED_BLUE, new_color.B);
    node_color.R = new_color.R;
    node_color.G = new_color.G;
    node_color.B = new_color.B;
    char msg[50];
    snprintf(msg, 50, "Nuovo colore led RGB: %d %d %d", node_color.R, node_color.G, node_color.B);
    logMessage(msg, taskName(0), 0);
}

bool readButton(){
    vxbGpioSetDir(BUTTON, IN);
    return vxbGpioGetValue(BUTTON);
}

void deactivateLed(){
    // Ogni volta che utilizziamo un led che è stato precedentemente accesso
    // si deve procedere a settare tutti i canali LOW e disallocarli con Free   
    vxbGpioSetValue(LED_RED, LOW);
    vxbGpioFree(LED_RED);
    vxbGpioSetValue(LED_GREEN, LOW);
    vxbGpioFree(LED_GREEN);
    vxbGpioSetValue(LED_BLUE, LOW);
    vxbGpioFree(LED_BLUE);
    
}