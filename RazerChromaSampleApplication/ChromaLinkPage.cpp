// ChromaLinkPage.cpp : implementation file
//

#include "stdafx.h"
#include "RazerChromaSampleApplication.h"
#include "ChromaLinkPage.h"
#include "afxdialogex.h"
#include "HealthAmmoManaDlg.h"
#include "AlertDlg.h"


// CChromaLinkPage dialog

IMPLEMENT_DYNAMIC(CChromaLinkPage, CPropertyPage)

CChromaLinkPage::CChromaLinkPage()
    : CPropertyPage(CChromaLinkPage::IDD)
{

}

CChromaLinkPage::~CChromaLinkPage()
{
}

void CChromaLinkPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChromaLinkPage, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_LOADING_ANIMATION, &CChromaLinkPage::OnBnClickedButtonLoadingAnimation)
    ON_BN_CLICKED(IDC_BUTTON_HEALTH_AMMO_MANA, &CChromaLinkPage::OnBnClickedButtonHealthAmmoMana)
    ON_BN_CLICKED(IDC_BUTTON_ALERTS, &CChromaLinkPage::OnBnClickedButtonAlerts)
    ON_BN_CLICKED(IDC_BUTTON_DAMAGE_TAKEN, &CChromaLinkPage::OnBnClickedButtonDamageTaken)
    ON_BN_CLICKED(IDC_BUTTON_RAINBOW_ANIMATION, &CChromaLinkPage::OnBnClickedButtonRainbowAnimation)
    ON_BN_CLICKED(IDC_BUTTON_EXPLOSION_ANIMATION, &CChromaLinkPage::OnBnClickedButtonExplosionAnimation)
    ON_BN_CLICKED(IDC_BUTTON_STATIC_BASE, &CChromaLinkPage::OnBnClickedButtonStaticBase)
    ON_BN_CLICKED(IDC_BUTTON_BREATHING_BASE, &CChromaLinkPage::OnBnClickedButtonBreathingBase)
    ON_BN_CLICKED(IDC_BUTTON_SPECTRUM_BASE, &CChromaLinkPage::OnBnClickedButtonSpectrumBase)
    ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, &CChromaLinkPage::OnNMClickSyslink1)
END_MESSAGE_MAP()


// CChromaLinkPage message handlers


void CChromaLinkPage::OnBnClickedButtonLoadingAnimation()
{
    static RZEFFECTID EffectId = GUID_NULL;
    if(IsEqualGUID(EffectId, GUID_NULL))
    {
        g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

        RZEFFECTID Frames[11] = {};

        ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE Effect = {};

        for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
        {
            Effect.Color[i] = RGB(80,30,0);
        }
        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[0]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[0], 500);

        for(int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
        {
            Effect.Color[i] = ORANGE;

            g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[i+1]);

            g_ChromaSDKImpl.AddToGroup(EffectId, Frames[i+1], 80);
        }
    }

    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnBnClickedButtonHealthAmmoMana()
{
    CHealthAmmoManaDlg dlg(this);
    dlg.DoModal();
}


void CChromaLinkPage::OnBnClickedButtonAlerts()
{
    CAlertDlg dlg(this);
    dlg.DoModal();
}


