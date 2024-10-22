//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// $Header: $
// $NoKeywords: $
//
// A growable array class that keeps all elements in order using binary search
//===========================================================================//
#pragma once
#include "utlvector.h"


//-----------------------------------------------------------------------------
// class CUtlSortVector:
// description:
//   This is a sorted order-preserving vector. Items may be inserted or removed
//   at any point in the vector. When an item is inserted, all elements are
//   moved down by one element using memmove. When an item is removed, all
//   elements are shifted back down. Items are searched for in the vector
//   using a binary search technique. Clients must pass in a Less() function
//   into the constructor of the vector to determine the sort order.
//-----------------------------------------------------------------------------


template<class T>
class CUtlSortVectorDefaultLess {
public:
	bool Less( const T& lhs, const T& rhs, void* ) {
		return lhs < rhs;
	}
};

template<class T, class LessFunc = CUtlSortVectorDefaultLess<T>, class BaseVector = CUtlVector<T>>
class CUtlSortVector : public BaseVector {
	typedef BaseVector BaseClass;

public:
	/// constructor
	explicit CUtlSortVector( int nGrowSize = 0, int initSize = 0 );
	CUtlSortVector( T* pMemory, int numElements );

	/// inserts (copy constructs) an element in sorted order into the list
	int Insert( const T& src );

	/// inserts (copy constructs) an element in sorted order into the list if it isn't already in the list
	int InsertIfNotFound( const T& src );

	/// Finds an element within the list using a binary search. These are templatized based upon the key
	/// in which case the less function must handle the Less function for key, T and T, key
	template<typename TKey>
	int Find( const TKey& search ) const;
	template<typename TKey>
	int FindLessOrEqual( const TKey& search ) const;
	template<typename TKey>
	int FindLess( const TKey& search ) const;

	/// Removes a particular element
	void Remove( const T& search );
	void Remove( int i );

	/// Allows methods to set a context to be used with the less function..
	void SetLessContext( void* pCtx );

	/// A version of insertion that will produce an un-ordered list.
	/// Note that you can only use this index until sorting is redone with RedoSort!!!
	int InsertNoSort( const T& src );
	void RedoSort( bool bForceSort = false );

	/// Use this to insert at a specific insertion point; using FindLessOrEqual
	/// is required for use this this. This will test that what you've inserted
	/// produces a correctly ordered list.
	int InsertAfter( int nElemIndex, const T& src );

	/// finds a particular element using a linear search. Useful when used
	/// in between calls to InsertNoSort and RedoSort
	template<typename TKey>
	int FindUnsorted( const TKey& src ) const;

public:
	// No copy constructor
	explicit CUtlSortVector( const CUtlSortVector<T, LessFunc>& ) = delete;

	// never call these; illegal for this class
	int AddToHead() = delete;
	int AddToTail() = delete;
	int InsertBefore( int elem ) = delete;
	int InsertAfter( int elem ) = delete;
	int InsertBefore( int elem, const T& src ) = delete;
	int AddToHead( const T& src ) = delete;
	int AddToTail( const T& src ) = delete;
	int AddMultipleToHead( int num ) = delete;
	int AddMultipleToTail( int num, const T* pToCopy = nullptr ) = delete;
	int InsertMultipleBefore( int elem, int num, const T* pToCopy = nullptr ) = delete;
	int InsertMultipleAfter( int elem, int num ) = delete;
	int AddVectorToTail( CUtlVector<T> const& src ) = delete;
protected:

	struct QSortContext_t {
		void* m_pLessContext;
		LessFunc* m_pLessFunc;
	};

	static int CompareHelper( void* context, const T* lhs, const T* rhs ) {
		auto* ctx = static_cast<QSortContext_t*>( context );
		if ( ctx->m_pLessFunc->Less( *lhs, *rhs, ctx->m_pLessContext ) ) {
			return -1;
		}
		if ( ctx->m_pLessFunc->Less( *rhs, *lhs, ctx->m_pLessContext ) ) {
			return 1;
		}
		return 0;
	}

