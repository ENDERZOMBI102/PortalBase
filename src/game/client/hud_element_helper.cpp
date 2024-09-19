//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Helper for the CHudElement class to add themselves to the list of hud elements
//
// $NoKeywords: $
//=============================================================================//
#include "hud_element_helper.h"
#include "cbase.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Start with empty list
CHudElementHelper* CHudElementHelper::m_sHelpers = nullptr;

//-----------------------------------------------------------------------------
// Purpose: Constructs a technology factory
// Input  : pfnCreate - fn Ptr to the constructor for this element
//			depth - determines position in the creation list, 100 is farthest into the screen
//					0 is nearest, default is 50
//
//-----------------------------------------------------------------------------
CHudElementHelper::CHudElementHelper( CHudElementFactory pfnCreate, int depth ) {
	//Insert into the list based on depth

	//List is empty, or element belongs at front, insert here
	if ( m_sHelpers == nullptr || depth >= m_sHelpers->m_iDepth ) {
		m_pNext = m_sHelpers;
		m_sHelpers = this;
	} else {
		//Scan to insert in decreasing depth order
		CHudElementHelper* pPrev = m_sHelpers;
		CHudElementHelper* pCurrent = m_sHelpers->m_pNext;

		while ( pCurrent != nullptr && depth < pCurrent->m_iDepth ) {
			pPrev = pCurrent;
			pCurrent = pCurrent->m_pNext;
		}

		pPrev->m_pNext = this;
		m_pNext = pCurrent;
	}

	m_iDepth = depth;

	// Set attributes
	assert( pfnCreate );
	m_pfnCreate = pfnCreate;
}

//-----------------------------------------------------------------------------
// Purpose: Returns next object in list
// Output : CHudElementHelper
//-----------------------------------------------------------------------------
CHudElementHelper* CHudElementHelper::GetNext() {
	return m_pNext;
}

//-----------------------------------------------------------------------------
// Purpose: Static class creation factory
//  Searches list of registered factories for a match and then instances the
//  requested technology by name
// Input  : *name -
// Output : CBaseTFTechnology
//-----------------------------------------------------------------------------
void CHudElementHelper::CreateAllElements() {
	// Start of list
	CHudElementHelper* p = m_sHelpers;
	while ( p ) {
		// Dispatch creation function directly
		CHudElement* newElement = p->m_pfnCreate();
		if ( newElement ) {
			gHUD.AddHudElement( newElement );
		}

		p = p->GetNext();
	}
}
