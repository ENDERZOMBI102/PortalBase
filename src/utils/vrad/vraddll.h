//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once
#include "ilaunchabledll.h"
#include "ivraddll.h"


class CVRadDLL : public IVRadDLL, public ILaunchableDLL {
public: // IVRadDLL | ILaunchableDLL
	int main( int argc, char** argv ) override;
public: // IVRadDLL
	bool Init( char const* pFilename ) override;
	void Release() override;
	void GetBSPInfo( CBSPInfo* pInfo ) override;
	bool DoIncrementalLight( char const* pVMFFile ) override;
	bool Serialize() override;
	float GetPercentComplete() override;
	void Interrupt() override;
};
