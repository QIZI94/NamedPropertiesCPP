#include <iostream>
#include <utility>

#include "../property.hpp"

#define PROPERTIES(...) \
void propertiesFunc(const unip::Property::Visitor& visitor){\
using namespace unip;\
Property::Visitor::visit(visitor,{__VA_ARGS__});}\
void propertiesFuncConst(const unip::Property::Visitor& visitor) const{\
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
                Property::ExecWhenNotConst([](const unip::Property::any_type& input, SimpleClass* This){
                        const Range& newRange = Property::interface::cast_any<const Range&>(input);
                        This->setLimitedRange(newRange.first, newRange.second);
                    },
                    input ,this
                );
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
        using namespace unip;
        using any_t = unip::Property::any_type;
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

        if(Property::is_any<char>(value)){
            std::cout<<"\tValue["<<propName<<"]: "<<unip::Property::cast_any<char>(value)<<'\n';
        }
        else if(Property::is_any<short>(value)){
            std::cout<<"\tValue["<<propName<<"]: "<<std::hex<<"0x"<<Property::cast_any<short>(value)<<'\n';
        }
        else if(Property::is_any<int>(value)){
            std::cout<<"\tValue["<<propName<<"]: "<<std::dec <<Property::cast_any<int>(value)<<'\n';
        }
        else if(Property::is_any<float>(value)){
            std::cout<<"\tValue["<<propName<<"]: "<<Property::cast_any<float>(value)<<'\n';
        }
        else if(Property::is_any<Range>(value)){
            const Range& range = Property::cast_any<const Range&>(value);
            std::cout<<"\tValue["<<propName<<"]: {"<< range.first<<", "<<range.second<<"}\n";
        }
        else if(Property::is_any<std::string>(value)){
            const std::string& className = Property::cast_any<const std::string&>(value);
            std::cout<<"\tValue["<<propName<<"]: "<<className<<'\n';
        }

        return true;
        
    });


    unip::Property::Visitor writingVisitor(
        [](const unip::Property& property){
        using namespace unip;
        using any_t = Property::any_type;

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

        if(Property::is_any<char>(value)){
            value = 'A';
        }
        else if(Property::is_any<short>(value)){
            value = (short)0x4321;
        }
        else if(Property::is_any<int>(value)){
            value = (int)+1;
        }
        else if(Property::is_any<float>(value)){
            value = (float)3.14/2;
        }
        else if(Property::is_any<Range>(value)){
            value = (const Range&)range;
        }
        else if(Property::is_any<std::string>(value)){
            const std::string& className = Property::cast_any<const std::string&>(value);
            value = std::string("Changed ")+className;            
        }
        std::cout<<"Writting new value to: [" <<property.name()<<"]\n";
        
        property.write(value);
        

        return true;
        
    });

    simpleClass.propertiesFunc(readingVisitor);
    std::cout<<"\n<------------------------------------->\n\n";

    simpleClass.propertiesFunc(writingVisitor);
    std::cout<<"\n<------------------------------------->\n\n";

    simpleClass.propertiesFunc(readingVisitor);


}