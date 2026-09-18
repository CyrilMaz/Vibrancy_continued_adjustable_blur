#include <windows.h>
#include <dwmapi.h>

#include <unordered_map>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.Graphics.Effects.h>

#include <windows.ui.composition.interop.h>
#include <DispatcherQueue.h>

#include "composition-effects.h"

#pragma comment(lib, "dwmapi.lib")

namespace vibrancy_custom_blur
{
    static winrt::Windows::System::DispatcherQueueController
        g_dispatcherController{ nullptr };

    static winrt::Windows::UI::Composition::Compositor
        g_compositor{ nullptr };

    struct WindowBlurState
    {
        winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget
            target{ nullptr };

        winrt::Windows::UI::Composition::ContainerVisual
            root{ nullptr };

        winrt::Windows::UI::Composition::SpriteVisual
            visual{ nullptr };

        winrt::Windows::UI::Composition::CompositionEffectBrush
            effectBrush{ nullptr };
    };

    static std::unordered_map<HWND, WindowBlurState>
        g_windowStates;

    static void clearWindowState(HWND hwnd)
    {
        auto it = g_windowStates.find(hwnd);

        if (it == g_windowStates.end())
        {
            return;
        }

        if (it->second.target)
        {
            it->second.target.Root(nullptr);
            it->second.target.Close();
        }

        g_windowStates.erase(it);
    }

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

    static winrt::Windows::UI::Composition::CompositionEffectBrush
    createBlurBrush(float blurAmount)
    {
        using namespace Microsoft::UI::Composition::Effects;

        auto sourceParameter =
            winrt::Windows::UI::Composition::CompositionEffectSourceParameter(
                L"Backdrop"
            );

        auto blurEffect =
            Microsoft::WRL::Make<GaussianBlurEffect>();

        // Direct2D BlurAmount correspond au sigma du flou.
        // On expose ici une valeur plus proche d'un "rayon" utilisateur.
        winrt::check_hresult(
            blurEffect->put_BlurAmount(
                blurAmount / 3.0f
            )
        );

        winrt::check_hresult(
            blurEffect->put_BorderMode(
                EffectBorderMode_Hard
            )
        );

        winrt::check_hresult(
            blurEffect->put_Optimization(
                EffectOptimization_Speed
            )
        );

        auto sourceAbi =
            reinterpret_cast<
                ABI::Windows::Graphics::Effects::IGraphicsEffectSource*
            >(
                winrt::get_abi(sourceParameter)
            );

        winrt::check_hresult(
            blurEffect->put_Source(
                sourceAbi
            )
        );

        winrt::Windows::Graphics::Effects::IGraphicsEffect
            effect{ nullptr };

        winrt::check_hresult(
            blurEffect->QueryInterface(
                winrt::guid_of<
                    winrt::Windows::Graphics::Effects::IGraphicsEffect
                >(),
                winrt::put_abi(effect)
            )
        );

        auto factory =
            g_compositor.CreateEffectFactory(
                effect
            );

        auto brush =
            factory.CreateBrush();

        auto backdrop =
            g_compositor.CreateHostBackdropBrush();

        brush.SetSourceParameter(
            L"Backdrop",
            backdrop
        );

        return brush;
    }

    HRESULT enableCustomBlur(
        HWND hwnd,
        float blurAmount
    )
    {
        try
        {
            ensureComposition();

            // blurAmount == 0 doit réellement signifier :
            // aucun HostBackdrop et aucun effet natif ajouté.
            if (blurAmount <= 0.0f)
            {
                BOOL useHostBackdrop = FALSE;

                winrt::check_hresult(
                    DwmSetWindowAttribute(
                        hwnd,
                        DWMWA_USE_HOSTBACKDROPBRUSH,
                        &useHostBackdrop,
                        sizeof(useHostBackdrop)
                    )
                );

                clearWindowState(hwnd);

                return S_OK;
            }

            // Le HostBackdrop doit être explicitement activé
            // pour que CreateHostBackdropBrush puisse échantillonner
            // le contenu situé derrière la fenêtre Win32.
            BOOL useHostBackdrop = TRUE;

            winrt::check_hresult(
                DwmSetWindowAttribute(
                    hwnd,
                    DWMWA_USE_HOSTBACKDROPBRUSH,
                    &useHostBackdrop,
                    sizeof(useHostBackdrop)
                )
            );

            // Nettoie proprement un éventuel effet précédent.
            clearWindowState(hwnd);

            namespace abi =
                ABI::Windows::UI::Composition::Desktop;

            auto interop =
                g_compositor.as<
                    abi::ICompositorDesktopInterop
                >();

            WindowBlurState state;

            winrt::check_hresult(
                interop->CreateDesktopWindowTarget(
                    hwnd,
                    false,
                    reinterpret_cast<
                        abi::IDesktopWindowTarget**
                    >(
                        winrt::put_abi(state.target)
                    )
                )


            );

            // Attach a real visual tree to the DesktopWindowTarget.
            // This is the path that lets a transparent Electron window show
            // our HostBackdropBrush + Gaussian blur behind Chromium content.
            state.root =
                g_compositor.CreateContainerVisual();

            state.root.RelativeSizeAdjustment(
                { 1.0f, 1.0f }
            );

            state.visual =
                g_compositor.CreateSpriteVisual();

            state.visual.RelativeSizeAdjustment(
                { 1.0f, 1.0f }
            );

            state.effectBrush =
                createBlurBrush(
                    blurAmount
                );

            state.visual.Brush(
                state.effectBrush
            );

            state.root.Children().InsertAtTop(
                state.visual
            );

            state.target.Root(
                state.root
            );

            g_windowStates.emplace(
                hwnd,
                std::move(state)
            );

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

    HRESULT probeWindowTarget(HWND hwnd)
    {
        try
        {
            ensureComposition();

            namespace abi =
                ABI::Windows::UI::Composition::Desktop;

            auto interop =
                g_compositor.as<
                    abi::ICompositorDesktopInterop
                >();

            winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget
                target{ nullptr };

            HRESULT hr =
                interop->CreateDesktopWindowTarget(
                    hwnd,
                    false,
                    reinterpret_cast<
                        abi::IDesktopWindowTarget**
                    >(
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