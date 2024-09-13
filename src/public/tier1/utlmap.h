//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#pragma once

#include "tier0/dbg.h"
#include "utlrbtree.h"

//-----------------------------------------------------------------------------
//
// Purpose:	An associative container. Pretty much identical to std::map.
//
//-----------------------------------------------------------------------------

// This is a useful macro to iterate from start to end in order in a map
#define FOR_EACH_MAP( mapName, iteratorName ) \
	for ( int iteratorName = ( mapName ).FirstInorder(); ( mapName ).IsUtlMap && iteratorName != ( mapName ).InvalidIndex(); iteratorName = ( mapName ).NextInorder( iteratorName ) )

// faster iteration, but in an unspecified order
#define FOR_EACH_MAP_FAST( mapName, iteratorName )                                                                \
	for ( int iteratorName = 0; ( mapName ).IsUtlMap && iteratorName < ( mapName ).MaxElement(); ++iteratorName ) \
		if ( !( mapName ).IsValidIndex( iteratorName ) ) continue;                                                \
		else

struct base_utlmap_t {
public:
	// This enum exists so that FOR_EACH_MAP and FOR_EACH_MAP_FAST cannot accidentally
	// be used on a type that is not a CUtlMap. If the code compiles then all is well.
	// The check for IsUtlMap being true should be free.
	// Using an enum rather than a static const bool ensures that this trick works even
	// with optimizations disabled on gcc.
	enum CompileTimeCheck {
		IsUtlMap = 1
	};
};

template<typename K, typename T, typename I = uint16>
class CUtlMap : public base_utlmap_t {
public:
	using KeyType_t = K;
	using ElemType_t = T;
	using IndexType_t = I;

	// Less func typedef
	// Returns true if the first parameter is "less" than the second
	using LessFunc_t = bool(*)( const KeyType_t&, const KeyType_t& );

	// constructor, destructor
	// Left at growSize = 0, the memory will first allocate 1 element and double in size
	// at each increment.
	// LessFunc_t is required, but may be set after the constructor using SetLessFunc() below
	explicit CUtlMap( int growSize = 0, int initSize = 0, LessFunc_t lessfunc = 0 )
		: m_Tree( growSize, initSize, CKeyLess( lessfunc ) ) { }

	explicit CUtlMap( LessFunc_t lessfunc )
		: m_Tree( CKeyLess( lessfunc ) ) { }

	void EnsureCapacity( int num ) { m_Tree.EnsureCapacity( num ); }

	// gets particular elements
	ElemType_t& Element( IndexType_t i ) { return this->m_Tree.Element( i ).elem; }
	const ElemType_t& Element( IndexType_t i ) const { return this->m_Tree.Element( i ).elem; }
	ElemType_t& operator[]( IndexType_t i ) { return this->m_Tree.Element( i ).elem; }
	const ElemType_t& operator[]( IndexType_t i ) const { return this->m_Tree.Element( i ).elem; }
	ElemType_t& operator[]( KeyType_t k ) { return this->m_Tree.Element( this->Find( k ) ).elem; }
	const ElemType_t& operator[]( KeyType_t k ) const { return this->m_Tree.Element( this->Find( k ) ).elem; }
	KeyType_t& Key( IndexType_t i ) { return this->m_Tree.Element( i ).key; }
	const KeyType_t& Key( IndexType_t i ) const { return this->m_Tree.Element( i ).key; }


	// Num elements
	[[nodiscard]]
	unsigned int Count() const { return m_Tree.Count(); }

	// Max "size" of the vector
	IndexType_t MaxElement() const { return m_Tree.MaxElement(); }

	// Checks if a node is valid and in the map
	bool IsValidIndex( IndexType_t i ) const { return m_Tree.IsValidIndex( i ); }

	// Checks if the map as a whole is valid
	[[nodiscard]]
	bool IsValid() const { return m_Tree.IsValid(); }

	// Invalid index
	static IndexType_t InvalidIndex() { return CTree::InvalidIndex(); }

	/// region ASRC_EDIT
	struct Node_t {
		Node_t() { }; // NOLINT(*-use-equals-default), needed as it may be implicitly deleted...

