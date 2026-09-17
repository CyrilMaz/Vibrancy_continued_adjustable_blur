#include <windows.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>

#include <windows.ui.composition.interop.h>
#include <DispatcherQueue.h>

#include "composition-effects.h"

namespace vibrancy_custom_blur
{
    static winrt::Windows::System::DispatcherQueueController
        g_dispatcherController{ nullptr };

    static winrt::Windows::UI::Composition::Compositor
        g_compositor{ nullptr };

    static void ensureComposition()
    {
        if (!winrt::Windows::System::DispatcherQueue::GetForCurrentThread())
        {
            DispatcherQueueOptions options{
                sizeof(DispatcherQueueOptions),
                DQTYPE_THREAD_CURRENT,
                DQTAT_COM_STA
            };

            namespace abi = ABI::Windows::System;

            winrt::check_hresult(
                CreateDispatcherQueueController(
                    options,
                    reinterpret_cast<abi::IDispatcherQueueController**>(
                        winrt::put_abi(g_dispatcherController)
                    )
                )
            );
        }

        if (!g_compositor)
        {
            g_compositor =
                winrt::Windows::UI::Composition::Compositor();
        }
    }

    HRESULT probeWindowTarget(HWND hwnd)
    {
        try
        {
            ensureComposition();

            namespace abi =
                ABI::Windows::UI::Composition::Desktop;

            auto interop =
                g_compositor.as<abi::ICompositorDesktopInterop>();

            winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget
                target{ nullptr };

            HRESULT hr =
                interop->CreateDesktopWindowTarget(
                    hwnd,
                    false,
                    reinterpret_cast<abi::IDesktopWindowTarget**>(
                        winrt::put_abi(target)
                    )
                );

            if (FAILED(hr))
            {
                return hr;
            }

            target.Root(nullptr);
            target.Close();

            return S_OK;
        }
        catch (winrt::hresult_error const& error)
        {
            return error.code();
        }
        catch (...)
        {
            return E_FAIL;
        }
    }
}