void CChromaLinkPage::OnBnClickedButtonDamageTaken()
{
    static RZEFFECTID EffectId = GUID_NULL;
    if(IsEqualGUID(EffectId, GUID_NULL))
    {
        g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

        RZEFFECTID Frame1 = GUID_NULL;
        RZEFFECTID Frame2 = GUID_NULL;

        ChromaSDK::ChromaLink::STATIC_EFFECT_TYPE Static = {};
        Static.Color = RED;

        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Static, &Frame1);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frame1, 50);

        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_NONE, NULL, &Frame2);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frame2, 50);
    }

    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnBnClickedButtonRainbowAnimation()
{
    // TODO: Add your control notification handler code here
    static RZEFFECTID EffectId = GUID_NULL;
    if (IsEqualGUID(EffectId, GUID_NULL))
    {
        g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

        RZEFFECTID Frames[8] = {};

        ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE Effect = {};

        for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
        {
            Effect.Color[i] = RGB(150, 150, 150);
        }
        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[0]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[0], 500);

        for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
        {
            COLORREF clr;
            if (i == 0)clr = RGB(255, 0, 0);
            else if (i == 1)clr = RGB(255, 255, 0);
            else if (i == 2)clr = RGB(0, 255, 0);
            else if (i == 3)clr = RGB(0, 255, 255);
            else if (i == 4)clr = RGB(255, 0, 255);

            Effect.Color[i] = clr;

            clr = RGB(150, 150, 150);
            for (int j = i+1; j < ChromaSDK::ChromaLink::MAX_LEDS; j++)
            {
                if(j < ChromaSDK::ChromaLink::MAX_LEDS)Effect.Color[j] = clr;
            }

            g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[i + 1]);

            g_ChromaSDKImpl.AddToGroup(EffectId, Frames[i + 1], 80);
        }
    }

    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnBnClickedButtonExplosionAnimation()
{
    // TODO: Add your control notification handler code here
    static RZEFFECTID EffectId = GUID_NULL;
    if (IsEqualGUID(EffectId, GUID_NULL))
    {
        g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

        RZEFFECTID Frames[4] = {};

        ChromaSDK::ChromaLink::CUSTOM_EFFECT_TYPE Effect = {};

        //frame 0
        for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
        {
            Effect.Color[i] = RGB(80,30,0);
        }
        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[0]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[0], 500);

        //frame 1
        Effect.Color[0] = RGB(80, 30, 0);
        Effect.Color[1] = RGB(80, 30, 0);
        Effect.Color[2] = RGB(235, 54, 0);
        Effect.Color[3] = RGB(235, 54, 0);
        Effect.Color[4] = RGB(80, 30, 0);

        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[1]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[1], 400);

        //frame 2
        Effect.Color[0] = RGB(80, 30, 0);
        Effect.Color[1] = RGB(235, 54, 0);
        Effect.Color[2] = RGB(80, 30, 0);
        Effect.Color[3] = RGB(80, 30, 0);
        Effect.Color[4] = RGB(235, 54, 0);

        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[2]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[2], 400);

        //frame 3
        for (int i = 0; i < ChromaSDK::ChromaLink::MAX_LEDS; i++)
        {
            Effect.Color[i] = RGB(80, 30, 0);
        }
        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_CUSTOM, &Effect, &Frames[3]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[3], 80);
    }

    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnBnClickedButtonStaticBase()
{
    // TODO: Add your control notification handler code here
    static RZEFFECTID EffectId = GUID_NULL;

    g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

    RZEFFECTID Frames[1] = {};

    ChromaSDK::ChromaLink::STATIC_EFFECT_TYPE Effect = {};

    Effect.Color = RED;

    g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Effect, &Frames[0]);

    g_ChromaSDKImpl.AddToGroup(EffectId, Frames[0], 80);

    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnBnClickedButtonBreathingBase()
{
    // TODO: Add your control notification handler code here
    static RZEFFECTID EffectId = GUID_NULL;
    if (IsEqualGUID(EffectId, GUID_NULL))
    {
        g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

        RZEFFECTID Frames[30] = {};

        ChromaSDK::ChromaLink::STATIC_EFFECT_TYPE Effect = {};

        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Effect, &Frames[0]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[0], 500);

        for (int i = 0; i < 11; i++)
        {
            Effect.Color = RGB(0, i * 25, 0);

            g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Effect, &Frames[i + 1]);

            g_ChromaSDKImpl.AddToGroup(EffectId, Frames[i + 1], 150);
        }
        int nextIndex = 12;
        for (int i = 10; i >= 0; i--)
        {
            Effect.Color = RGB(0, i * 25, 0);

            g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Effect, &Frames[nextIndex]);

            g_ChromaSDKImpl.AddToGroup(EffectId, Frames[nextIndex], 150);

            nextIndex++;
        }
    }
    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnBnClickedButtonSpectrumBase()
{
    // TODO: Add your control notification handler code here
    static RZEFFECTID EffectId = GUID_NULL;
    if (IsEqualGUID(EffectId, GUID_NULL))
    {
        g_ChromaSDKImpl.CreateEffectGroup(&EffectId);

        RZEFFECTID Frames[15] = {};

        ChromaSDK::ChromaLink::STATIC_EFFECT_TYPE Effect = {};

        g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Effect, &Frames[0]);

        g_ChromaSDKImpl.AddToGroup(EffectId, Frames[0], 500);

        COLORREF Spectrum[14] = {
            RGB(125,0,0),   RGB(255,0,0),   RGB(255,125,0),
            RGB(255,255,0), RGB(125,255,0), RGB(0,255,0),
            RGB(0,255,125), RGB(0,255,255), RGB(0,125,255),
            RGB(0,0,255),   RGB(125,0,255), RGB(255,0,255),
            RGB(125,0,125), RGB(0,0,0)
        };

        for (int i = 0; i < 14; i++)
        {
            Effect.Color = Spectrum[i];

            g_ChromaSDKImpl.CreateChromaLinkEffectImpl(ChromaSDK::ChromaLink::CHROMA_STATIC, &Effect, &Frames[i + 1]);

            g_ChromaSDKImpl.AddToGroup(EffectId, Frames[i + 1], 200);
        }
    }
    g_ChromaSDKImpl.SetEffectImpl(EffectId);
}


void CChromaLinkPage::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
    PNMLINK pNMLink = (PNMLINK)pNMHDR;
    ShellExecuteW(NULL, L"open", L"http://developer.razerzone.com/works-with-chroma/chroma-link-guide/", NULL, NULL, SW_SHOWNORMAL);
    *pResult = 0;
}
