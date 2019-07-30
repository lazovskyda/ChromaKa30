#pragma once


// CChromaLinkPage dialog

class CChromaLinkPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CChromaLinkPage)

public:
    CChromaLinkPage();
    virtual ~CChromaLinkPage();

// Dialog Data
    enum { IDD = IDD_CHROMALINK_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonLoadingAnimation();
    afx_msg void OnBnClickedButtonHealthAmmoMana();
    afx_msg void OnBnClickedButtonAlerts();
    afx_msg void OnBnClickedButtonDamageTaken();
    afx_msg void OnBnClickedButtonRainbowAnimation();
    afx_msg void OnBnClickedButtonExplosionAnimation();
    afx_msg void OnBnClickedButtonStaticBase();
    afx_msg void OnBnClickedButtonBreathingBase();
    afx_msg void OnBnClickedButtonSpectrumBase();
    afx_msg void OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult);
};
