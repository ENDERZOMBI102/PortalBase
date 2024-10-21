//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: C callable material system interface for the utils.
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "utilmatlib.h"
#include "filesystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/materialsystem_config.h"
#include "mathlib/mathlib.h"
#include "tier0/dbg.h"
#include <cmdlib.h>


#if IsPosix()
	#define DX_TO_GL_ABSTRACTION
	#include <appframework/ilaunchermgr.h>

	static CreateInterfaceFn g_fileSystemFactory;

	class FakeLauncherMgr : public ILauncherMgr {
	public:
		bool Connect( CreateInterfaceFn factory ) override {
			return true;
		}

		void Disconnect() override { }

		void* QueryInterface( const char* pInterfaceName ) override {
			return nullptr;
		}

		InitReturnVal_t Init() override {
			return InitReturnVal_t::INIT_OK;
		}

		void Shutdown() override { }

		bool CreateGameWindow( const char* pTitle, bool bWindowed, int width, int height ) override {
			return true;
		}

		void IncWindowRefCount() override { }

		void DecWindowRefCount() override { }

		int GetEvents( CCocoaEvent* pEvents, int nMaxEventsToReturn, bool debugEvents = false ) override {
			return 0;
		}

		#if IsLinux()
			int PeekAndRemoveKeyboardEvents( bool* pbEsc, bool* pbReturn, bool* pbSpace, bool debugEvents = false ) override {
				return 0;
			}
		#endif

		void SetCursorPosition( int x, int y ) override { }

		void SetWindowFullScreen( bool bFullScreen, int nWidth, int nHeight ) override { }

		bool IsWindowFullScreen() override {
			return true;
		}

		void MoveWindow( int x, int y ) override { }

		void SizeWindow( int width, int tall ) override { }

		void PumpWindowsMessageLoop() override { }

		void DestroyGameWindow() override { }
		void SetApplicationIcon( const char* pchAppIconFile ) override { }

		void GetMouseDelta( int& x, int& y, bool bIgnoreNextMouseDelta = false ) override { }

		void GetNativeDisplayInfo( int nDisplay, uint& nWidth, uint& nHeight, uint& nRefreshHz ) override { }

		void RenderedSize( uint& width, uint& height, bool set ) override { }

		void DisplayedSize( uint& width, uint& height ) override { }

		#if defined( DX_TO_GL_ABSTRACTION )
			PseudoGLContextPtr GetMainContext() override {
				return nullptr;
			}

			PseudoGLContextPtr GetGLContextForWindow( void* windowref ) override {
				return nullptr;
			}

			PseudoGLContextPtr CreateExtraContext() override {
				return nullptr;
			}

			void DeleteContext( PseudoGLContextPtr hContext ) override { }

			bool MakeContextCurrent( PseudoGLContextPtr hContext ) override {
				return true;
			}

			GLMDisplayDB* GetDisplayDB() override {
				return nullptr;
			}

			void GetDesiredPixelFormatAttribsAndRendererInfo( uint** ptrOut, uint* countOut, GLMRendererInfoFields* rendInfoOut ) override { }

			void ShowPixels( CShowPixelsParams* params ) override { }
		#endif

		void GetStackCrawl( CStackCrawlParams* params ) override { }

		void WaitUntilUserInput( int msSleepTime ) override { }

		void* GetWindowRef() override {
			return nullptr;
		}

		void SetMouseVisible( bool bState ) override { }

		void SetMouseCursor( SDL_Cursor* hCursor ) override { }

		void SetForbidMouseGrab( bool bForbidMouseGrab ) override { }

		void OnFrameRendered() override { }

		void SetGammaRamp( const uint16* pRed, const uint16* pGreen, const uint16* pBlue ) override { }

		double GetPrevGLSwapWindowTime() override {
			return 0.0;
		}

	} g_FakeLauncherMgr;

	void* SDLMgrFactoryRedirector( const char* pName, int* pReturnCode ) {
		if ( strcmp( "SDLMgrInterface001", pName ) == 0 ) {
			return &g_FakeLauncherMgr;
		}
		return g_fileSystemFactory( pName, pReturnCode );
	}
#endif

