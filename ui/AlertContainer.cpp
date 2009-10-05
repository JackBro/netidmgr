#include "khmapp.h"
#include "AlertContainer.hpp"

namespace nim {

    void AlertContainer::PickTitle()
    {
    }

    void AlertContainer::ProcessCommand(int id)
    {
	const int alert_idx = AlertFromControlId(id);
	const int cmd_idx = ButtonFromControlId(id);

	khm_int32 flags = 0;
	khm_int32 cmd = 0;

	assert(alert_idx >= 0 && alert_idx < (int) m_alerts.size());

	if (alert_idx >= (int) m_alerts.size() || alert_idx < 0 || cmd_idx < 0)
	    return;

	AlertElement * e = NULL;
	int n_idx = 0;

	for (DisplayElement *de = TQFIRSTCHILD(this); de; de = TQNEXTSIBLING(de)) {
	    e = dynamic_cast<AlertElement *>(de);
	    if (e) {
		if (n_idx == alert_idx)
		    break;
		n_idx++;
	    }
	}

	assert(alert_idx == 0);
	assert(e);

	if (e == NULL || alert_idx != 0)
	    return;

	{
	    AutoLock<Alert> a_lock(&e->m_alert);

	    assert(cmd_idx >= 0 && cmd_idx < e->m_alert->n_alert_commands);

	    if (cmd_idx >= 0 && cmd_idx < e->m_alert->n_alert_commands) {
		cmd = e->m_alert->alert_commands[cmd_idx];
	    }

	    flags = e->m_alert->flags;

	    e->m_alert->response = cmd;
	}

	// if we were supposed to dispatch the command, do so
	if (cmd != 0 &&
	    cmd != KHUI_PACTION_CLOSE &&
	    (flags & KHUI_ALERT_FLAG_DISPATCH_CMD)) {
	    ::PostMessage(khm_hwnd_main, WM_COMMAND,
			  MAKEWPARAM(cmd, 0), 0);
	}

	// if this was the only alert in the alert group and its close
	// button was clicked, we close the alert window.  Otherwise,
	// the alert window creates its own close button that closes
	// the window.

	if (m_alerts.size() == 1) {
	    HWND hwnd_parent = GetParent(hwnd);

	    if (hwnd_parent)
		::PostMessage(hwnd_parent, WM_CLOSE, 0, 0);
	}

	// While we are at it, we should disable the buttons for this
	// alert since we have already dispatched the command for it.

	if (cmd != 0) {
	    HWND hw_focus = GetFocus();
	    bool focus_trapped = false;

#if 0
	    for (int i=0; i < e->m_alert->n_alert_commands; i++) {
		HWND hw_button = GetDlgItem(hwnd, IDC_FROM_IDX(alert_idx, i));
		if (hw_button) {
		    if (hw_focus == hw_button)
			focus_trapped = true;

		    EnableWindow(hw_button, FALSE);
		}
	    }
#endif

	    if (focus_trapped) {
		HWND hwnd_parent = GetParent(hwnd);
		if (hwnd_parent) {
		    hw_focus = GetNextDlgTabItem(hwnd_parent, hw_focus, FALSE);
		    if (hw_focus)
			::PostMessage(hwnd_parent, WM_NEXTDLGCTL,
				      (WPARAM) hw_focus, MAKELPARAM(TRUE,0));
		}
	    }
	}
    }

    bool AlertContainer::Add(Alert &alert)
    {
	{
	    AutoLock<Alert> a_lock(&alert);

	    if ((alert->flags & KHUI_ALERT_FLAG_DISPLAY_BALLOON) &&
		!(alert->flags & KHUI_ALERT_FLAG_DISPLAY_WINDOW))

		return false;

	    if ((alert->flags & (KHUI_ALERT_FLAG_REQUEST_BALLOON |
				 KHUI_ALERT_FLAG_REQUEST_WINDOW)) == KHUI_ALERT_FLAG_REQUEST_BALLOON)
		return false;

	    if (m_alerts.size() > 0 &&
		(alert->flags & KHUI_ALERT_FLAG_MODAL))
		return false;
	}

	if (m_alerts.size() > 0) {
	    Alert& ref = m_alerts.front();
	    AutoLock<Alert> a_lock(&alert, &ref);

	    if (ref->flags & KHUI_ALERT_FLAG_MODAL)
		return false;

	    if (// Should ideally only be checked if the type isn't NONE
		ref->alert_type != alert->alert_type)
		return false;
	}

	m_alerts.push_back(alert);

	// If the alert container has been initialized already, we
	// have to add new AlertElement elements for this alert.
	return true;
    }

    void AlertContainer::UpdateLayoutPre(Graphics & g, Rect & layout)
    {
	// We only push new alerts at the end of m_alerts and we don't
	// remove alerts.  So we should find any new alerts at the end
	// of m_alerts.

	size_t index = 0;
	AlertList::iterator al = m_alerts.begin();

	for (DisplayElement * e = TQFIRSTCHILD(this);
	     e && al != m_alerts.end();
	     e = TQNEXTSIBLING(e)) {

	    AlertElement * ae = dynamic_cast<AlertElement *>(e);

	    if (ae) {
		assert(ae->m_index == index);
		assert(ae->m_alert == *al);

		++index;
		++al;
	    }
	}

	for (; al != m_alerts.end(); ++al) {

	    AlertElement * e = new AlertElement(*al, (int) index++);

	    InsertChildAfter(e);
	}
    }

    AlertElement *AlertContainer::GetAlertElement(Alert& alert, DisplayElement *container)
    {
	DisplayElement * de;

	for (de = TQFIRSTCHILD(container); de; de = TQNEXTSIBLING(de)) {
	    AlertElement *ae = dynamic_cast<AlertElement *>(de);
	    if (ae && ae->m_alert == alert)
		return ae;
	    ae = GetAlertElement(alert, de);
	    if (ae != NULL)
		return ae;
	}
	return NULL;
    }


    bool AlertContainer::BeginMonitoringAlert(Alert& a)
    {
	AlertElement *ae = GetAlertElement(a, this);

	if (ae == NULL)
	    return false;

	// Context monitors are self-disposing
	AlertContextMonitor * ctxmon = new AlertContextMonitor(ae, this, IDC_NTF_ERRCTXMONITOR);

	return true;
    }

    void AlertContainer::OnCommand(int id, HWND hwndCtl, UINT codeNotify)
    {
	if (id == IDC_NTF_ERRCTXMONITOR) {
	    AlertElement * ae = static_cast<AlertElement *>((void *) hwndCtl);
	    if (ae) {
		ae->OnErrCtxEvent((enum kherr_ctx_event) codeNotify);
	    }
	} else {
	    __super::OnCommand(id, hwndCtl, codeNotify);
	}
    }

    bool AlertContainer::TranslateAccelerator(LPMSG pMsg)
    {
        return !! ::TranslateAccelerator(hwnd, khm_global_accel_table, pMsg);
    }
}