/*
 * Copyright (c) 2005 Massachusetts Institute of Technology
 * Copyright (c) 2007, 2009, 2010 Secure Endpoints Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* $Id$ */

#ifndef __KHIMAIRA_CREDFUNCS_H
#define __KHIMAIRA_CREDFUNCS_H

BEGIN_C

khm_boolean
khm_cred_begin_new_cred_op(void);

void
khm_cred_end_new_cred_op(void);

khm_boolean
khm_new_cred_ops_pending(void);

void KHMAPI
kmsg_cred_completion(kmq_message *m);

void
khm_cred_destroy_creds(khm_boolean sync,
                       khm_boolean quiet);

void
khm_cred_destroy_identity(khm_handle identity);

void
khm_cred_renew_all_identities(void);

void
khm_cred_renew_identity(khm_handle identity);

void
khm_cred_renew_cred(khm_handle cred);

void
khm_cred_renew_creds(void);

void
khm_cred_show_identity_options();

void
khm_cred_prompt_for_identity_modal(const wchar_t * w_title,
                                   khm_handle *pidentity);

void
khm_cred_obtain_new_creds(wchar_t * window_title);

void
khm_cred_obtain_new_creds_for_ident(khm_handle ident, wchar_t * title);

LRESULT
khm_cred_configure_identity(khui_configure_identity_data * pcid);

LRESULT
khm_cred_collect_privileged_creds(khui_collect_privileged_creds_data * pcpcd);

khm_int32
khm_cred_derive_identity_from_privileged_creds(khui_collect_privileged_creds_data * pcd);

void
khm_cred_set_default(void);

void
khm_cred_set_default_identity(khm_handle identity);

void
khm_cred_change_password(wchar_t * window_title);

void
khm_cred_dispatch_process_message(khui_new_creds *nc);

BOOL
khm_cred_dispatch_process_level(khui_new_creds *nc);

khm_int32
khm_cred_abort_process_message(khui_new_creds * nc);

khm_boolean
khm_cred_conclude_processing(khui_new_creds * nc);

BOOL
khm_cred_is_in_dialog(void);

typedef void (*nc_dialog_completion_callback)(khui_new_creds *, void *);

khm_int32
khm_cred_wait_for_dialog(DWORD timeout, nc_dialog_completion_callback cb,
                         void *);

void
khm_cred_begin_startup_actions(void);

void
khm_cred_process_startup_actions(void);

void
khm_cred_refresh(void);

void
khm_cred_addr_change(void);

void
khm_cred_import(void);

void
khm_cred_end_dialog(khui_new_creds * nc);

BOOL khm_cred_is_new_creds_pending(khui_new_creds * nc);

END_C

#endif
