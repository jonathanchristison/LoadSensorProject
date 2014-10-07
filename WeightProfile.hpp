#ifndef WEIGHTPROFILE_HPP
#define WEIGHTPROFILE_HPP
#include <Common.hpp>
#include <EEPROMex.h>
#include <Printable.h>

class WeightProfile : public Printable
{
public:
    WeightProfile();
    WeightProfile(int min, int max, char name);
    WeightProfile(int min, int max, char name, Colour colour);
    WeightProfile(int min, int max, Colour colour);
    void calibrate();
    void maximal(int max);
    void minimum(int min);
    void name(char name);
    int maximal();
    int minimum();
    char name();
    void cVal(Colour colour);
    Colour cVal();
    bool match(int val);
    void save(byte offset);
    WeightProfile load(byte offset);
    virtual size_t printTo(Print& p) const;
    bool operator < (const WeightProfile& that) const;
    bool operator <= (const WeightProfile& that) const;
    bool operator > (const WeightProfile& that) const;
    bool operator >= (const WeightProfile& that) const;
    bool operator == (const WeightProfile& that) const;
private:
    int weightRangeMin_;
    int weightRangeMax_;
    char name_;
    Colour cVal_;
};
#endif
