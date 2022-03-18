#include <iostream>
#include <string>
#include <utility>


#include "../property.hpp"

#define PROPERTIES(...) \
void propertiesFunc(const unip::Property::Visitor& visitor){\
using namespace unip;\
Property::Visitor::visit(visitor,{__VA_ARGS__});}\
void propertiesFuncConst(const unip::Property::Visitor& visitor) const{\
using namespace unip;\
Property::Visitor::visit(visitor,{__VA_ARGS__});}



void PropertyToString(const unip::Property& property, std::string& output){
    using namespace unip;
    Property::string_type propName = property.name();
    output.append(propName);

    if(property.isNameOnly()){

        output.append(": none");
        return;
    }

    Property::any_type value;
    property.read(value);

    if(Property::is_any<int>(value)){
        output.append(": ");
        output.append(std::to_string(Property::cast_any<int>(value)));
    }
    else if(Property::is_any<float>(value)){
        output.append(": ");
        output.append(std::to_string(Property::cast_any<float>(value)));
    }
    else if(Property::is_any<std::string_view>(value)){
        output.append(": ");
        output.append(Property::cast_any<const std::string_view&>(value));
    }

    output.append(", ");
}

bool SerializeFromString(const unip::Property& property, std::string_view& input){
    using namespace unip;
    auto itCurrent = input.begin();
    auto itEnd = input.end();


    //while(itCurrent != itEnd) {
    if(itCurrent != itEnd){
        auto itSpace = std::find(itCurrent, itEnd, ':');

        if(itSpace == itEnd){
            std::cerr<<"Wrong formating "<<input << '\n';
        }

        std::string_view propName(&*itCurrent, std::distance(itCurrent,itSpace));
        
        if(propName != property.name()){
            std::cerr<<"Wrong property name: "<<propName << " != " << property.name()<<'\n';
            
            return false;
        }
        itCurrent = itSpace + 2;
        auto itComa = std::find(itCurrent, itEnd, ',');

        std::string s_value(itCurrent, itComa);

        unip::Property::any_type value;
        property.read(value);

        if(Property::is_any<int>(value)){
            
            int i = std::stoi(s_value);
            value = i;
            property.write(value);
        }
        else if(Property::is_any<float>(value)){
            float f = std::stof(s_value);
            value = f;
            property.write(value);
        }
       else if(Property::is_any<std::string_view>(value)){
            auto s_propVal = Property::cast_any<const std::string_view&>(value);
            
            if(s_propVal != s_value)
            {
                std::cerr<<"Wrong constant value: "<<s_propVal << " != " << s_value<<'\n';
                return false;
            }
        }
        
        itCurrent = std::find_if(itComa, itEnd, [](char ch){return (ch != ' ' &&  ch != ',');});
        input = std::string_view(itCurrent, std::distance(itCurrent, itEnd));

        return true;
    }

    return false;
}

class Point{
    public:
    PROPERTIES(
        Property("ClassName", std::string_view("Point")),
        Property("x", x),
        Property("y", y),
    )
    Point(){}
    Point(int x, int y) : x(x), y(y){}

    void serialize(std::string& fileBuf, bool reading){
        if(reading){
            std::string_view entries = fileBuf;
            unip::Property::Visitor serializationVisitor(
                [&entries](const unip::Property& property){
                    return SerializeFromString(property, entries);
                }
            );
            propertiesFunc(serializationVisitor);
        }
        else{
            fileBuf = toString();
        }

    }

    std::string toString() const{
        std::string ret;
        unip::Property::Visitor toStringVisitor(
            [&ret](const unip::Property& property){
                PropertyToString(property, ret);
                return true;
            }
        );
        propertiesFuncConst(toStringVisitor);

        return ret;
    }


    private:
    int x=0,y=0;
    
};

class Pointf{
    public:
    PROPERTIES(
        Property("ClassName", std::string_view("Pointf")),
        Property("x", x),
        Property("y", y),
    )
    Pointf(){}
    Pointf(float x, float y) : x(x), y(y) {}

    void serialize(std::string& fileBuf, bool reading){
        std::string_view entries = fileBuf;
        unip::Property::Visitor serializationVisitor(
            [&entries](const unip::Property& property){
                return SerializeFromString(property, entries);
            }
        );
        propertiesFunc(serializationVisitor);
    }

    std::string toString() const{
        std::string ret;
        unip::Property::Visitor toStringVisitor(
            [&ret](const unip::Property& property){
                PropertyToString(property, ret);
                return true;
            }
        );
        propertiesFuncConst(toStringVisitor);

        return ret;
    }


    private:
    float x=0,y=0;
    
};



int main(){
    Point p;
    Pointf pf;
    std::cout<<p.toString()<<"\n";
    std::cout<<pf.toString()<<"\n\n";

    std::string s_pf = Pointf(5,4).toString();
    pf.serialize(s_pf, true);
    std::cout<<pf.toString()<<"\n";

}