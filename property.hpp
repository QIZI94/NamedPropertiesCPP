#pragma once

#include <functional>
#include <any>
#include <string_view>
#include <array>
/** Universal property */
namespace unip{

namespace detail{
struct DefaultInterface{
	using any_type = std::any;
	using string_type = std::string_view;
	template<typename T>
	static any_type make_any(T value){
		if constexpr(sizeof(T) > sizeof(void*)){
			return std::make_any<const T&>(value);
		}
		else{
			return std::make_any<T>(value);
		}
	}
	template<typename T>
	static auto cast_any(const any_type& any){
		if constexpr(sizeof(T) > sizeof(void*)){
			return std::any_cast<const T&>(any);
		}
		else{
			return std::any_cast<T>(any);
		}
	}
	template<typename T>
	static T& cast_any(any_type& any){
			return std::any_cast<T&>(any);
	}
};
}


template<class InterfaceImpl>
class PropertyTemplate{
public: //type definitions
	class Visitor{
	public: // functions
		using PropertyVisitFunc = std::function<bool(const PropertyTemplate&)>;
		Visitor(const PropertyVisitFunc& propertyVistFunc) : m_visitProperty(propertyVistFunc){}
		bool visit(const PropertyTemplate& property) const {
			return m_visitProperty(property);
		}

		template<class V, class PropertyArray>
		static bool visit(const V& visitor, const PropertyArray& properties){
			for(const auto& property : properties){
				if(visitor.visit(property) == false){
					return false;
				}
			}
			return true;
		}
		template<class V>
		static bool visit(const V& visitor, std::initializer_list<PropertyTemplate> ilProperties){
			for(const auto& property : ilProperties){
				if(visitor.visit(property) == false){
					return false;
				}
			}
			return true;
		}

	private: // members
		const PropertyVisitFunc m_visitProperty;
	};

	using WriteFunction = std::function<void(const std::any& entry)>;
	using ReadFunction  = std::function<void(std::any& entry)>;
	
	using interface     = InterfaceImpl;
	using string_type   = typename interface::string_type;
	using any_type      = typename interface::any_type;

public: // member functions
	PropertyTemplate(string_type name, const ReadFunction& readFunc, const WriteFunction& writeFunc) : m_name(name), m_read(readFunc), m_write(writeFunc) {}
	PropertyTemplate(string_type name, const ReadFunction& readFunc) : m_name(name), m_read(readFunc), m_write(nullptr) {}
	PropertyTemplate(string_type name, const WriteFunction& writeFunc) : m_name(name), m_read(nullptr), m_write(writeFunc) {}
	PropertyTemplate(string_type name) : m_name(name), m_read(nullptr), m_write(nullptr) {}

	template<typename T>
	PropertyTemplate(string_type name, const T& constMember) : m_name(name),
	m_read(
		[&constMember](typename interface::any_type& entry){
			entry = interface:: template make_any<T>(constMember);
		}
	), 
	m_write({})
	{}

	template<typename T>
	PropertyTemplate(string_type name, T& member) : m_name(name), 
	m_read(
		[&member](typename interface::any_type& entry){
			entry = interface:: template make_any<T>(member);
		}
	), 
	m_write(
		[&member](const typename interface::any_type& entry){
			member = interface:: template cast_any<T>(entry);
		}
	) 
	{}

	const string_type& name() const{return m_name;}
	void read(any_type& entry) const{m_read(entry);}
	void write(const any_type& entry) const{m_write(entry);}
	bool isReadOnly() const{ return (m_write == nullptr);}
	bool isWriteOnly() const{ return (m_read == nullptr);}
	bool isNameOnly() const{return (isWriteOnly() && isReadOnly());}

private: // members
	const typename interface::string_type m_name;
	const ReadFunction m_read;
	const WriteFunction m_write;
};

template<typename T>
const T& ReadOnly(const T& member){
	return member;
}

using Property = PropertyTemplate<detail::DefaultInterface>;
}