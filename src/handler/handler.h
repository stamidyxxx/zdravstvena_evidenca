#pragma once
#include "../includes.h"

using namespace std;

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

class Handler 
{
public:

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void WaitForLastSubmittedFrame();
	FrameContext* WaitForNextFrameResources();
//	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int Run(function<void()> func);


	// Data
	FrameContext				g_frameContext[NUM_FRAMES_IN_FLIGHT];
	UINT                        g_frameIndex;
	ID3D12Device*				g_pd3dDevice;
	ID3D12DescriptorHeap*		g_pd3dRtvDescHeap;
	ID3D12DescriptorHeap*		g_pd3dSrvDescHeap;
	ID3D12CommandQueue*			g_pd3dCommandQueue;
	ID3D12GraphicsCommandList*	g_pd3dCommandList;
	ID3D12Fence*				g_fence;
	HANDLE						g_fenceEvent;
	UINT64						g_fenceLastSignaledValue;
	IDXGISwapChain3*			g_pSwapChain;
	bool						g_SwapChainOccluded;
	HANDLE						g_hSwapChainWaitableObject;
	ID3D12Resource*				g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE	g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
};

extern Handler handler;