		Node_t( const Node_t& from )
			: key( from.key ), elem( from.elem ) { }

		KeyType_t key;
		ElemType_t elem;
	};

	// STL compatible const_iterator class
	class const_iterator {
	public:
		// Default constructor -- gives a currently unusable iterator.
		const_iterator()
			: m_Map( 0 ), m_index( CUtlMap::InvalidIndex() ) {}
		// Normal constructor.
		const_iterator( const CUtlMap& pMap, IndexType_t index )
			: m_Map( &pMap ), m_index( index ) {}

		// Pre-increment operator++. This is the most efficient increment
		// operator so it should always be used.
		const_iterator& operator++() {
			do {
				if ( this->m_index >= this->m_Map->MaxElement() ) {
					this->m_index = CUtlMap::InvalidIndex();
					break;
				}
				this->m_index += 1;
			} while ( !this->m_Map->IsValidIndex( this->m_index ) );

			return *this;
		}
		// Post-increment operator++. This is less efficient than pre-increment.
		const_iterator operator++( int ) {
			// Copy ourselves.
			const_iterator temp = *this;
			// Increment ourselves.
			++*this;
			// Return the copy.
			return temp;
		}

		// Pre-decrement operator--. This is the most efficient decrement
		// operator so it should always be used.
		const_iterator& operator--() {
			Assert( this->m_index != 0 );

			do this->m_index -= 1; // TODO: Verify this doesn't cause irrational behavior
			while (! this->m_Map->IsValidIndex( this->m_index ) );

			return *this;
		}
		// Post-decrement operator--. This is less efficient than post-decrement.
		const_iterator operator--( int ) {
			// Copy ourselves.
			const_iterator temp = *this;
			// Decrement ourselves.
			--*this;
			// Return the copy.
			return temp;
		}

		bool operator==( const const_iterator& other ) const {
			Assert( this->m_Map == other.m_Map );
			return this->m_index == other.m_index;
		}

		bool operator!=( const const_iterator& other ) const {
			Assert( this->m_Map == other.m_Map );
			return this->m_index != other.m_index;
		}

		const Node_t& operator*() const {
			return this->m_Map->m_Tree.Element( this->m_index );
		}

		const Node_t* operator->() const {
			return ( &**this );
		}

	protected:
		// Use a pointer rather than a reference so that we can support assignment of iterators.
		const CUtlMap* m_Map;
		IndexType_t m_index;
	};

	// STL compatible iterator class, using derivation so that a non-const
	// list can return a const_iterator.
	class iterator : public const_iterator {
	public:
		using Base = const_iterator;

		// Default constructor -- gives a currently unusable iterator.
		iterator() = default;

		// Normal constructor.
		iterator( const CUtlMap& pMap, IndexType_t index )
			: const_iterator( pMap, index ) { }

		// Pre-increment operator++. This is the most efficient increment
		// operator so it should always be used.
		iterator& operator++() {
			do {
				if ( this->m_index >= this->m_Map->MaxElement() ) {
					this->m_index = CUtlMap::InvalidIndex();
					break;
				}
				this->m_index += 1;
			} while ( !this->m_Map->IsValidIndex( this->m_index ) );

			return *this;
		}
		// Post-increment operator++. This is less efficient than pre-increment.
		iterator operator++( int ) {
			// Copy ourselves.
			iterator temp = *this;
			// Increment ourselves.
			++*this;
			// Return the copy.
			return temp;
		}

		// Pre-decrement operator--. This is the most efficient decrement
		// operator so it should always be used.
		iterator& operator--() {
			Assert( this->m_index != 0 );

			do this->m_index -= 1; // TODO: Verify this doesn't cause irrational behavior
			while (! this->m_Map->IsValidIndex( this->m_index ) );

			return *this;
		}
		// Post-decrement operator--. This is less efficient than post-decrement.
		iterator operator--( int ) {
			// Copy ourselves.
			iterator temp = *this;
			// Decrement ourselves.
			--*this;
			// Return the copy.
			return temp;
		}

		Node_t& operator*() const {
			return const_cast<CUtlMap*>( this->m_Map )->m_Tree.Element( this->m_index );
		}

		Node_t* operator->() const {
			return ( &**this );
		}
	};

