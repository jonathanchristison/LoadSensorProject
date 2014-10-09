#include <Average.h>
#include <ArduinoDevices.h>
#include <EEPROMex.h>
#include "WeightProfile.hpp"
/** Lazy globals */
/** Serial Event String (Interpret cmd strings from serial)
    @todo use serial command lib
**/
String inputString = "";
bool stringComplete = false;
bool blockrun = false;

/** Time of running
    @todo use "Time" rather than "Duration" and use a RTC
**/
const Timing::Duration StartTime;

/** LED and Strain devices **/
AnalogInputDevice::Strain sg(A0);
AnalogOutputDevice::LED bed(9, 10, 11);


void setup()
{
    analogReference(EXTERNAL);
    Serial.begin(9600);
    EEPROM.setMaxAllowedWrites(512);
    EEPROM.setMemPool(1, 512);
    inputString.reserve(30);
    Serial.println(F("Starting up..."));
    Serial.println(F("Usage:"));
    Serial.println(F("\tRESET\n\tCalibrate\n\tEDIT\n\tRUN\n\tShow\n\tREALTIME"));

    StartTime.from_millisecs(millis());
}


static int qcmp(const void* a, const void* b)
{
    if(*(WeightProfile*)a < * (WeightProfile*)b)
    {
        return -1;
    }
    if(*(WeightProfile*)a == *(WeightProfile*)b)
    {
        return 0;
    }
    if(*(WeightProfile*)a > *(WeightProfile*)b)
    {
        return 1;
    }
}

void serialEvent()
{
    while(Serial.available())
    {
        char inChar = (char)Serial.read();
        inputString += inChar;
        if(inChar == '\n')
        {
            stringComplete = true;
        }
    }
}

bool serialVerify()
{
    Serial.println(F("Are you sure? yes/no"));
    while(Serial.available() < 3)
    {
        delay(100);
    }
    String answ;
    answ.reserve(5);
    answ = Serial.readString();
    answ.trim();
    return answ == "yes" ? true : false;
}

void Calibrate()
{
    Serial.println(F("Calibration mode...."));
    Colour chosenColour(WHITE);
    bed.colour(chosenColour);
    bed.fadeTime(Timing::Duration::from_millisecs(5));
    bed.pulse(10);

    WeightProfile cal;
    Serial.println(F("Enter RGB Values:"));
    while(Serial.available() < 8)
    {
        delay(100);
    }

    chosenColour.hex(Serial.readString());
    Serial.println(chosenColour);
    cal.cVal(chosenColour);
    Serial.println(cal);
    bed.colour(chosenColour);
    bed.pulse(5);

    sg.resolution(300);
    if(EEPROM.read(0) < 1)
    {
        //Get a baseline reference
        Serial.println(F("Getting a no load reference"));
        bed.colour(Colour(0, 255, 0));
        bed.pulse(2);
        WeightProfile empty(0, sg.averageValue(), 'E');
        Serial.println(F("No load value:"));
        Serial.println(empty);
        empty.save(1);
        bed.pulse(2);
    }
    delay(Timing::Duration::from_secs(10).to_millisecs());
    Serial.println(F("Stand on the load sensor for 5 seconds"));
    Average<uint16_t> average(50);

    Timing::Duration standtime;
    standtime = Timing::Duration::from_millisecs(millis());
    do
    {
        for(int i = 0; i < 10; i++)
        {
            average.push(sg.averageValue());
        }

        delay(Timing::Duration::from_secs(1).to_millisecs());
    }
    while(Timing::Duration::from_millisecs(millis()) <
            (standtime + Timing::Duration::from_secs(5)));

    cal.maximal(average.maximum());
    cal.minimum(average.minimum());
    Serial.println(F("Saving:"));
    Serial.println(cal);
    cal.save(EEPROM.read(0) + 1);
    bed.colour(cal.cVal());
    bed.pulse(10);
}

void Reset()
{
    Serial.println(F("Wiping EEPROM"));
    if(serialVerify())
    {
        Serial.println("Erasing...");
        unsigned long percent = 0;
        for(int i = 0; i < 512; i++)
        {
            EEPROM.write(i, 0);
            if(percent != (i * 100UL / 512UL))
            {
                percent = (i * 100UL / 512UL);
                Serial.print(percent, DEC);
                Serial.print(F("%..."));
            }
        }
        Serial.println(F("Done"));
    }
    else
    {
        Serial.println(F("Operation canceled"));
    }
}

void Show()
{
    Serial.println(F("Listing EEPROM"));
    int wpscnt = (int)EEPROM.read(0);
    Serial.print(F("Number of entries:"));
    Serial.println(wpscnt, DEC);
    for(int i = 0; i < wpscnt; i++)
    {
        WeightProfile wp;
        wp = wp.load(i + 1);
        Serial.print(F("\n--------\t"));
        Serial.print(i, DEC);
        Serial.println(F("\t--------\n"));
        Serial.print(wp);
        Serial.print(F("\n---------------"));
    }
}

