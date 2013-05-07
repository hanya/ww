
#ifndef _TOOLS_HPP_
#define _TOOLS_HPP_

#include <com/sun/star/resource/XStringResourceWithLocation.hpp>


namespace ww
{

::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceWithLocation > getResourceLoader(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, 
        const ::rtl::OUString & rResDir, 
        const ::rtl::OUString & rFileName );


} // namespace ww

#endif
