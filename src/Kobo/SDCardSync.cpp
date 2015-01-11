/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "SDCardSync.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Dialogs/Message.hpp"
#include "UIGlobals.hpp"
#include "Screen/Key.h"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Form/ActionListener.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Event/Timer.hpp"
#include "System.hpp"
#include <stdlib.h>

/**
 * Dialog that allows syncing both to and from the SDCard with the
 * XCSoarData folder.
 */
class SDCardSyncWidget final
  : public RowFormWidget, ActionListener, Timer {
  enum Buttons {
    SDCardToDevice,
    DeviceToSDCard,
  };

  WndButton *upload_to_device_button, *copy_to_sd_card_button;

public:
  /**
   * Hack to allow the widget to close its surrounding dialog.
   */
  ActionListener *listener;

public:
  SDCardSyncWidget(const DialogLook &look)
    :RowFormWidget(look) {}

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;

  virtual void Unprepare() override {
    Timer::Cancel();
    RowFormWidget::Unprepare();
  }

private:
  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;

  /* virtual methods from class Timer */
  virtual void OnTimer() {
    if (!IsUSBStorageConnected())
      listener->OnAction(mrOK);
  }
};

void
SDCardSyncWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  upload_to_device_button = AddButton(_T("USB card --> device"),
                                      *this, SDCardToDevice);

  copy_to_sd_card_button = AddButton(
      _T("Device --> USB card"),
      *this, DeviceToSDCard);

  Timer::Schedule(1000);
}

void
SDCardSyncWidget::OnAction(int id)
{
  if (!IsUSBStorageConnected())
    ShowMessageBox(_T("USB card is not connected"),
                   _T("Error"), MB_OK | MB_ICONERROR);
  else
    switch (id) {
    case SDCardToDevice:
      UploadSDCardToDevice();
      ShowMessageBox(_T("USB card successfully copied to device"),
                     _T("Success"), MB_OK);
      break;

    case DeviceToSDCard:
      CopyTopHatDataToSDCard();
      ShowMessageBox(_T("Top Hat data and logs copied to USB card"),
                     _T("Success"), MB_OK);
      break;
    }

  listener->OnAction(mrOK);
}

void
ShowSDCardSyncDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  SDCardSyncWidget widget(look);
  WidgetDialog dialog(look);
  widget.listener = &dialog;
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Top Hat data sync"), &widget);
  dialog.AddButton(_("Close"), mrOK);
  dialog.ShowModal();
  dialog.StealWidget();
}
