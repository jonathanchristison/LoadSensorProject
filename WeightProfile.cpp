#include "WeightProfile.hpp"

WeightProfile::WeightProfile()
{
    name_ = 'C';
    weightRangeMin_ = 0;
    weightRangeMax_ = 0;
    cVal_ = Colour(255, 255, 255);
}

WeightProfile::WeightProfile(int min, int max, char name)
{
    weightRangeMin_ = min;
    weightRangeMax_ = max;
    name_ = name;
}

WeightProfile::WeightProfile(int min, int max, char name, Colour colour)
{
    weightRangeMin_ = min;
    weightRangeMax_ = max;
    name_ = name;
    cVal_ = colour;
}

WeightProfile::WeightProfile(int min, int max, Colour colour)
{
    weightRangeMin_ = min;
    weightRangeMax_ = max;
    name_ = 'C';
    cVal_ = colour;
}

void WeightProfile::maximal(int max)
{
    weightRangeMax_ = max;
}

void WeightProfile::minimum(int min)
{
    weightRangeMin_ = min;
}

void WeightProfile::name(char name)
{
    name_ = name;
}

int WeightProfile::maximal()
{
    return weightRangeMax_;
}

int WeightProfile::minimum()
{
    return weightRangeMin_;
}

char WeightProfile::name()
{
    return name_;
}

unsigned int WeightProfile::deviation(int dev)
{
    int mean = (this->maximal() + this->minimum()) / 2;
    return mean > dev ? mean - dev : dev - mean;
}

void WeightProfile::cVal(Colour colour)
{
    cVal_ = colour;
}

Colour WeightProfile::cVal()
{
    return cVal_;
}

bool WeightProfile::match(int val)
{
    return val > this->minimum() && val < this->maximal();
}

void WeightProfile::save(byte offset)
{
    EEPROM.write(0, offset);
    offset = offset * sizeof(WeightProfile);
    Serial.print("Saving WP to: ");
    Serial.println(offset, DEC);
    int writtenblocks = EEPROM.writeBlock(offset, *this);
    delay(100);
    Serial.print("Blocks written: ");
    Serial.println(writtenblocks, DEC);
}

WeightProfile WeightProfile::load(byte offset)
{
    WeightProfile wp;
    offset = offset * sizeof(WeightProfile);
    int rblocks = EEPROM.readBlock(offset, wp);
    #ifdef DEBUG
    Serial.print("Reading offset: ");
    Serial.println(offset, DEC);
    Serial.print("Read blocks: ");
    Serial.println(rblocks, DEC);
    Serial.print("Min: ");
    Serial.println(read.weightRangeMin, DEC);
    Serial.print("Max: ");
    Serial.println(read.weightRangeMax, DEC);
    Serial.print("Colour: ");
    Serial.println(read.c.hex());
    #endif
    return wp;
}

size_t WeightProfile::printTo(Print& p) const
{
    size_t n = 0;
    n += p.print("---Weight Profile--\n");
    n += p.print("Min:\t");
    n += p.print(weightRangeMin_, DEC);
    n += p.print("\nMax:\t");
    n += p.print(weightRangeMax_, DEC);
    n += p.print("\nName:\t");
    n += p.print(name_);
    n += p.print("\nColour:\t\n");
    n += p.print(cVal_);
    return n;
}

bool WeightProfile::operator < (const WeightProfile& that) const
{
    return weightRangeMin_ < that.weightRangeMin_
           && weightRangeMax_ < that.weightRangeMax_;
}

bool WeightProfile::operator <= (const WeightProfile& that) const
{
    return weightRangeMin_ <= that.weightRangeMin_
           && weightRangeMax_ <= that.weightRangeMax_;
}

bool WeightProfile::operator > (const WeightProfile& that) const
{
    return weightRangeMin_ > that.weightRangeMin_
           && weightRangeMax_ > that.weightRangeMax_;
}

bool WeightProfile::operator >= (const WeightProfile& that) const
{
    return weightRangeMin_ >= that.weightRangeMin_
           && weightRangeMax_ >= that.weightRangeMax_;
}
bool WeightProfile::operator == (const WeightProfile& that) const
{
    return weightRangeMin_  == that.weightRangeMin_
           && weightRangeMax_ == that.weightRangeMax_;
}