void RealTime()
{
    Serial.println(F("Real Time"));
    bed.colour(Colour(WHITE));
    bed.pulse(3);
    sg.resolution(10);
    sg.delayBetweenReads(Timing::Duration::zero());

    const Timing::Duration st = Timing::Duration::from_millisecs(millis());
    const Timing::Duration duration(60, 0);
    Colour energyc;
    int val = sg.averageValue();
    unsigned short i = 0;
    do
    {
        energyc = energyc.energy(val);
        bed.colour(energyc);
        bed.instant();
        delay(1);
        if(i > 50)
        {
            Serial.println(val, DEC);
            Serial.println(bed.colour());
            Serial.flush();
            i = 0;
        }
        val = sg.averageValue();
        i++;
    }
    while(st +  duration > Timing::Duration::from_millisecs(millis()));
}

void Run()
{
    WeightProfile wps[20];
    int wpscnt = EEPROM.read(0);

    bed.colour(Colour(WHITE));
    bed.pulse(5);
    Serial.println(F("Loading from EEPROM\n-----------------\n"));
    for(int i = 0; i < wpscnt; i++)
    {
        wps[i] = wps[i].load(i + 1);
        Serial.println(wps[i]);
        Serial.flush();
    }
    //Re arrage list lowest to highest
    Serial.println(F("Sorting\n-----------------\n"));
    qsort(wps, wpscnt, sizeof(WeightProfile), qcmp);
    for(int i = 0; i < wpscnt; i++)
    {
        bed.colour(wps[i].cVal());
        bed.instant();
        delay(1000);
        bed.colour(Colour(OFF));
        bed.instant();
        Serial.println(wps[i]);
    }

    while(true)
    {
        sg.resolution(200);
        sg.delayBetweenReads(Timing::Duration::from_microsecs(40));
        int val = sg.averageValue();
        WeightProfile* wp;
        Serial.println(val, DEC);
        for(int i = 0; i < wpscnt; i++)
        {
            if(wps[i].match(val) && i > 0)
            {
                sg.resolution(300);
                sg.delayBetweenReads(Timing::Duration::from_microsecs(300));
                delay(10);
                int sval = sg.averageValue();
                wp = (wps[i+1].deviation(sval) < wps[i].deviation(sval)) ? &wps[i+1] : &wps[i];

                Serial.println(F("Matches"));
                Serial.println(*wp);
                Serial.println(F("With Value:\t"));
                Serial.print(val, DEC);
                bed.fadeTime(Timing::Duration::from_millisecs(10));
                bed.colour(wp->cVal());
                bed.fadeIn();
                delay(Timing::Duration::from_secs(5).to_millisecs());
                bed.fadeOut();
            }
        }
    }
}

void Edit()
{
    Show();
    Serial.println(F("Select Which to Edit"));
    while(Serial.available() < 2)
    {
        delay(100);
    }

    int epv = Serial.parseInt();
    WeightProfile wp;
    wp = wp.load(epv);
    Serial.println(F("Edit fields: Min, Max"));

    while(Serial.available() < 3)
    {
        delay(100);
    }

    wp.minimum(Serial.parseInt());
    wp.maximal(Serial.parseInt());

    Serial.println(F("Edit field: RGB"));
    while(Serial.available() < 8)
    {
        delay(100);
    }

    Colour chosenColour;
    chosenColour.hex(Serial.readString());
    wp.cVal(chosenColour);
    bed.colour(chosenColour);
    bed.instant();
    Serial.println(F("Edit field: Name (char)"));
    while(Serial.available() < 1)
    {
        delay(100);
    }

    wp.name(Serial.read());

    Serial.println(F("About to write:"));
    Serial.println(wp);
    if(serialVerify())
    {
        byte cnt = EEPROM.read(0);
        wp.save(epv + 1);
        EEPROM.write(0, cnt);
    }
    else
    {
        Serial.println(F("Not saving changes"));
    }
    bed.colour(Colour(OFF));
    bed.instant();
}

void loop()
{
    if(stringComplete)
    {
        if(inputString == "Calibrate\r\n")
        {
            inputString = "";
            stringComplete = false;
            Calibrate();
            blockrun = true;
        }
        else if(inputString == "Show\r\n")
        {
            inputString = "";
            stringComplete = false;
            Show();
            blockrun = true;
        }
        else if(inputString == "RESET\r\n")
        {
            inputString = "";
            stringComplete = false;
            Reset();
            blockrun = true;
        }
        else if(inputString == "RUN\r\n")
        {
            inputString = "";
            stringComplete = false;
            Run();
            blockrun = true;
        }
        else if(inputString == "REALTIME\r\n")
        {
            inputString = "";
            stringComplete = false;
            RealTime();
            blockrun = true;
        }
        else if(inputString == "EDIT\r\n")
        {
            inputString = "";
            stringComplete = false;
            Edit();
            blockrun = true;
        }
        else
        {
            inputString = "";
            stringComplete = false;
            Serial.println(F("Not a valid command"));
        }
    }
    if((StartTime + Timing::Duration::from_millisecs(millis())) >= Timing::Duration::from_secs(30)
            && !blockrun)
    {
        Serial.println(F("Timed out, Run cycle"));
        Run();
    }
}

