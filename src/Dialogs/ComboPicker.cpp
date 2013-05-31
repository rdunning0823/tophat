/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Dialogs/ComboPicker.hpp"
#include "Dialogs/ListPicker.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Base.hpp"
#include "Form/DataField/ComboList.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"

#include <assert.h>

static WndProperty *wComboPopupWndProperty;
static DataField *ComboPopupDataField;
static const ComboList *ComboListPopup;
static UPixelScalar padding;

static void
OnPaintComboPopupListItem(Canvas &canvas, const PixelRect rc, unsigned i)
{
  assert(i < (unsigned)ComboListPopup->size());

  canvas.text_clipped(rc.left + padding,
                      rc.top + padding, rc,
                      (*ComboListPopup)[i].StringValueFormatted);
}
static const TCHAR*
OnItemHelp(unsigned i)
{
  if ((*ComboListPopup)[i].StringHelp)
    return (*ComboListPopup)[i].StringHelp;

  return _T("");
}

int
ComboPicker(SingleWindow &parent, const TCHAR *caption,
            const ComboList &combo_list,
            ListHelpCallback_t help_callback,
            bool enable_item_help)
{
  ComboListPopup = &combo_list;

  const UPixelScalar font_height =
    UIGlobals::GetDialogLook().text_font->GetHeight() + Layout::FastScale(2);
  const UPixelScalar max_height = Layout::GetMaximumControlHeight();
  const UPixelScalar row_height = font_height >= max_height
    ? font_height
    /* this formula is supposed to be a compromise between too small
       and too large: */
    : (font_height + max_height) / 2;

  padding = (row_height - font_height) / 2;

  return ListPicker(parent, caption,
                    combo_list.size(),
                    combo_list.ComboPopupItemSavedIndex,
                    row_height,
                    OnPaintComboPopupListItem, false,
                    help_callback,
                    enable_item_help ? OnItemHelp : NULL);
}

static void
OnHelpClicked(unsigned i)
{
  if (i < ComboListPopup->size()) {
    const ComboList::Item &item = (*ComboListPopup)[i];
    ComboPopupDataField->SetFromCombo(item.DataFieldIndex,
                                      item.StringValue);
  }

  wComboPopupWndProperty->OnHelp();
}

static int
ComboPicker(SingleWindow &parent, const WndProperty &control,
            const ComboList &combo_list, bool EnableItemHelp)
{
  return ComboPicker(parent, control.GetCaption(), combo_list, OnHelpClicked, EnableItemHelp);
}

int
dlgComboPicker(SingleWindow &parent, WndProperty *theProperty)
{
  static bool bInComboPicker = false;
  bool bInitialPage = true;
  // used to exit loop (optionally reruns combo with
  // lower/higher index of items for int/float
  bool bOpenCombo = true;

  /**
   * non-static pointer to datafield.
   * used during closing of picker so that dlgComboPicker can be
   * recursively called by OnModified()
   */
  DataField *non_static_datafield;

  // prevents multiple instances
  if (bInComboPicker)
    return 0;

  bInComboPicker = true;

  TCHAR sSavedInitialValue[100];
  int iSavedInitialDataIndex = -1;

  while (bOpenCombo) {
    assert(theProperty != NULL);
    wComboPopupWndProperty = theProperty;

    non_static_datafield = ComboPopupDataField =
        wComboPopupWndProperty->GetDataField();
    assert(ComboPopupDataField != NULL);

    ComboListPopup = ComboPopupDataField->CreateComboList();
    if (bInitialPage) { // save values for "Cancel" from first page only
      bInitialPage = false;
      iSavedInitialDataIndex =
        (*ComboListPopup)[ComboListPopup->ComboPopupItemSavedIndex]
        .DataFieldIndex;
      ComboPopupDataField->CopyString(sSavedInitialValue, false);
    }

    int idx = ComboPicker(parent, *theProperty, *ComboListPopup, ComboPopupDataField->GetItemHelpEnabled());

    bOpenCombo = false; //tell  combo to exit loop after close

    if (idx >= 0 && (unsigned)idx < ComboListPopup->size()) {
      const ComboList::Item *item = &(*ComboListPopup)[idx];

      // OK/Select
      if (item->DataFieldIndex == ComboList::Item::NEXT_PAGE) {
        // we're last in list and the want more past end of list so select last real list item and reopen
        ComboPopupDataField->SetDetachGUI(true);
        // we'll reopen, so don't call xcsoar data changed routine yet
        item = &(*ComboListPopup)[idx - 1];
        bOpenCombo = true; // reopen combo with new selected index at center
      } else if (item->DataFieldIndex == ComboList::Item::PREVIOUS_PAGE) {
        // same as above but lower items needed
        ComboPopupDataField->SetDetachGUI(true);
        item = &(*ComboListPopup)[idx + 1];
        bOpenCombo = true;
      }
      const ComboList::Item item_temp(item->DataFieldIndex,
                                      item->StringValue, item->StringValue);

      if (!bOpenCombo) {
        delete ComboListPopup;
        bInComboPicker = false; // nothing below this line can use statics!
      }
      non_static_datafield->SetFromCombo(item_temp.DataFieldIndex,
                                         item_temp.StringValue);
    } else {
      // Cancel
      // if we've detached the GUI during the load, then there is nothing to do here
      delete ComboListPopup;
      bInComboPicker = false; // nothing below this line can use statics!
      assert(iSavedInitialDataIndex >= 0);
      if (iSavedInitialDataIndex >= 0)
        // saved from first page if multiple were used
        non_static_datafield->SetFromCombo(iSavedInitialDataIndex,
                                           sSavedInitialValue);
    }
    theProperty->RefreshDisplay();
  } // loop reopen combo if <<More>>  or <<Less>> picked

  return 1;
}