	void* m_pLessContext;
	bool m_bNeedsSort;

private:
	template<typename TKey>
	int FindLessOrEqual( const TKey& search, bool* pFound ) const;

	void QuickSort( LessFunc& less, int X, int I );
};


//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
CUtlSortVector<T, LessFunc, BaseVector>::CUtlSortVector( int nGrowSize, int initSize )
	: BaseVector( nGrowSize, initSize ), m_pLessContext( nullptr ), m_bNeedsSort( false ) { }

template<class T, class LessFunc, class BaseVector>
CUtlSortVector<T, LessFunc, BaseVector>::CUtlSortVector( T* pMemory, int numElements )
	: BaseVector( pMemory, numElements ), m_pLessContext( nullptr ), m_bNeedsSort( false ) { }

//-----------------------------------------------------------------------------
// Allows methods to set a context to be used with the less function..
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
void CUtlSortVector<T, LessFunc, BaseVector>::SetLessContext( void* pCtx ) {
	m_pLessContext = pCtx;
}

//-----------------------------------------------------------------------------
// grows the vector
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
int CUtlSortVector<T, LessFunc, BaseVector>::Insert( const T& src ) {
	AssertFatal( !m_bNeedsSort );

	int pos = FindLessOrEqual( src ) + 1;
	this->GrowVector();
	this->ShiftElementsRight( pos );
	CopyConstruct<T>( &this->Element( pos ), src );
	return pos;
}

template<class T, class LessFunc, class BaseVector>
int CUtlSortVector<T, LessFunc, BaseVector>::InsertNoSort( const T& src ) {
	m_bNeedsSort = true;
	int lastElement = BaseVector::m_Size;
	// Just stick the new element at the end of the vector, but don't do a sort
	this->GrowVector();
	this->ShiftElementsRight( lastElement );
	CopyConstruct( &this->Element( lastElement ), src );
	return lastElement;
}

/// inserts (copy constructs) an element in sorted order into the list if it isn't already in the list
template<class T, class LessFunc, class BaseVector>
int CUtlSortVector<T, LessFunc, BaseVector>::InsertIfNotFound( const T& src ) {
	AssertFatal( !m_bNeedsSort );
	bool bFound;
	int pos = FindLessOrEqual( src, &bFound );
	if ( bFound ) {
		return pos;
	}

	pos += 1;
	this->GrowVector();
	this->ShiftElementsRight( pos );
	CopyConstruct<T>( &this->Element( pos ), src );
	return pos;
}

template<class T, class LessFunc, class BaseVector>
int CUtlSortVector<T, LessFunc, BaseVector>::InsertAfter( int nElemIndex, const T& src ) {
	int nInsertedIndex = this->BaseClass::InsertAfter( nElemIndex, src );

	#if IsDebug()
		LessFunc less;
		if ( nInsertedIndex > 0 ) {
			Assert( less.Less( this->Element( nInsertedIndex - 1 ), src, m_pLessContext ) );
		}
		if ( nInsertedIndex < BaseClass::Count() - 1 ) {
			Assert( less.Less( src, this->Element( nInsertedIndex + 1 ), m_pLessContext ) );
		}
	#endif
	return nInsertedIndex;
}


template<class T, class LessFunc, class BaseVector>
void CUtlSortVector<T, LessFunc, BaseVector>::QuickSort( LessFunc& less, int X, int I ) {
	using QSortCompareFunc_t = int(__cdecl*)( void* context, const void*, const void* );

	#if IsLinux()
		extern "C" void qsort_s( void* base, size_t num, size_t width, QSortCompareFunc_t compare, void* context );
	#endif

	if ( this->Count() > 1 ) {
		QSortContext_t ctx;
		ctx.m_pLessContext = m_pLessContext;
		ctx.m_pLessFunc = &less;

		qsort_s( this->Base(), this->Count(), sizeof( T ), static_cast<QSortCompareFunc_t>( &CUtlSortVector<T, LessFunc>::CompareHelper ), &ctx );
	}
}

