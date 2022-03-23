# Named Properties for C++


## Motivation
For years I was looking for a way to have UI and serialization able to interact with classes universally without needing to know details about the classes they are interacting with, via commonly know types And still keep it simple for each class that will be presenting it's members to outside visitors.
## Description
A highly customizable and header only library which is offering simple way to present named members of an class(or really anything that can be access via reference or value) to a user defined visitor that can be used for variety of cases from debugging to serialization of objects, to GUI which will simply interpret the members one by one into a final picture.

Properties are made with idea of high customizability which will not push any particular design. When default implementation is used via predefined interface, priority is to move when possible(new value) and in the same time to not make class mutable avoiding property defined mutable rules via write function. In the same time using stack only allocation.

## Depencies
This project has no dependencies  in terms of libraries. But requires at least C++17 standard to be used when compiling.
 
## Usage
Behavior of properties using default implementation will prefer types with size bigger than pointer to be stored as reference to passed variable to avoid expensive copies, otherwise it will copy variable by value. 
### Basic:
#### Note: assume that 'using namespace nap;' was declared
Defining a single property:
```cpp
int i = 5;
//some list of properties
...
Property("MyInt", i),
... 
```
this will copy/reference variable 'i' into underling type (in default implementation std::any is used)


Then to access underling value:
```cpp
const Property& property = <property entry from some list>;
...
Property::any_type value;
property.read(value); // read value from property into local value
//you can access property name via property.name();

if(Property::is_any<int>(value)){
    int i = Property::cast_any<int>(value); // extract the value of given type
    i = i + 1;
    value = Property::make_any(i); // make underling type in order to write new value via write function
    property.write(value);
}
```
Build in mechanism for visiting each property in list:
```cpp
// use lambda to define how and what to read/write for each property
Property::Visitor visitor([](const Property& property){
//  read and write into property
});
...
for(const Property& property : propertyList){
    visitor.visit(property);
    
}
//or equivalent via build-in way:
Property::Visitor::visit(visitor, propertyList);
```
Now to put it all togather:
```cpp
class Point{
    void presentProperties(const Property::Visitor& visitor){
        Property::Visitor::visit(visitor, {
            Property("x", x),
            Property("y", y)
        });
    }
    float x=1.0,y=2.0;
};
int main(){
    auto printProperties = [](const Property& property){
        Property::any_type value;
        property.read(value);

        if(Property::is_any<float>(value)){
             std::cout<<"Value of "<<property.name()
                      <<": "<<Property::cast_any<float>(value)<<'\n';
        }
    }
    Property::Visitor visitor(printProperties);
    Point point;
    point.presentProperties(visitor);
}
```
Expected output:
```
Value of x: 1.0
Value of y: 2.0
```
### Macro
We can use macro to define a pass properties to simplify setup for each class which will hold properties:
```cpp
#define PROPERTIES(...) \
void presentProperties(const nap::Property::Visitor& visitor){\
using namespace nap;\
Property::Visitor::visit(visitor,{__VA_ARGS__});}
```
```cpp
class Point{
   PROPERTIES(
       Property("x", x),
       Property("y", y)
   )
   ...
```
### Advanced
When declaring properties, we can use custom write and read behavior. By default Property constructor takes reference to value given the context being const or otherwise.  
###### Given this examples which is using previously define macro
Readonly helper function:
```cpp
Property("x", Property::ReadOnly(x)),
```
ReadOnly will force Property to only create read function for given variable. This will force Property to only pass const reference to value.

To check if property is read only, we can use member function:
```cpp
if(property.isReadable()){
    property.read(value);
}
```

Custom read and write behavior for property:
```cpp
Property("x", 
        [this]{Property::any_type& value){
            value = Property::read(x);
            std::cout<<"Reading "<<x<<" from 'x'\n";
        },
        [this](Property::any_type& value)(
            x = Property::write<float>(value);
            std::cout<<"Writing "<<x<<" to 'x'\n";
        }
),
```
#### Note: Property::write and Property::read are used by Property constructors when not defining custom read and write functions.
For const context we can use following build in helper function which will only executes as long as all forwarded parameters are not const:

```cpp
[this](Property::any_type& value){
    Property::ExecWhenNotConst([&value](Point* This){
        This->x = Property::write<float>(value);
        std::cout<<"Writing "<<This->x<<" to 'x'\n";
    });
}
```
This is useful for const context functions e.g. to_string(). 
