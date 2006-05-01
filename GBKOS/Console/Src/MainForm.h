// MainForm.h
//
// This file declares a class for the main form of
// your application.

#ifndef MAINFORM_H_
#define MAINFORM_H_

class CMainForm : public CForm
{
public:
	// Form event handlers
	Boolean OnOpen(EventType* pEvent, Boolean& bHandled);
	Boolean OnClose(EventType *pEvent, Boolean &bHandled);
	
	// Command handlers
	Boolean OnAbout(EventType* pEvent, Boolean& bHandled);
	Boolean OnEnableCheckboxSelect(EventType *pEvent, Boolean &bHandled);
	Boolean OnCharsetPopupSelect(EventType *pEvent, Boolean &bHandle);
	Boolean OnDetectCharsetCheckboxSelect(EventType *pEvent, Boolean &bHandled);
	Boolean OnBoldCheckboxSelect(EventType *pEvent, Boolean &bHandled);
	Boolean OnLargeBoldCheckboxSelect(EventType *pEvent, Boolean &bHandled);
	Boolean OnWordWrapCheckboxSelect(EventType *pEvent, Boolean &bHandled);
	Boolean OnSpaceCheckboxSelect(EventType *pEvent, Boolean &bHandled);

	// Event map
	BEGIN_EVENT_MAP(CForm)
		EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
		EVENT_MAP_ENTRY(frmCloseEvent, OnClose)
		EVENT_MAP_MENU_ENTRY(OptionsAboutGBKOS, OnAbout)

		EVENT_MAP_COMMAND_ENTRY(MainEnableCheckbox, OnEnableCheckboxSelect)
		EVENT_MAP_POPUP_SELECT_ENTRY(MainCharsetTrigger, OnCharsetPopupSelect)
		EVENT_MAP_COMMAND_ENTRY(MainDetectCharsetCheckbox, OnDetectCharsetCheckboxSelect)
		EVENT_MAP_COMMAND_ENTRY(MainBoldCheckbox, OnBoldCheckboxSelect)
		EVENT_MAP_COMMAND_ENTRY(MainLargeBoldCheckbox, OnLargeBoldCheckboxSelect)
		EVENT_MAP_COMMAND_ENTRY(MainWordWrapCheckbox, OnWordWrapCheckboxSelect)
		EVENT_MAP_COMMAND_ENTRY(MainSpaceCheckbox, OnSpaceCheckboxSelect)
	END_EVENT_MAP()

protected:
	UInt16 _gbkosLibRef;
	ftrGBK _store;
	
	// controls
	CCheckBox _cbxEnable;
	CLabel _labCharset;
	CPopupTrigger _triCharset;
	CSimpleListCtrl _lstCharset;
	CCheckBox _cbxDetectCharset;
	CCheckBox _cbxBoldFont;
	CCheckBox _cbxLargeBoldFont;
	CCheckBox _cbxWordWrap;
	CCheckBox _cbxSpace;
	
	void UpdateControls();
	void InitCharsetControls();
};

#endif // MAINFORM_H_
