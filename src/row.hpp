
#ifndef _ROW_HPP_
#define _ROW_HPP_

#ifdef SINGLE_WATCHING
#include <hash_map>
#endif
#include <vector>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>

// Do not allow to watch the same cell twice or more
//#define SINGLE_WATCHING
//#undef SINGLE_WATCHING

namespace ww
{

class Row;
class Rows;


typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::util::XModifyListener
            > ModifyListener_Base;
class ModifyListener :  protected ::cppu::BaseMutex, 
                        public ModifyListener_Base
{
public:
    ModifyListener( Row * pRow );
    ~ModifyListener();
    
    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    Row * m_pRow;
};


class Row
{
public:
    Row( Rows * pRows, const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > & xCell );
    ~Row();
    
    void modified();
    
    void setIndex( const int nIndex ) { m_nIndex = nIndex; };
    int getIndex() const { return m_nIndex; };
    
    ::rtl::OUString getAddress() const;
    ::rtl::OUString getFormula() const;
    void setFormula( const ::rtl::OUString & rFormula ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > getData( ::com::sun::star::uno::Any & rOutTooltip ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > getTokens() const;
#ifdef SINGLE_WATCHING
    sal_Int32 getIntegerAddress() const;
#endif
    void enableWatching( const bool bState );
    void listenerDisposing();
    
    static void setLabels( const ::com::sun::star::uno::Sequence< ::rtl::OUString > & aRes );
    static bool hasLabels();
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > m_xCell;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyListener;
    int m_nIndex;
    
    bool m_bWatching;
    Rows * m_pRows;
    
    //::rtl::OUString getTooltipFromData( const ::com::sun::star::uno::Sequence< ::rtl::OUString > & aData ) const;
    ::rtl::OUString getTooltipFromData( const ::rtl::OUString & rSheet, const ::rtl::OUString & rCell, 
                    const ::rtl::OUString & rValue, const ::rtl::OUString & rFormula ) const;
    
    static ::rtl::OUString m_sSheetLabel;
    static ::rtl::OUString m_sCellLabel;
    static ::rtl::OUString m_sValueLabel;
    static ::rtl::OUString m_sFormulaLabel;
};


typedef ::std::vector< Row * > RowVector;


class Rows : protected ::cppu::BaseMutex
{
public:
    Rows( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XMutableGridDataModel > & xGridDataModel );
    ~Rows();
    
    int getWatchCount() const;
    void updateRow( Row * pRow ) const;
    void addWatch( const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > & rxCell ) throw ( ::com::sun::star::lang::IllegalArgumentException );
    void removeWatches( const ::com::sun::star::uno::Sequence< sal_Int32 > & rIndex );
    void removeAllWatches();
    void updateAll();
    void moveWatches( const sal_Int32 nDest, const ::com::sun::star::uno::Sequence< sal_Int32 > & rIndex );
    ::rtl::OUString getRowAddress( const ::sal_Int32 nIndex ) const;
    ::rtl::OUString getRowFormula( const ::sal_Int32 nIndex ) const;
    void setRowFormula( const ::sal_Int32 nIndex, const ::rtl::OUString & rFormula ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > getReferenceAddresses( const sal_Int32 nIndex ) const;
    
    void setLabels( const ::com::sun::star::uno::Sequence< ::rtl::OUString > & aRes );
    bool hasLabels() const;
private:
    ::com::sun::star::uno::Sequence< ::sal_Int32 > m_aIndexes;
    RowVector m_aRows;
#ifdef SINGLE_WATCHING
    typedef ::std::hash_map< sal_Int32, bool > AddressHash;
    AddressHash m_aAddresses;
#endif
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XMutableGridDataModel > m_xGridDataModel;
    
    void insertRow( const int nIndex, Row * pRow );
    void reassignIndex( const int nStart );
};

} // namespace ww

#endif
