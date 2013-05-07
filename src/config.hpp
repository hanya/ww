
#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace ww
{

class Config
{
public:
    Config( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const bool bWritable );
    ~Config();
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xPropSet;
};

class ConfigReader : public Config
{
public:
    ConfigReader( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    ~ConfigReader();
    
    sal_Bool getBoolValue( const ::rtl::OUString & rName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );
    sal_Int32 getIntValue( const ::rtl::OUString & rName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );;
};

class ConfigWriter : public Config
{

public:
    ConfigWriter( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    ~ConfigWriter();
    
    void setBoolValue( const ::rtl::OUString & rName, const sal_Bool bState ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );;
    void setIntValue( const ::rtl::OUString & rName, const sal_Int32 nValue ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );;
    
    void Commit() throw ( ::com::sun::star::uno::RuntimeException );
};

} // namespace ww

#endif