	const_iterator begin() const {
		return const_iterator( *this, this->FirstInorder() );
	}
	iterator begin() {
		return iterator( *this, this->FirstInorder() );
	}

	const_iterator end() const {
		return const_iterator( *this, InvalidIndex() );
	}
	iterator end() {
		return iterator( *this, InvalidIndex() );
	}
	/// endregion ASRC_EDIT


	// Sets the less func
	void SetLessFunc( LessFunc_t func ) {
		m_Tree.SetLessFunc( CKeyLess( func ) );
	}

	// Insert method (inserts in order)
	IndexType_t Insert( const KeyType_t& key, const ElemType_t& insert ) {
		Node_t node;
		node.key = key;
		node.elem = insert;
		return m_Tree.Insert( node );
	}

	IndexType_t Insert( const KeyType_t& key ) {
		Node_t node;
		node.key = key;
		return m_Tree.Insert( node );
	}

	// Find method
	IndexType_t Find( const KeyType_t& key ) const {
		Node_t dummyNode;
		dummyNode.key = key;
		return m_Tree.Find( dummyNode );
	}

	// Remove methods
	void RemoveAt( IndexType_t i ) { m_Tree.RemoveAt( i ); }
	bool Remove( const KeyType_t& key ) {
		Node_t dummyNode;
		dummyNode.key = key;
		return m_Tree.Remove( dummyNode );
	}

	void RemoveAll() { m_Tree.RemoveAll(); }
	void Purge() { m_Tree.Purge(); }

	// Purges the list and calls delete on each element in it.
	void PurgeAndDeleteElements();

	// Iteration
	IndexType_t FirstInorder() const { return m_Tree.FirstInorder(); }
	IndexType_t NextInorder( IndexType_t i ) const { return m_Tree.NextInorder( i ); }
	IndexType_t PrevInorder( IndexType_t i ) const { return m_Tree.PrevInorder( i ); }
	IndexType_t LastInorder() const { return m_Tree.LastInorder(); }

	// If you change the search key, this can be used to reinsert the
	// element into the map.
	void Reinsert( const KeyType_t& key, IndexType_t i ) {
		m_Tree[ i ].key = key;
		m_Tree.Reinsert( i );
	}

	IndexType_t InsertOrReplace( const KeyType_t& key, const ElemType_t& insert ) {
		IndexType_t i = Find( key );
		if ( i != InvalidIndex() ) {
			Element( i ) = insert;
			return i;
		}

		return Insert( key, insert );
	}

	void Swap( CUtlMap<K, T, I>& that ) {
		m_Tree.Swap( that.m_Tree );
	}

	class CKeyLess {
	public:
		CKeyLess( LessFunc_t lessFunc ) : m_LessFunc( lessFunc ) {}

		bool operator!() const {
			return !m_LessFunc;
		}

		bool operator()( const Node_t& left, const Node_t& right ) const {
			return m_LessFunc( left.key, right.key );
		}

		LessFunc_t m_LessFunc;
	};

	typedef CUtlRBTree<Node_t, I, CKeyLess> CTree;

	CTree* AccessTree() { return &m_Tree; }

protected:
	CTree m_Tree;
};

//-----------------------------------------------------------------------------

// Purges the list and calls delete on each element in it.
template<typename K, typename T, typename I>
inline void CUtlMap<K, T, I>::PurgeAndDeleteElements() {
	for ( I i = 0; i < MaxElement(); ++i ) {
		if ( !IsValidIndex( i ) )
			continue;

		delete Element( i );
	}

	Purge();
}

//-----------------------------------------------------------------------------

// This is horrible and slow and meant to be used only when you're dealing with really
// non-time/memory-critical code and desperately want to copy a whole map element-by-element
// for whatever reason.
template<typename K, typename T, typename I>
void DeepCopyMap( const CUtlMap<K, T, I>& pmapIn, CUtlMap<K, T, I>* out_pmapOut ) {
	Assert( out_pmapOut );

	out_pmapOut->Purge();
	FOR_EACH_MAP_FAST( pmapIn, i ) {
		out_pmapOut->Insert( pmapIn.Key( i ), pmapIn.Element( i ) );
	}
}
