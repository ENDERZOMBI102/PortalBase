//
// Created by ENDERZOMBI102 on 30/06/2024.
//
#include "fsdriver.hpp"
#include "tier1/mempool.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static CMemoryPoolMT g_DescriptorArena{ sizeof( FileDescriptor ), 10, UTLMEMORYPOOL_GROW_SLOW, "FileSystem|DescriptorArena" };

auto FileDescriptor::Make() -> FileDescriptor* {
	auto desc{ static_cast<FileDescriptor*>( g_DescriptorArena.AllocZero( sizeof( FileDescriptor ) ) ) };
	return new(desc) FileDescriptor;
}

auto FileDescriptor::Free( FileDescriptor* desc ) -> void {
	delete[] desc->m_Path;  // make sure we delete the duped path
	g_DescriptorArena.Free( desc );
}

auto FileDescriptor::CleanupArena() -> void {
	g_DescriptorArena.Clear();
}

CFsDriver::CFsDriver() = default;
CFsDriver::~CFsDriver() = default;
