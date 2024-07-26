/*********************************************************************************
*  MIT License
*  
*  Copyright (c) 2023 Strata Source Contributors
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*********************************************************************************/
#include "cbase.h"

#include "imgui_impl_source.hpp"

#include "KeyValues.h"
#include "filesystem.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "pixelwriter.h"

#include "vgui/ISystem.h"
#include "vgui_controls/Controls.h"

#include "tier0/memdbgon.h"

static IMaterial* g_pFontMat = nullptr;

class CDearImGuiFontTextureRegenerator : public ITextureRegenerator {
public:
	CDearImGuiFontTextureRegenerator() = default;

	// Inherited from ITextureRegenerator
	void RegenerateTextureBits( ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect ) override {
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

		Assert( pVTFTexture->Width() == width );
		Assert( pVTFTexture->Height() == height );
		// if we ever use freetype for font loading, this should do format conversion instead
		memcpy( pVTFTexture->ImageData(), pixels, 4ULL * width * height );
	}

	void Release() override {
		delete this;
	}
};

void ImGui_ImplSource_SetupRenderState( IMatRenderContext* ctx, ImDrawData* draw_data ) {
	// Apply imgui's display dimensions
	ctx->Viewport( draw_data->DisplayPos.x, draw_data->DisplayPos.y, draw_data->DisplaySize.x, draw_data->DisplaySize.y );

	// Setup orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	ctx->MatrixMode( MATERIAL_PROJECTION );
	ctx->LoadIdentity();
	ctx->Scale( 1, -1, 1 );

	float L = draw_data->DisplayPos.x + 0.5f;
	float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
	float T = draw_data->DisplayPos.y + 0.5f;
	float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
	ctx->Ortho( L, T, R, B, 0.f, 1.f );

	ctx->MatrixMode( MATERIAL_VIEW );
	ctx->LoadIdentity();
}

void ImGui_ImplSource_RenderDrawData( ImDrawData* draw_data ) {
	// Avoid rendering when minimized
	if ( draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f )
		return;

	CMatRenderContextPtr ctx( materials );

	ctx->MatrixMode( MATERIAL_VIEW );
	ctx->PushMatrix();
	ctx->MatrixMode( MATERIAL_PROJECTION );
	ctx->PushMatrix();

	ImGui_ImplSource_SetupRenderState( ctx, draw_data );

	// Render command lists
	ImVec2 clip_off = draw_data->DisplayPos;
	for ( int n = 0; n < draw_data->CmdListsCount; n++ ) {
		const ImDrawList* cmd_list = draw_data->CmdLists[ n ];
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

		// Draw the mesh
		for ( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++ ) {
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ cmd_i ];
			if ( pcmd->UserCallback != nullptr ) {
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if ( pcmd->UserCallback == ImDrawCallback_ResetRenderState )
					ImGui_ImplSource_SetupRenderState( ctx, draw_data );
				else
					pcmd->UserCallback( cmd_list, pcmd );
			} else {
				if ( pcmd->GetTexID() ) {
					Vector2D clipmin = { pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y };
					Vector2D clipmax = { pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y };

					// Avoid rendering completely clipped draws
					if ( clipmax.x <= clipmin.x || clipmax.y <= clipmin.y )
						continue;

					ctx->SetScissorRect( clipmin.x, clipmin.y, clipmax.x, clipmax.y, true );
					IMesh* mesh = ctx->GetDynamicMesh( false, nullptr, nullptr, static_cast<IMaterial*>( pcmd->GetTexID() ) );
					CMeshBuilder mb;
					mb.Begin( mesh, MATERIAL_TRIANGLES, cmd_list->VtxBuffer.Size, pcmd->ElemCount );

					const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
					for ( int i = 0; i < cmd_list->VtxBuffer.Size; i++ ) {
						mb.Position3f( Vector2DExpand( vtx_src->pos ), 0 );
						mb.Color4ubv( reinterpret_cast<const unsigned char*>( &vtx_src->col ) );
						mb.TexCoord2fv( 0, &vtx_src->uv.x );
						mb.AdvanceVertexF<VTX_HAVEPOS | VTX_HAVECOLOR, 1>();
						vtx_src++;
					}

					static_cast<CIndexBuilder&>( mb ).FastIndexList( idx_buffer + pcmd->IdxOffset, 0, pcmd->ElemCount );
					mb.End( false, true );
					ctx->SetScissorRect( clipmin.x, clipmin.y, clipmax.x, clipmax.y, false );
				}
			}
		}
	}

	ctx->MatrixMode( MATERIAL_PROJECTION );
	ctx->PopMatrix();
	ctx->MatrixMode( MATERIAL_VIEW );
	ctx->PopMatrix();
}

