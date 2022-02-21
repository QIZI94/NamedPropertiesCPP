#include <iostream>
#include <utility>

#include "../property.hpp"

#define PROPERTIES(...) \
void propertiesFunc(const unip::Property::Visitor& visitor){\
using namespace unip;\
Property::Visitor::visit(visitor,{__VA_ARGS__});}

using Range = std::pair<long long, long long>;
class SimpleClass{
    public:
    PROPERTIES(
        Property("Primitive types"),
        Property("a", a),
        Property("b", b),
        Property("c", c),
        Property("d", d),
        Property("Complex types"),
        Property("Range", range),
        Property("Limited Range", 
            // read function
            [this](Property::any_type& output){
                output = Property::interface::make_any<const Range&>(limitedRange);
            },
            // write function
            [this](const Property::any_type& input){
                const Range& newRange = Property::interface::cast_any<const Range&>(input);
                setLimitedRange(newRange.first, newRange.second);
            }
        ),
        Property("Class Name", className),
    )

    void setLimitedRange(long long start, long long end){
        if(start < -1500) start = -1500;
        if(end  > 1500) end = 1500;
        limitedRange.first = start;
        limitedRange.second = end;
    }

    private:
    char a = 'a';
    short b = 0x1234;
    int c = -1;
    float d = 3.14;
    Range range {-10, +10};
    Range limitedRange {-10, +10};
    std::string className = "SimpleClass";
    
};



int main(){
    
    SimpleClass simpleClass;

    unip::Property::Visitor readingVisitor(
        [](const unip::Property& property){
        using any_t = unip::Property::any_type;
        using impl = unip::Property::interface;
        if(property.isNameOnly()){
            std::cout<<"[Category] "<<property.name()<<":\n";
            return true;
        }
        if(property.isWriteOnly()){
            return true;
        }
        any_t value;
        
        // read into value
        property.read(value);

        unip::Property::string_type propName = property.name();

        if(value.type() == typeid(char)){
            std::cout<<"\tValue["<<propName<<"]: "<<impl::cast_any<char>(value)<<'\n';
        }
        else if(value.type() == typeid(short)){
            std::cout<<"\tValue["<<propName<<"]: "<<std::hex<<"0x"<<impl::cast_any<short>(value)<<'\n';
        }
        else if(value.type() == typeid(int)){
            std::cout<<"\tValue["<<propName<<"]: "<<std::dec <<impl::cast_any<int>(value)<<'\n';
        }
        else if(value.type() == typeid(float)){
            std::cout<<"\tValue["<<propName<<"]: "<<impl::cast_any<float>(value)<<'\n';
        }
        else if(value.type() == typeid(Range)){
            const Range& range = impl::cast_any<const Range&>(value);
            std::cout<<"\tValue["<<propName<<"]: {"<< range.first<<", "<<range.second<<"}\n";
        }
        else if(value.type() == typeid(std::string)){
            const std::string& className = impl::cast_any<const std::string&>(value);
            std::cout<<"\tValue["<<propName<<"]: "<<className<<'\n';
        }

        return true;
        
    });

    simpleClass.propertiesFunc(readingVisitor);
    std::cout<<"\n<------------------------------------->\n\n";

    unip::Property::Visitor writingVisitor(
        [](const unip::Property& property){
        using any_t = unip::Property::any_type;
        using impl = unip::Property::interface;
        if(property.isNameOnly()){
            return true;
        }
        if(property.isReadOnly()){
           return true; 
        }
        
        any_t value;
        // read into value
        if(!property.isWriteOnly()){
            property.read(value);
        }
        
        auto range = Range(-20000,30000);

        if(value.type() == typeid(char)){
            value = 'A';
        }
        else if(value.type() == typeid(short)){
            value = (short)0x4321;
        }
        else if(value.type() == typeid(int)){
            value = (int)+1;
        }
        else if(value.type() == typeid(float)){
            value = (float)3.14/2;
        }
        else if(value.type() == typeid(Range)){
            value = (const Range&)range;
        }
        else if(value.type() == typeid(std::string)){
            const std::string& className = impl::cast_any<const std::string&>(value);
            value = std::string("Changed ")+className;            
        }
        std::cout<<"Writting new value to: [" <<property.name()<<"]\n";
        
            property.write(value);
        }

        return true;
        
    });

    simpleClass.propertiesFunc(writingVisitor);
    std::cout<<"\n<------------------------------------->\n\n";


    simpleClass.propertiesFunc(readingVisitor);


}