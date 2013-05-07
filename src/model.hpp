
#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include "row.hpp"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>

namespace ww
{

class WWView;

class WWModel
{
public:
    
    WWModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    ~WWModel();
    
    void setView( WWView * pView );
    void disconnectView();
    
    void addEntry();
    void addCell( const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > & xSelection );
    void addCellRange( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRange > & xSheetCellRange );
    void addCellRanges( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > & xSheetCellRanges );
    void removeEntries( const ::com::sun::star::uno::Sequence< sal_Int32 > & aIndex );
    void removeAllEntries();
    void updateAll();
    void setFormula( const sal_Int32 nIndex, const ::rtl::OUString & rFormula ) const;
    void moveEntries( const sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< sal_Int32 > & aIndex );
    int getRowCount() const;
    ::rtl::OUString getAddress( const ::sal_Int32 nIndex ) const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getCellReferences( const sal_Int32 nIndex ) const;
    ::rtl::OUString getFormula( const ::sal_Int32 nIndex ) const;
    
private:
    
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    WWView * m_pView;
    Rows * m_pRows;
    
};

} // namespace ww

#endif
