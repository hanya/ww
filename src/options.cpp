
#include "options.hpp"
#include "config.hpp"
#include "defs.hpp"
#include "tools.hpp"

#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#define OPTIONS_IMPLE_NAME    "mytools.config.CppWatchingWindowOptions"
#define OPTIONS_SERVICE_NAME  OPTIONS_IMPLE_NAME

#define OUSTR( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )

namespace ww
{

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::rtl::OUString;

#define CTL_INPUT_LINE "check_InputLine"
#define CTL_STORE      "check_StoreWatches"
#define CTL_ADD_CELLS  "num_AddCells"
#define CTL_LINE_1     "FixedLine1"


WWOptions::WWOptions( const Reference< XComponentContext >&xComponentContext )
 : WWOptions_Base( m_aMutex )
 , m_xContext( xComponentContext )
 , m_bUseInputLine( true )
 //, m_bStoreWatches( true )
 //, m_nWarnCells( 20 )
{
}

WWOptions::~WWOptions()
{
}

Reference< XInterface > WWOptions::create( const Reference< XComponentContext > &xComponentContext)
{
    return static_cast< ::cppu::OWeakObject * >( new WWOptions( xComponentContext ) );
}

// XContainerWindowEventHandler
sal_Bool SAL_CALL WWOptions::callHandlerMethod( 
        const Reference< XWindow >& xWindow, const Any& EventObject, const ::rtl::OUString& MethodName ) throw (WrappedTargetException, RuntimeException)
{
    if ( MethodName.equalsAsciiL( "external_event", 14 ) )
    {
        OUString sEvt;
        if ( EventObject >>= sEvt )
        {
            if ( sEvt.equalsAsciiL( "ok", 2 ) )
                return saveData( xWindow, sEvt );
            else if ( sEvt.equalsAsciiL( "back", 4 ) || 
                      sEvt.equalsAsciiL( "initialize", 10 ) )
                return loadData( xWindow, sEvt );
        }
    }
    return sal_False;
}

Sequence< OUString > SAL_CALL WWOptions::getSupportedMethodNames() throw (RuntimeException)
{
    Sequence< OUString > aRet( 1 );
    aRet[0] = OUString::createFromAscii( "external_event" );
    return aRet;
}


#define BOOL_TO_CHECK_STATE( b ) b ? (sal_Int16)1 : (sal_Int16)0


::sal_Bool WWOptions::loadData( const Reference< XWindow >& rWindow, const OUString & rEventName )
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        if ( !rEventName.equalsAsciiL( "back", 4 ) )
        {
            ConfigReader aConfig( m_xContext );
            m_bUseInputLine = aConfig.getBoolValue( OUSTR( PROP_USE_INPUT_LINE ) );
            //m_bStoreWatches = aConfig.getBoolValue( OUSTR( PROP_STORE_WATCHES ) );
            //m_nWarnCells = aConfig.getIntValue( OUSTR( PROP_WARN ) );
        }
        Reference< XControlContainer > xContainer( rWindow, UNO_QUERY_THROW );
        
        Reference< XControl > xControl;
        Reference< XPropertySet > xPropSet;
        
        xControl.set( xContainer->getControl( OUSTR( CTL_INPUT_LINE ) ), UNO_SET_THROW );
        xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( OUSTR( "State" ), 
                    makeAny( BOOL_TO_CHECK_STATE( m_bUseInputLine ) ) );
        /*
        xControl.set( xContainer->getControl( OUSTR( CTL_STORE ) ), UNO_SET_THROW );
        xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( OUSTR( "State" ), 
                    makeAny( BOOL_TO_CHECK_STATE( m_bStoreWatches ) ) );
        
        xControl.set( xContainer->getControl( OUSTR( CTL_ADD_CELLS ) ), UNO_SET_THROW );
        xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( OUSTR( "Value" ), 
                    makeAny( (sal_Int32)m_nWarnCells ) );
        */
        if ( rEventName.equalsAsciiL( "initialize", 10 ) )
        {
            // ToDo translate
            Reference< ::com::sun::star::resource::XStringResourceWithLocation > xRes( 
                getResourceLoader( m_xContext, OUSTR( RES_DIR ), OUSTR( RES_NAME ) ) );
            if ( xRes.is() )
            {
                xControl.set( xContainer->getControl( OUSTR( CTL_LINE_1 ) ), UNO_SET_THROW );
                xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
                xPropSet->setPropertyValue( OUSTR( "Label" ), 
                            makeAny( xRes->resolveString( OUSTR( "id.label.default" ) ) ) );
                
                xControl.set( xContainer->getControl( OUSTR( CTL_INPUT_LINE ) ), UNO_SET_THROW );
                xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
                xPropSet->setPropertyValue( OUSTR( "Label" ), 
                            makeAny( xRes->resolveString( OUSTR( "id.label.inputline" ) ) ) );
            }
        }
    }
    catch ( Exception & )
    {
    }
    return sal_True;
}

