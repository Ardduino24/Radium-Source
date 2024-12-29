#include "Window.hpp"
#include "Dependencies/imgui/imgui.h"
#include "Dependencies/imgui/imgui_impl_dx11.h"
#include "Dependencies/imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <D3DX11.h>
#include <thread>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND Win, UINT Msg, WPARAM WParam, LPARAM LParam) {
	if (ImGui_ImplWin32_WndProcHandler(Win, Msg, WParam, LParam)) {
		return TRUE;
	}

	return DefWindowProc(Win, Msg, WParam, LParam);
}


HWND Windo{};


int Width;
int Height;


ID3D11Device* Device{};
ID3D11DeviceContext* DeviceContext{ };
IDXGISwapChain* SwapChain{ };
ID3D11RenderTargetView* RenderTargetView{ };
D3D_FEATURE_LEVEL Level{};

WNDCLASSEX wndClass;


HWND WINAPI Window::NewWindow() {
	// Define and register the window class
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.cbClsExtra = NULL;
	wndClass.cbWndExtra = NULL;
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wndClass.hInstance = GetModuleHandle(0);
	wndClass.lpfnWndProc = WndProc;
	wndClass.lpszClassName = "@";
	wndClass.lpszMenuName = "@";
	wndClass.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wndClass)) {
		exit(1);
	}

	// Create the transparent window
	this->Win = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Layered and transparent styles
		"@",
		"@",
		WS_POPUP,
		1, 1, 200, 200,
		FindWindowA(NULL, "Roblox"),
		0, 0, 0
	);

	if (!this->Win) {
		exit(1);
	}

	// Set transparency using LWA_ALPHA
	SetLayeredWindowAttributes(this->Win, 0, 255, LWA_ALPHA);
	SetLayeredWindowAttributes(this->Win, RGB(0, 0, 0), 0, ULW_COLORKEY);

	// Return the created window handle
	return this->Win;
}

// Set the window style to layered and make it transparent
void SetTransparentWindow(HWND hwnd) {
	// Add WS_EX_LAYERED style for transparency
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	// Set transparency to semi-transparent (adjust alpha as needed, 0 to 255)
	SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA); // 128 is 50% transparency
}

Window::Window()
{
	// Create your window (assuming Windo is the HWND)
	Windo = this->NewWindow();
	this->Win = Windo;

	// Debug output
	MessageBoxA(NULL, std::to_string((uintptr_t)this->Win).c_str(), NULL, NULL);

	// Show the window
	ShowWindow(this->Win, SW_SHOW);

	this->InitRendering();
}

void Gui()
{

}

void UpdateOverlay(HWND overlayWindow) {
	RECT lastRect = { 0 }; // To track the previous position and size of the client area

	HWND hWnd = FindWindowA(NULL, "Roblox");

	while (true) {
		Sleep(1); // Use minimal delay for smoother updates

		// Get the target window's client area in screen coordinates
		RECT clientRect;
		if (GetClientRect(hWnd, &clientRect)) {
			POINT topLeft = { clientRect.left, clientRect.top };
			POINT bottomRight = { clientRect.right, clientRect.bottom };
			ClientToScreen(hWnd, &topLeft);
			ClientToScreen(hWnd, &bottomRight);

			// Calculate overlay dimensions
			int overlayWidth = bottomRight.x - topLeft.x;
			int overlayHeight = bottomRight.y - topLeft.y;

			// Check if the overlay needs updating
			RECT currentRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
			if (memcmp(&currentRect, &lastRect, sizeof(RECT)) != 0) {
				lastRect = currentRect; // Update the stored rect

				// Update overlay position and size
				MoveWindow(overlayWindow, topLeft.x, topLeft.y, overlayWidth, overlayHeight, TRUE);
			}
		}
	}
}void Window::InitRendering()
{
	// Swap chain configuration
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1; // Single back buffer
	swapChainDesc.BufferDesc.Width = 800; // Initial width of overlay
	swapChainDesc.BufferDesc.Height = 600; // Initial height of overlay
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Supports alpha channel
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Render target output
	swapChainDesc.OutputWindow = this->Win; // Output to this window
	swapChainDesc.SampleDesc.Count = 1; // No anti-aliasing
	swapChainDesc.Windowed = TRUE; // Windowed mode
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allow resolution switching

	// Create DirectX device, swap chain, and context
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &Device, &featureLevel, &DeviceContext);

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to create device and swap chain!", "Error", MB_ICONERROR);
		return;
	}

	// Create the render target view
	ID3D11Texture2D* BackBuffer = nullptr;
	hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to get back buffer from swap chain!", "Error", MB_ICONERROR);
		return;
	}

	hr = Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTargetView);
	BackBuffer->Release();
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to create the render target view!", "Error", MB_ICONERROR);
		return;
	}

	// Initialize ImGui
	ImGui::CreateContext();
	ImGui::StyleColorsLight(); // Light theme
	ImGui_ImplWin32_Init(this->Win);
	ImGui_ImplDX11_Init(Device, DeviceContext);

	// Start rendering loop
	std::thread(UpdateOverlay, this->Win).detach();

	while (Running)
	{
		// Process window messages
		MSG Msg = {};
		while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);

			if (Msg.message == WM_QUIT) // Graceful exit on WM_QUIT
			{
				Running = false;
			}
		}

		if (!Running)
			break;

		// Start a new ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Example: Draw a filled circle
		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(200, 200), 50, IM_COL32(255, 0, 0, 255));

		// Render ImGui
		ImGui::EndFrame();
		ImGui::Render();

		// Clear the render target with transparency
		constexpr float Color[4] = { 0.f, 0.f, 0.f, 0.f }; // Fully transparent
		DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
		DeviceContext->ClearRenderTargetView(RenderTargetView, Color);

		// Set up blending for transparency
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ID3D11BlendState* blendState = nullptr;
		Device->CreateBlendState(&blendDesc, &blendState);
		float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		DeviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
		blendState->Release(); // Release the blend state after use

		// Render ImGui draw data
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present the frame
		SwapChain->Present(1, 0);
	}

	// Cleanup ImGui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Cleanup DirectX
	if (RenderTargetView) RenderTargetView->Release();
	if (SwapChain) SwapChain->Release();
	if (DeviceContext) DeviceContext->Release();
	if (Device) Device->Release();
}
