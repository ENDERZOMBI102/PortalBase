#include "vstdlib/random.h"
#include <random>
#include <iostream>


CUniformRandomStream g_UniformRandomStream{};
IUniformRandomStream* g_pUniformRandomStream{ &g_UniformRandomStream };
CGaussianRandomStream g_GaussianRandomStream{ &g_UniformRandomStream };

// --- CUniformRandomStream ---
CUniformRandomStream::CUniformRandomStream() = default;
void CUniformRandomStream::SetSeed( const int iSeed ) {
	this->m_Mutex.Lock();
	this->m_Engine.seed( iSeed );
	this->m_Mutex.Unlock();
}

float CUniformRandomStream::RandomFloat( const float flMinVal, const float flMaxVal ) {
	this->m_Mutex.Lock();
	const auto res{ std::uniform_real_distribution{ flMinVal, flMaxVal }( this->m_Engine ) };
	this->m_Mutex.Unlock();
	return res;
}
int CUniformRandomStream::RandomInt( const int iMinVal, const int iMaxVal ) {
	this->m_Mutex.Lock();
	const auto res{ std::uniform_int_distribution{ iMinVal, iMaxVal }( this->m_Engine ) };
	this->m_Mutex.Unlock();
	return res;
}
float CUniformRandomStream::RandomFloatExp( const float flMinVal, const float flMaxVal, const float flExponent ) {
	this->m_Mutex.Lock();
	const auto res{ std::uniform_real_distribution{ flMinVal, flMaxVal }( this->m_Engine ) };
	this->m_Mutex.Unlock();
	return std::pow( res, flExponent );
}

// --- CGaussianRandomStream ---
CGaussianRandomStream::CGaussianRandomStream( IUniformRandomStream* pUniformStream ) {
	this->m_pUniformStream = pUniformStream ? pUniformStream : g_pUniformRandomStream;
}
void CGaussianRandomStream::AttachToStream( IUniformRandomStream* pUniformStream ) {
	this->m_pUniformStream = pUniformStream ? pUniformStream : g_pUniformRandomStream;
}
float CGaussianRandomStream::RandomFloat( const float flMean, const float flStdDev ) {
	return std::normal_distribution{ flMean, flStdDev }( dynamic_cast<CUniformRandomStream*>( this->m_pUniformStream )->m_Engine );
}

// --- globals ---
void RandomSeed( const int iSeed ) {
	g_pUniformRandomStream->SetSeed( iSeed );
}
float RandomFloat( const float flMinVal, const float flMaxVal ) {
	return g_pUniformRandomStream->RandomFloat( flMinVal, flMaxVal );
}
float RandomFloatExp( const float flMinVal, const float flMaxVal, const float flExponent ) {
	return g_pUniformRandomStream->RandomFloatExp( flMinVal, flMaxVal, flExponent );
}
int RandomInt( const int iMinVal, const int iMaxVal ) {
	return g_pUniformRandomStream->RandomInt( iMinVal, iMaxVal );
}
float RandomGaussianFloat( const float flMean, const float flStdDev ) {
	return g_GaussianRandomStream.RandomFloat( flMean, flStdDev );
}

void InstallUniformRandomStream( IUniformRandomStream* pStream ) {
	if ( not pStream ) {
		pStream = &g_UniformRandomStream;
	}
	g_pUniformRandomStream = pStream;
	g_GaussianRandomStream.AttachToStream( pStream );
}