::sal_Bool WWOptions::saveData( const Reference< XWindow >& rWindow, const OUString & rEventName )
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        Reference< XControlContainer > xContainer( rWindow, UNO_QUERY_THROW );
        
        Reference< XControl > xControl;
        Reference< XPropertySet > xPropSet;
        
        sal_Int16 nUseInputLine;
        //sal_Int16 nStoreWatches;
        //double fWarnCells;
        
        xControl.set( xContainer->getControl( OUSTR( CTL_INPUT_LINE ) ), UNO_SET_THROW );
        xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->getPropertyValue( OUSTR( "State" ) ) >>= nUseInputLine;
        /*
        xControl.set( xContainer->getControl( OUSTR( CTL_STORE ) ), UNO_SET_THROW );
        xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->getPropertyValue( OUSTR( "State" ) ) >>= nStoreWatches;
        
        xControl.set( xContainer->getControl( OUSTR( CTL_ADD_CELLS ) ), UNO_SET_THROW );
        xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->getPropertyValue( OUSTR( "Value" ) ) >>= fWarnCells;
        */
        bool bUseInputLine = nUseInputLine == (sal_Int16)1;
        //bool bStoreWatches = nStoreWatches == (sal_Int16)1;
        
        ConfigWriter aConfig( m_xContext );
        
        if ( bUseInputLine != m_bUseInputLine )
            aConfig.setBoolValue( OUSTR( PROP_USE_INPUT_LINE ), bUseInputLine );
        /*
        if ( bStoreWatches != m_bStoreWatches )
            aConfig.setBoolValue( OUSTR( PROP_STORE_WATCHES ), bStoreWatches );
        if ( ((sal_Int32)fWarnCells) != m_nWarnCells )
            aConfig.setIntValue( OUSTR( PROP_WARN ), (sal_Int32)fWarnCells );
        */
        aConfig.Commit();
    }
    catch ( Exception & )
    {
    }
    return sal_True;
}


// XServiceInfo

OUString WWOptions::getImplementationName_Static() 
{
    return OUString::createFromAscii( OPTIONS_IMPLE_NAME );
}

Sequence< OUString > WWOptions::getSupportedServiceNames_Static() 
{
    Sequence< OUString > aRet( 1 );
    aRet[0] = OUString::createFromAscii( OPTIONS_SERVICE_NAME );
    return aRet;
}


OUString SAL_CALL WWOptions::getImplementationName() throw (RuntimeException)
{
    return WWOptions::getImplementationName_Static();
}

::sal_Bool SAL_CALL WWOptions::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return ServiceName.equalsAscii( OPTIONS_SERVICE_NAME );
}

Sequence< OUString > SAL_CALL WWOptions::getSupportedServiceNames() throw (RuntimeException)
{
    return WWOptions::getSupportedServiceNames_Static();
}


static struct cppu::ImplementationEntry g_entries[] = 
{
    {
        WWOptions::create, 
        WWOptions::getImplementationName_Static, 
        WWOptions::getSupportedServiceNames_Static, 
        cppu::createSingleComponentFactory, 
        0, 
        0
    }, 
    {0, 0, 0, 0, 0, 0}
};

} // namespace ww

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(const sal_Char **ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ww::g_entries);
}

SAL_DLLPUBLIC_EXPORT void* SAL_CALL
component_getFactory(const sal_Char *pImplName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ww::g_entries);
}

}