void LoadMaterialSystemInterface( CreateInterfaceFn fileSystemFactory ) {
	if ( g_pMaterialSystem ) {
		return;
	}

	#if IsPosix()
		g_fileSystemFactory = fileSystemFactory;
		fileSystemFactory = SDLMgrFactoryRedirector;
	#endif

	// materialsystem.dll should be in the path, it's in bin along with vbsp.
	CSysModule* materialSystemDLLHInst{ g_pFullFileSystem->LoadModule( "materialsystem" ) };
	if ( !materialSystemDLLHInst ) {
		Error( "Can't load MaterialSystem's module\n" );
	}

	CreateInterfaceFn clientFactory = Sys_GetFactory( materialSystemDLLHInst );
	if ( clientFactory ) {
		g_pMaterialSystem = static_cast<IMaterialSystem*>( clientFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, nullptr ) );
		if ( !g_pMaterialSystem ) {
			Error( "Could not get the material system interface from MaterialSystem's module (" __FILE__ ")" );
		}
	} else {
		Error( "Could not find factory interface in MaterialSystem module" );
	}

	if ( !g_pMaterialSystem->Init( "shaderapiempty", nullptr, fileSystemFactory ) ) {
		Error( "Could not start the empty shader (shaderapiempty%s)!", DLL_EXT_STRING );
	}
}

void InitMaterialSystem( const char* materialBaseDirPath, CreateInterfaceFn fileSystemFactory ) {
	LoadMaterialSystemInterface( fileSystemFactory );
	MaterialSystem_Config_t config;
	g_pMaterialSystem->OverrideConfig( config, false );
}

void ShutdownMaterialSystem() {
	if ( g_pMaterialSystem ) {
		g_pMaterialSystem->Shutdown();
		g_pMaterialSystem = nullptr;
	}
}

MaterialSystemMaterial_t FindMaterial( const char* materialName, bool* pFound, bool bComplain ) {
	IMaterial* pMat = g_pMaterialSystem->FindMaterial( materialName, TEXTURE_GROUP_OTHER, bComplain );

	if ( pFound ) {
		*pFound = true;
		if ( IsErrorMaterial( pMat ) ) {
			*pFound = false;
		}
	}

	return pMat;
}

void GetMaterialDimensions( MaterialSystemMaterial_t materialHandle, int* width, int* height ) {
	ImageFormat dummyImageFormat;
	const auto* material = static_cast<IMaterial*>( materialHandle );
	bool translucent;
	PreviewImageRetVal_t retVal = material->GetPreviewImageProperties( width, height, &dummyImageFormat, &translucent );
	if ( retVal != MATERIAL_PREVIEW_IMAGE_OK ) {
		#if 0
			if ( retVal == MATERIAL_PREVIEW_IMAGE_BAD ) {
				Error( "problem getting preview image for %s", g_pMaterialSystem->GetMaterialName( materialInfo[matID].materialHandle ) );
			}
		#else
			*width = 128;
			*height = 128;
		#endif
	}
}

void GetMaterialReflectivity( MaterialSystemMaterial_t materialHandle, float* reflectivityVect ) {
	auto* material = static_cast<IMaterial*>( materialHandle );
	bool found;
	const IMaterialVar* reflectivityVar = material->FindVar( "$reflectivity", &found, false );
	if ( !found ) {
		Vector tmp;
		material->GetReflectivity( tmp );
		VectorCopy( tmp.Base(), reflectivityVect );
	} else {
		reflectivityVar->GetVecValue( reflectivityVect, 3 );
	}
}

int GetMaterialShaderPropertyBool( MaterialSystemMaterial_t materialHandle, int propID ) {
	auto* material = static_cast<IMaterial*>( materialHandle );
	switch ( propID ) {
		case UTILMATLIB_NEEDS_BUMPED_LIGHTMAPS:
			return material->GetPropertyFlag( MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS );

		case UTILMATLIB_NEEDS_LIGHTMAP:
			return material->GetPropertyFlag( MATERIAL_PROPERTY_NEEDS_LIGHTMAP );

		default:
			Assert( 0 );
			return 0;
	}
}

int GetMaterialShaderPropertyInt( MaterialSystemMaterial_t materialHandle, int propID ) {
	if ( propID == UTILMATLIB_OPACITY ) {
		auto* material = static_cast<IMaterial*>( materialHandle );

		if ( material->IsTranslucent() ) {
			return UTILMATLIB_TRANSLUCENT;
		}
		if ( material->IsAlphaTested() ) {
			return UTILMATLIB_ALPHATEST;
		}

		return UTILMATLIB_OPAQUE;
	}

	Assert( 0 );
	return 0;
}

const char* GetMaterialVar( MaterialSystemMaterial_t materialHandle, const char* propertyName ) {
	auto* material{ static_cast<IMaterial*>( materialHandle ) };
	bool found;
	const IMaterialVar* var{ material->FindVar( propertyName, &found, false ) };
	if ( found ) {
		return var->GetStringValue();
	}

	return nullptr;
}

const char* GetMaterialShaderName( MaterialSystemMaterial_t materialHandle ) {
	return static_cast<IMaterial*>( materialHandle )->GetShaderName();
}