bool ImGui_ImplSource_Init() {
	// Setup backend capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendPlatformName = "source";
	io.BackendRendererName = "imgui_impl_source";
	io.BackendFlags = ImGuiBackendFlags_None;
	io.SetClipboardTextFn = []( void*, const char* c ) {
		vgui::system()->SetClipboardText( c, V_strlen( c ) );
	};
	io.GetClipboardTextFn = []( void* ctx ) -> const char* {
		auto& g = *static_cast<ImGuiContext*>( ctx );
		g.ClipboardHandlerData.clear();
		auto len = vgui::system()->GetClipboardTextCount();
		if ( !len )
			return nullptr;
		g.ClipboardHandlerData.resize( len );
		vgui::system()->GetClipboardText( 0, g.ClipboardHandlerData.Data, g.ClipboardHandlerData.Size );
		return g.ClipboardHandlerData.Data;
	};

	ImGui_ImplSource_CreateDeviceObjects();
	return true;
}

void ImGui_ImplSource_Shutdown() {
	ImGui_ImplSource_InvalidateDeviceObjects();
}

static bool ImGui_ImplSource_CreateFontsTexture() {
	if ( g_pFontMat )
		return true;

	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	int width, height;
	unsigned char* pixels;
	io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

	// Create a material for the texture
	ITexture* fonttex = g_pMaterialSystem->CreateProceduralTexture( "imgui_font", TEXTURE_GROUP_OTHER, width, height, IMAGE_FORMAT_RGBA8888, TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_POINTSAMPLE | TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_SINGLECOPY | TEXTUREFLAGS_NOLOD );
	fonttex->SetTextureRegenerator( new CDearImGuiFontTextureRegenerator );

	KeyValues* vmt = new KeyValues( "UnlitGeneric" );
	vmt->SetString( "$basetexture", "imgui_font" );
	vmt->SetInt( "$nocull", 1 );
	vmt->SetInt( "$vertexcolor", 1 );
	vmt->SetInt( "$vertexalpha", 1 );
	vmt->SetInt( "$translucent", 1 );
	g_pFontMat = materials->CreateMaterial( "imgui_font_mat", vmt );
	g_pFontMat->AddRef();

	// Store our identifier
	io.Fonts->SetTexID( g_pFontMat );

	return true;
}

bool ImGui_ImplSource_CreateDeviceObjects() {
	return ImGui_ImplSource_CreateFontsTexture();
}

void ImGui_ImplSource_InvalidateDeviceObjects() {
	if ( g_pFontMat ) {
		g_pFontMat->DecrementReferenceCount();
		g_pFontMat = nullptr;
	}
}

// The following functions are declared in imconfig_source.h and must not be renamed

ImFileHandle ImFileOpen( const char* filename, const char* mode ) {
	Assert( g_pFullFileSystem );
	return g_pFullFileSystem->Open( filename, mode );
}

bool ImFileClose( ImFileHandle f ) {
	if ( f == nullptr )
		return false;

	Assert( g_pFullFileSystem );
	g_pFullFileSystem->Close( f );
	return true;
}

uint64 ImFileGetSize( ImFileHandle f ) {
	Assert( g_pFullFileSystem && f );

	return g_pFullFileSystem->Size( f );
}

uint64 ImFileRead( void* data, uint64 sz, uint64 count, ImFileHandle f ) {
	Assert( g_pFullFileSystem && f );

	return g_pFullFileSystem->Read( data, sz * count, f );
}

uint64 ImFileWrite( const void* data, uint64 sz, uint64 count, ImFileHandle f ) {
	Assert( g_pFullFileSystem && f );

	return g_pFullFileSystem->Write( data, sz * count, f );
}