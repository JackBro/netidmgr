#pragma once

namespace nim
{
    /*! \brief Credential Row Outline Node
     */
    class CwCredentialRow : public CwOutline
    {
    public:
        Credential  credential;
        std::vector<DisplayElement *> el_list;

    public:
        CwCredentialRow(const Credential& _credential, int _column) :
            credential(_credential),
            CwOutline(_column) {
        }

        ~CwCredentialRow() {}

        virtual bool Represents(const Credential& _credential) {
            return credential == _credential;
        }

        virtual void UpdateLayoutPre(Graphics& g, Rect& layout) {

            WithColumnAlignment::UpdateLayoutPre(g, layout);

            if (el_list.size() == 0) {
                for (int i = col_idx;
                     (col_span > 0 && i < col_idx + col_span) ||
                         (col_span <= 0 && (unsigned int) i < owner->columns.size()); i++) {
                    CwColumn * cwc = dynamic_cast<CwColumn *>(owner->columns[i]);
                    DisplayElement * e;

                    switch (cwc->attr_id) {
                    case KCDB_ATTR_RENEW_TIMELEFT:
                    case KCDB_ATTR_TIMELEFT:
                        e = new CwTimeTextCellElement(credential, cwc->attr_id,
                                                      FTSE_INTERVAL, i);
                        break;

                    default:
                        e = new CwStaticCellElement(credential.GetAttribStringObj(cwc->attr_id), i);
                    }

                    InsertChildAfter(e);
                    el_list.push_back(e);
                }
            }
        }

        virtual void UpdateLayoutPost(Graphics& g, const Rect& layout) {
            DisplayElement::UpdateLayoutPost(g, layout);
        }

        virtual void PaintSelf(Graphics& g, const Rect& bounds, const Rect& clip) {
            g_theme->DrawCredWindowNormalBackground(g, bounds, GetDrawState());
        }

        void SetContext(SelectionContext& sctx) {
            sctx.Add(credential);
            dynamic_cast<CwOutlineBase*>(TQPARENT(this))->SetContext(sctx);
        }

        void Select(bool _select) {
            if (_select == selected)
                return;

            __super::Select(_select);

            credential.SetFlags((selected)? KCDB_CRED_FLAG_SELECTED : 0,
                                KCDB_CRED_FLAG_SELECTED);
        }
    };
}