template<class T, class LessFunc, class BaseVector>
void CUtlSortVector<T, LessFunc, BaseVector>::RedoSort( const bool bForceSort /*= false */ ) {
	if ( !m_bNeedsSort && !bForceSort ) {
		return;
	}

	m_bNeedsSort = false;
	LessFunc less;
	QuickSort( less, 0, this->Count() - 1 );
}

//-----------------------------------------------------------------------------
// finds a particular element
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
template<typename TKey>
int CUtlSortVector<T, LessFunc, BaseVector>::Find( const TKey& search ) const {
	AssertFatal( !m_bNeedsSort );

	LessFunc less;

	int start = 0, end = this->Count() - 1;
	while ( start <= end ) {
		const int mid = start + end >> 1;
		if ( less.Less( this->Element( mid ), search, m_pLessContext ) ) {
			start = mid + 1;
		} else if ( less.Less( search, this->Element( mid ), m_pLessContext ) ) {
			end = mid - 1;
		} else {
			return mid;
		}
	}
	return -1;
}


//-----------------------------------------------------------------------------
// finds a particular element using a linear search. Useful when used
// in between calls to InsertNoSort and RedoSort
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
template<typename TKey>
int CUtlSortVector<T, LessFunc, BaseVector>::FindUnsorted( const TKey& src ) const {
	LessFunc less;
	const int count = this->Count();
	for ( int i = 0; i < count; ++i ) {
		if ( less.Less( this->Element( i ), src, m_pLessContext ) ) {
			continue;
		}
		if ( less.Less( src, this->Element( i ), m_pLessContext ) ) {
			continue;
		}
		return i;
	}
	return -1;
}


//-----------------------------------------------------------------------------
// finds a particular element
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
template<typename TKey>
int CUtlSortVector<T, LessFunc, BaseVector>::FindLessOrEqual( const TKey& search, bool* pFound ) const {
	AssertFatal( !m_bNeedsSort );

	LessFunc less;
	int start = 0, end = this->Count() - 1;
	while ( start <= end ) {
		const int mid = start + end >> 1;
		if ( less.Less( this->Element( mid ), search, m_pLessContext ) ) {
			start = mid + 1;
		} else if ( less.Less( search, this->Element( mid ), m_pLessContext ) ) {
			end = mid - 1;
		} else {
			*pFound = true;
			return mid;
		}
	}

	*pFound = false;
	return end;
}

template<class T, class LessFunc, class BaseVector>
template<typename TKey>
int CUtlSortVector<T, LessFunc, BaseVector>::FindLessOrEqual( const TKey& search ) const {
	bool bFound;
	return FindLessOrEqual( search, &bFound );
}

template<class T, class LessFunc, class BaseVector>
template<typename TKey>
int CUtlSortVector<T, LessFunc, BaseVector>::FindLess( const TKey& search ) const {
	AssertFatal( !m_bNeedsSort );

	LessFunc less;
	int start = 0, end = this->Count() - 1;
	while ( start <= end ) {
		const int mid = start + end >> 1;
		if ( less.Less( this->Element( mid ), search, m_pLessContext ) ) {
			start = mid + 1;
		} else {
			end = mid - 1;
		}
	}
	return end;
}


//-----------------------------------------------------------------------------
// Removes a particular element
//-----------------------------------------------------------------------------
template<class T, class LessFunc, class BaseVector>
void CUtlSortVector<T, LessFunc, BaseVector>::Remove( const T& search ) {
	AssertFatal( !m_bNeedsSort );

	const int pos = Find( search );
	if ( pos != -1 ) {
		BaseVector::Remove( pos );
	}
}

template<class T, class LessFunc, class BaseVector>
void CUtlSortVector<T, LessFunc, BaseVector>::Remove( int i ) {
	BaseVector::Remove( i );
}
